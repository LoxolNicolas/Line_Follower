#ifndef CAPTEUR_POSITION_HPP
#define CAPTEUR_POSITION_HPP

#include "mbed.h"

#define RESOLUTION_CAPTEUR 5000.0f
#define INCH_TO_CM 2.54
#define NOMBRE_MESURE 1000
#define NOMBRE_MESURE_I2C 2000
#define DEGRE_ECHANTILLONAGE 0.5f
#define PI 3.14159265358979323846f
#define DISTANCE_CENTRE_CAPTEUR 15.666f

extern Serial pc; // TX / RX
extern Serial bt; //MODULE bluetooth


extern Timer mesureTimer;

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
	float distance_avant_next_point;
};

extern char initComplete;
extern volatile int16_t xydat[]; //Valeur de X et Y
extern volatile char movementFlag;
extern float theta_1;
extern int indice;
extern int numero_coordonnee;
extern int numero_i2c;

extern Timer mesureTimer;

extern Timer vitesseTimer;
extern float vitesse;
extern float currentAngle;

extern float pas_courbure;

extern int nb_tour_circuit;

extern Coordonnee tab_cord[];
extern Data_I2C tab_I2C[];

extern float distance_act;
extern char courbure_act;

extern float courbure_moyenne;
constexpr float courbure_moyenne_new_coef = 0.5f;

void setup();
int UpdatePointer(Coordonnee* prec, Coordonnee* act, float* distance_act = nullptr);
float mini();
float maxi();
int arrondi(float valeur);
unsigned char Courbure_To_Char(float courbure);
void remplir_tab_I2C(Data_I2C tab_I2C[], Coordonnee tab_cord[], int taille);
void next_info_I2C(Data_I2C tab_I2C[], float* distance_act, char* courbure_act, int* indice_tableau_act);

#endif