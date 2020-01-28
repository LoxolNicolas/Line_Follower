#include "mbed-os/mbed.h"

// Registers
#define Product_ID  0x00
#define Revision_ID 0x01
#define Motion  0x02
#define Delta_X_L 0x03
#define Delta_X_H 0x04
#define Delta_Y_L 0x05
#define Delta_Y_H 0x06
#define SQUAL 0x07
#define Raw_Data_Sum  0x08
#define Maximum_Raw_data  0x09
#define Minimum_Raw_data  0x0A
#define Shutter_Lower 0x0B
#define Shutter_Upper 0x0C
#define Control 0x0D
#define Config1 0x0F
#define Config2 0x10
#define Angle_Tune  0x11
#define Frame_Capture 0x12
#define SROM_Enable 0x13
#define Run_Downshift 0x14
#define Rest1_Rate_Lower  0x15
#define Rest1_Rate_Upper  0x16
#define Rest1_Downshift 0x17
#define Rest2_Rate_Lower  0x18
#define Rest2_Rate_Upper  0x19
#define Rest2_Downshift 0x1A
#define Rest3_Rate_Lower  0x1B
#define Rest3_Rate_Upper  0x1C
#define Observation 0x24
#define Data_Out_Lower  0x25
#define Data_Out_Upper  0x26
#define Raw_Data_Dump 0x29
#define SROM_ID 0x2A
#define Min_SQ_Run  0x2B
#define Raw_Data_Threshold  0x2C
#define Config5 0x2F
#define Power_Up_Reset  0x3A
#define Shutdown  0x3B
#define Inverse_Product_ID  0x3F
#define LiftCutoff_Tune3  0x41
#define Angle_Snap  0x42
#define LiftCutoff_Tune1  0x4A
#define Motion_Burst  0x50
#define LiftCutoff_Tune_Timeout 0x58
#define LiftCutoff_Tune_Min_Length  0x5A
#define SROM_Load_Burst 0x62
#define Lift_Config 0x63
#define Raw_Data_Burst  0x64
#define LiftCutoff_Tune2  0x65

// Macros for PROGMEM adaptation
#define pgm_read_word(x)          (*(const short int*)x)
#define pgm_read_dword_near(x)    (*(const int*)x)
#define pgm_read_word_near(x)     (*(const unsigned int*)x)
#define pgm_read_int_near(x)      (*(const int*)x)
#define pgm_read_int(x)           (*(const int*)x)
#define pgm_read_char(x)          (*(const char*)x)
#define pgm_read_byte(x)          (*(const char*)x)
#define pgm_read_byte_near(x)     (*(const char*)x)
#define PROGMEM                   const

#define RESOLUTION_CAPTEUR 12000.0
#define INCH_TO_CM 2.54
#define NOMBRE_MESURE 2000
#define DISTANCE_ECHANTILLONAGE 1.0 //Correspond au deplacement d'un cm
#define DEGRE_ECHANTILLONAGE 1.0f
#define PI 3.14159265358979323846f
#define DISTANCE_CENTRE_CAPTEUR 15.666f

#define TO_DEGREE(x) ((x) * (180/PI))

SPI spi(D11, D12, D13); // MOSI, MISO, SCLK
DigitalOut ss(A3); //SS
Serial pc(USBTX, USBRX); // TX / RX
Serial bt(D1, D0); //MODULE bluetooth


struct Coordonnee
{
    float x;
    float y;
    float theta;
    float distance;
};

char initComplete = 0;
volatile int16_t xydat[2]; //Valeur de X et Y
volatile char movementFlag = 0;
char testctr = 0;
double distance1 = 0.0;
float theta_1 = 0.0;
int numero_coordonnee = 1;
int nb_mesure = 0;

Timer mesureTimer;

Timer vitesseTimer;
float vitesse = 0;

Coordonnee tab_cord[NOMBRE_MESURE] = {0};

I2CSlave i2cslave(D4, D5); //SLAVE DECLARATION
const int SLAVE_ADDRESS = 0xA0; //SLAVE ADRESS

char i2cInfoRequested = 0;
