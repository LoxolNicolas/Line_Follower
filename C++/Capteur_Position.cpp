#include "mbed.h"
#include "SROM_0x04.hpp"

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

void spi_upload_firmware()
{
    // send the firmware to the chip, cf p.18 of the datasheet
    //pc.printf("Uploading firmware...\r\n");

    //Write 0 to Rest_En bit of Config2 register to disable Rest mode.
    spi_write_reg(Config2, 0x00);

    // write 0x1d in SROM_enable reg for initializing
    spi_write_reg(SROM_Enable, 0x1d);

    // wait for more than one frame period
    wait_ms(10); // assume that the frame rate is as low as 100fps... even if it should never be that low

    // write 0x18 to SROM_enable to start SROM download
    spi_write_reg(SROM_Enable, 0x18);

    spi_com_begin();

    // write the SROM file (=firmware data)
    spi.write(SROM_Load_Burst | 0b10000000); // write burst destination adress
    wait_us(15);

    // send all chars of the firmware
    unsigned char c;
    
    for(uint16_t i = 0; i < firmware_length; i++) 
    {
        c = firmware_data[i];
        spi.write(c);
        wait_us(15);
    }

    spi_com_end();
    wait_us(160);

    // perform SROM CRC test
    spi_write_reg(SROM_Enable, 0x15); // start CRC test
    wait_ms(10);
    int16_t crc = 0;
    crc |= (spi_read_reg(Data_Out_Upper) & 0xFF);
    crc = crc << 8;
    crc |= (spi_read_reg(Data_Out_Lower) & 0xFF);
    //pc.printf("CRC Check: %04X\r\n", crc);


    //Read the SROM_ID register to verify the ID before any other register reads or writes.
    char result = spi_read_reg(SROM_ID);
    //printf("SROM_ID: %02X\r\n", result);

    //Write 0x00 to Config2 register for wired mouse or 0x20 for wireless mouse design.
    spi_write_reg(Config2, 0x00);

    // set initial CPI resolution
    spi_write_reg(Config1, 0x77); //RESOLUTION A 12000 CPI
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
    spi_upload_firmware();
    wait_ms(10);
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
//            pc.printf("%02X ", tmp[i]);
        }
//       pc.printf("\r\n");

        xydat[0] = tmp[1]; // High bits from Delta_X
        xydat[0] = xydat[0] << 8;
        xydat[0] |= tmp[0]; // Low bits from Delta_X

        xydat[1] = tmp[3]; // High bits from Delta_X
        xydat[1] = xydat[1] << 8;
        xydat[1] |= tmp[2]; // Low bits from Delta_X
                
        float x_cm = (xydat[0] / RESOLUTION_CAPTEUR) * INCH;
        float y_cm = (xydat[1] / RESOLUTION_CAPTEUR) * INCH;
        
        //tab_coord[indice].theta = TO_DEGREE(atan2(tab_coord[indice].y - tab_coord[indice - 1].y, tab_coord[indice].x - tab_coord[indice - 1].x));
        
        //act->x = prec->x + x_cm; //REPERE EN ABSOLU
        //act->y = prec->y - y_cm; //REPERE EN ABSOLU
        act->distance = prec->distance + sqrt(pow(x_cm, 2) + pow(-y_cm, 2));
        
        act->theta = prec->theta + ((360.0f / (2.0f * PI * DISTANCE_CENTRE_CAPTEUR)) * x_cm / 2);
        
        act->x -= y_cm * sin((act->theta) / 360.0f * 2.0f * PI); //REPERE DU ROBOT
        act->y -= y_cm * cos((act->theta) / 360.0f * 2.0f * PI); //REPERE DU ROBOT
                
        act->theta = act->theta + ((360.0f / (2.0f * PI * DISTANCE_CENTRE_CAPTEUR)) * x_cm / 2);
        
        distance1 = distance1 + sqrt(pow(act->x - prec->x, 2) + pow(act->y - prec->y, 2));
        theta_1 = theta_1 + (360.0f / (2.0f * PI * DISTANCE_CENTRE_CAPTEUR)) * x_cm;
        
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

void setup()
{
    pc.baud(115200);
    bt.baud(115200);

    spi.frequency(1000000); // 2MHz maximum frequency
    spi.format(8, 3); // SPI mode 3

    performStartup();

    wait(5);
    
    initComplete=9;
}

int main()
{
    setup();
    
    int indice = 1;
    
    Coordonnee prec = {0.0, 0.0, 0.0, 0.0};
    
    Coordonnee act = {0.0, 0.0, 0.0, 0.0};

    timer.start();
    
    tab_cord[0].x = 0;
    tab_cord[0].y = 0;
    tab_cord[0].theta = 0;
    tab_cord[0].distance = 0;
    
    bt.printf("%f %f %f\n", act.x, act.y, act.distance);
        
    while(act.distance < 1000.0f)
    { 
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

        wait_ms(1);
    }
    
    bt.printf("stop\n");
    
    timer.stop();
    
    return 0;
}
