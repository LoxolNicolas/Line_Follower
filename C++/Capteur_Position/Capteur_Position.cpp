#include "mbed.h"
#include "Capteur_Position.hpp"

void spi_com_begin()
{
	spi.lock();
	ss = 0; //NCS Non Chip Select
}

void spi_com_end()
{
	spi.unlock();
	ss = 1; //NCS Non Chip Select
}

char spi_read_reg(char reg_addr)
{
	spi_com_begin();

	spi.write(reg_addr & 0b01111111); //Bit de poid fort a 0

	wait_us(160); // tSRAD

	char data = spi.write(0);

	wait_us(1); // tSCLK-NCS for read operation is 120ns

	spi_com_end();

	wait_us(19); //  tSRW/tSRR (=20us) minus tSCLK-NCS

	return data;
}

void spi_write_reg(char reg_addr, char data)
{
	spi_com_begin();

	spi.write(reg_addr | 0b10000000 );

	wait_us(10);

	spi.write(data);

	wait_us(20); // tSCLK-NCS for write operation

	spi_com_end();

	wait_us(100); // tSWW/tSWR (=120us) minus tSCLK-NCS. Could be shortened, but is looks like a safe lower bound
}

void performStartup(void)
{
	spi_com_end(); // ensure that the serial port is reset
	spi_com_begin(); // ensure that the serial port is reset
	spi_com_end(); // ensure that the serial port is reset
	spi_write_reg(Power_Up_Reset, 0x5a); // force reset
	wait_ms(50); // wait for it to reboot
	// read registers 0x02 to 0x06 (and discard the data)
	spi_read_reg(Motion);
	spi_read_reg(Delta_X_L);
	spi_read_reg(Delta_X_H);
	spi_read_reg(Delta_Y_L);
	spi_read_reg(Delta_Y_H);
	// upload the firmware

	//pc.printf("Optical Chip Initialized\r\n");
}

int UpdatePointer(Coordonnee* prec, Coordonnee* act)
{
	int valeur = 0;

	if(initComplete == 9)
	{
		//write 0x01 to Motion register and read from it to freeze the motion values and make them available
		//spi_write_reg(Motion, 0x01); //DO NOT WRITE NOW writting clear the x, y registers and MOT bit. Data written is not saved, write dummy sata
		spi_read_reg(Motion); // Freeze Delta_X_L, Delta_X_H, Delta_Y_L, Delta_Y_H, and the MOT bit registers

		unsigned char tmp[4];

		for(char i = 0; i < 4; i++)
		{
			tmp[i] = spi_read_reg(Delta_X_L + i);
			//pc.printf("%02X ", tmp[i]);
		}
		//pc.printf("\r\n");

		xydat[0] = tmp[1]; // High bits from Delta_X
		xydat[0] = xydat[0] << 8;
		xydat[0] |= tmp[0]; // Low bits from Delta_X

		xydat[1] = tmp[3]; // High bits from Delta_X
		xydat[1] = xydat[1] << 8;
		xydat[1] |= tmp[2]; // Low bits from Delta_X

		float x_cm = (xydat[0] / RESOLUTION_CAPTEUR) * INCH_TO_CM;
		float y_cm = (xydat[1] / RESOLUTION_CAPTEUR) * INCH_TO_CM;

		vitesse = y_cm / vitesseTimer.read();
		vitesseTimer.reset();

		//tab_coord[indice].theta = TO_DEGREE(atan2(tab_coord[indice].y - tab_coord[indice - 1].y, tab_coord[indice].x - tab_coord[indice - 1].x));

		//act->x = prec->x + x_cm; //REPERE EN ABSOLU
		//act->y = prec->y - y_cm; //REPERE EN ABSOLU
		act->distance = prec->distance + y_cm;

		act->theta = prec->theta + ((360.0f / (2.0f * PI * DISTANCE_CENTRE_CAPTEUR)) * x_cm / 2);

		act->x -= y_cm * sin((act->theta) / 360.0f * 2.0f * PI); //REPERE DU ROBOT
		act->y -= y_cm * cos((act->theta) / 360.0f * 2.0f * PI); //REPERE DU ROBOT

		act->theta = act->theta + ((360.0f / (2.0f * PI * DISTANCE_CENTRE_CAPTEUR)) * x_cm / 2);

		distance1 = distance1 + sqrt(pow(act->x - prec->x, 2) + pow(act->y - prec->y, 2));
		theta_1 = theta_1 + (360.0f / (2.0f * PI * DISTANCE_CENTRE_CAPTEUR)) * abs(x_cm);

		if(theta_1 > DEGRE_ECHANTILLONAGE && numero_coordonnee < NOMBRE_MESURE)
		{
			valeur = 1;

			tab_cord[numero_coordonnee].x = act->x;
			tab_cord[numero_coordonnee].y = act->y;
			tab_cord[numero_coordonnee].theta = act->theta;
			tab_cord[numero_coordonnee].distance = act->distance;

			//pc.printf("Coordonee : %d ---> X : %f | Y : %f | D : %f | T : %f | A : %f\r\n", numero_coordonnee, tab_cord[numero_coordonnee].x, tab_cord[numero_coordonnee].y, tab_cord[numero_coordonnee].distance, timer.read(), tab_cord[numero_coordonnee].theta);

			numero_coordonnee++;

			theta_1 -= DEGRE_ECHANTILLONAGE;
		}

		movementFlag = 1;

		spi_write_reg(Motion, 0x00);
	}

	return valeur;
}

void i2c() {
	pc.printf("receive\n");
	int operation = i2cslave.receive(); //IF THE SLAVE HAS BEEN ADRESSED
	switch(operation)
	{
		case I2CSlave::ReadAddressed: //MASTER IS READING AT THE SLAVE ADRESS
		{
			switch(i2cInfoRequested) {
				case 0:
					i2cslave.write((char *) &vitesse , sizeof(vitesse));
					pc.printf("writen speed: %f\n", vitesse);
				break;
				case 1:
					// TODO changer les infos renvoyées par la distance avant le prochain point et la prochaine courbure
					i2cslave.write((char *) tab_cord , sizeof(tab_cord[0]));
					pc.printf("writen map info\n");
				break;
			}


			break;
		}
		case I2CSlave::WriteAddressed: //MASTER IS WRITING TO THE SLAVE ADRESS
		{
			i2cslave.read(&i2cInfoRequested, 1);

			break;
		}
		default:
			break;
	}
}

void setup()
{
	pc.baud(115200);
	bt.baud(115200);

	spi.frequency(1000000); // 2MHz maximum frequency
	spi.format(8, 3); // SPI mode 3

	performStartup();


	initComplete=9;
}

int main()
{
	setup();

	vitesseTimer.start();
	mesureTimer.start();

	int indice = 1;

	Coordonnee prec = {0.0, 0.0, 0.0, 0.0};

	Coordonnee act = {0.0, 0.0, 0.0, 0.0};

	tab_cord[0].x = 0;
	tab_cord[0].y = 0;
	tab_cord[0].theta = 0;
	tab_cord[0].distance = 0;


	bt.printf("%f %f %f\n", act.x, act.y, act.distance);

	while(act.distance < 1000.0f)
	{
		if(mesureTimer.read_ms() > 1) {
			int type = UpdatePointer(&prec, &act);

			if(type == 0)
			{
				bt.printf("%f %f %f A\n", act.x, act.y, act.distance);
			}

			if(type == 1)
			{
				bt.printf("%f %f %f B\n", tab_cord[indice].x, tab_cord[indice].y, tab_cord[indice].distance);
				indice++;
			}

			prec = act;

			mesureTimer.reset();
		}

		i2c();
	}

	bt.printf("stop\n");

	return 0;
}
