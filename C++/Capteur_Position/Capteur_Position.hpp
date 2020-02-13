#ifndef CAPTEUR_POSITION_HPP
#define CAPTEUR_POSITION_HPP

#include "mbed.h"

#define RESOLUTION_CAPTEUR 12000.0
#define INCH_TO_CM 2.54
#define NOMBRE_MESURE 1000
#define NOMBRE_MESURE_I2C 2000
#define DEGRE_ECHANTILLONAGE 0.4f
#define PI 3.14159265358979323846f
#define DISTANCE_CENTRE_CAPTEUR 15.666f

extern Serial pc; // TX / RX
extern Serial bt; //MODULE bluetooth

struct Coordonnee
{
	float x;
	float y;
	float theta;
	float distance;
	float courbure;
};

struct Data_I2C
{
	char courbure_discret;
	float distance;
};

extern char initComplete;
extern volatile int16_t xydat[]; //Valeur de X et Y
extern volatile char movementFlag;
extern float theta_1;
extern int numero_coordonnee;
extern int numero_i2c;

extern Timer mesureTimer;

extern Timer vitesseTimer;
extern float vitesse;
extern float currentAngle;

extern Coordonnee tab_cord[];
extern Data_I2C tab_I2C[];

void setup();
int UpdatePointer(Coordonnee* prec, Coordonnee* act);
float mini();
float maxi();
int RangeVitesse(float vitesse);

#endif
