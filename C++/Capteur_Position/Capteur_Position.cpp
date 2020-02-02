#include "mbed.h"
#include "Capteur_Position.hpp"
#include "SPI_Capteur.hpp"
#include "Config_capteur.hpp"

Coordonnee tab_cord[NOMBRE_MESURE] = {0};
Serial pc(USBTX, USBRX);
Serial bt(D1, D0); //MODULE bluetooth
Timer mesureTimer;
Timer vitesseTimer;
char initComplete = 0;
volatile char movementFlag = 0;
float theta_1 = 0.0;
int numero_coordonnee = 1;
float vitesse = 0.0f;
volatile int16_t xydat[2]; //Valeur de X et Y

void setup()
{
	pc.baud(115200);
	bt.baud(115200);

	spi.frequency(1000000); // 2MHz maximum frequency
	spi.format(8, 3); // SPI mode 3
	performStartup();

	initComplete = 9;
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
		}

		xydat[0] = tmp[1]; // High bits from Delta_X
		xydat[0] = xydat[0] << 8;
		xydat[0] |= tmp[0]; // Low bits from Delta_X

		xydat[1] = tmp[3]; // High bits from Delta_X
		xydat[1] = xydat[1] << 8;
		xydat[1] |= tmp[2]; // Low bits from Delta_X

		float x_cm = (xydat[0] / RESOLUTION_CAPTEUR) * INCH_TO_CM;
		float y_cm = (xydat[1] / RESOLUTION_CAPTEUR) * INCH_TO_CM;

		vitesse = -y_cm / vitesseTimer.read();
		vitesseTimer.reset();

		//act->x = prec->x + x_cm; //REPERE EN ABSOLU
		//act->y = prec->y - y_cm; //REPERE EN ABSOLU

		act->distance = prec->distance + abs(y_cm);

		act->theta = prec->theta + ((360.0f / (2.0f * PI * DISTANCE_CENTRE_CAPTEUR)) * x_cm / 2);

		act->x -= y_cm * sin((act->theta) / 360.0f * 2.0f * PI); //REPERE DU ROBOT
		act->y -= y_cm * cos((act->theta) / 360.0f * 2.0f * PI); //REPERE DU ROBOT

		act->theta = act->theta + ((360.0f / (2.0f * PI * DISTANCE_CENTRE_CAPTEUR)) * x_cm / 2);

		theta_1 = theta_1 + (360.0f / (2.0f * PI * DISTANCE_CENTRE_CAPTEUR)) * abs(x_cm);

		if(theta_1 > DEGRE_ECHANTILLONAGE && numero_coordonnee < NOMBRE_MESURE) //TO DO AVEC COURBURE
		{
			valeur = 1;

			tab_cord[numero_coordonnee].x = act->x;
			tab_cord[numero_coordonnee].y = act->y;
			tab_cord[numero_coordonnee].theta = act->theta;
			tab_cord[numero_coordonnee].distance = act->distance;
			tab_cord[numero_coordonnee].courbure = (360.0f / (2.0f * PI * DISTANCE_CENTRE_CAPTEUR) * x_cm) / y_cm;

			numero_coordonnee++;

			theta_1 -= DEGRE_ECHANTILLONAGE;
		}

		movementFlag = 1;

		spi_write_reg(Motion, 0x00);
	}

	return valeur;
}

float mini(float tabvitesse[], int taille)
{
	float minimum = tabvitesse[0];

	for(int i = 1; i < taille; i++)
	{
		if(minimum > tabvitesse[i])
		{
			minimum = tabvitesse[i];
		}
	}

	return minimum;
}

float maxi(float tabvitesse[], int taille)
{
	float maximum = tabvitesse[0];

	for(int i = 1; i < taille; i++)
	{
		if(maximum < tabvitesse[i])
		{
			maximum = tabvitesse[i];
		}
	}

	return maximum;
}

int RangeVitesse(float vitesse, float vitesse_min, float vitesse_max)
{
	//return round(((vitesse_max - vitesse_min) / 127.0) * vitesse); //ROUND PAS DEFINI
	return 0;
}