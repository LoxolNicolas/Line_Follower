#include "mbed.h"
#include "Capteur_Position.hpp"
#include "SPI_Capteur.hpp"
#include "Config_capteur.hpp"
#include "I2C_Capteur.hpp"

Coordonnee tab_cord[NOMBRE_MESURE] = {0};
Data_I2C tab_I2C[NOMBRE_MESURE_I2C] = {0};
Serial pc(USBTX, USBRX);
Serial bt(D1, D0); //MODULE bluetooth
char initComplete = 0;
volatile char movementFlag = 0;
float theta_1 = 0.0f;
int numero_coordonnee = 1;
int indice = 1;
int numero_i2c = 1;
volatile int16_t xydat[2]; //Valeur de X et Y
float pas_courbure = 0.0f;
int nb_tour_circuit = 1;

Timer mesureTimer;

Timer vitesseTimer;
float vitesse = 0.0f;
float currentAngle = 0.0f;

int oldCourbure = 0;

float distance_act = 0.0f;
char courbure_act = 0;

float courbure_moyenne = 0;

void setup()
{
	pc.baud(115200);
	bt.baud(115200);

	spi.frequency(1000000); // 2MHz maximum frequency
	spi.format(8, 3); // SPI mode 3
	performStartup();

	i2cSetup();

	initComplete = 9;
}

int UpdatePointer(Coordonnee* prec, Coordonnee* act, float* distance_act)
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
			tmp[i] = (unsigned char) spi_read_reg(Delta_X_L + i);
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

		currentAngle = act->theta;

		theta_1 = theta_1 + (360.0f / (2.0f * PI * DISTANCE_CENTRE_CAPTEUR)) * abs(x_cm);

		if(abs(y_cm) > 0.000000001f) {
			act->courbure = (360.0f / (2.0f * PI * DISTANCE_CENTRE_CAPTEUR) * x_cm) / y_cm;
			courbure_moyenne = courbure_moyenne * (1.0f - courbure_moyenne_new_coef) + courbure_moyenne_new_coef * act->courbure;
			int cd = static_cast<int>(courbure_moyenne / DEGRE_ECHANTILLONAGE);
			pc.printf("fc: %f c: %d oc: %d\n",courbure_moyenne, cd , oldCourbure);

			if( cd != oldCourbure)
			{
				valeur = 1;

				tab_cord[numero_coordonnee].x = act->x;
				tab_cord[numero_coordonnee].y = act->y;
				tab_cord[numero_coordonnee].theta = act->theta;
				tab_cord[numero_coordonnee].distance = act->distance;
				tab_cord[numero_coordonnee].courbure = act->courbure;

				numero_coordonnee++;

				//theta_1 -= DEGRE_ECHANTILLONAGE; //Lorsque les point sont echantillones selon l'angle

				pc.printf("aaaaaaaaaaaaa\n");
				oldCourbure = cd;
			}
		}

		if(distance_act != nullptr)
			*distance_act = act->distance;



		movementFlag = 1;

		spi_write_reg(Motion, 0x00);
	}

	return valeur;
}

float mini(Coordonnee tab[], int taille)
{
	float minimum = tab[0].courbure;

	for(int i = 1; i < taille; i++)
	{
		if(minimum > tab[i].courbure)
		{
			minimum = tab[i].courbure;
		}
	}

	return minimum;
}

float maxi(Coordonnee tab[], int taille)
{
	float maximum = tab[0].courbure;

	for(int i = 1; i < taille; i++)
	{
		if(maximum < tab[i].courbure)
		{
			maximum = tab[i].courbure;
		}
	}

	return maximum;
}

int arrondi(float valeur)
{
	int entier_inf = (int)valeur;

	int retour = entier_inf;

	if(valeur - entier_inf >= 0.5f)
	{
		retour = entier_inf + 1;
	}

	return retour;
}

unsigned char Courbure_To_Char(float courbure)
{
	float courbureMax = mini(tab_cord, indice);
	float courbureMin = maxi(tab_cord, indice);

	pas_courbure = (courbureMax - courbureMin) / 256;

	return arrondi((pas_courbure) * courbure);
}

void remplir_tab_I2C(Data_I2C tab_I2C[], Coordonnee tab_cord[], int taille)
{
	for(int i = 0; i < taille; i++)
	{
		tab_I2C[i].courbure_discret = Courbure_To_Char(tab_cord[i].courbure);
		tab_I2C[i].distance_avant_next_point = tab_cord[i].distance;
	}
}

void next_info_I2C(Data_I2C tab_I2C[], float* distance_act, char* courbure_act, int* indice_tableau_act)
{
	for(int i = (*indice_tableau_act); i < indice; i++)
	{
		if(tab_I2C[i].distance_avant_next_point >= (*distance_act))
		{
			*distance_act = tab_I2C[i].distance_avant_next_point - (*distance_act);
			*courbure_act = tab_I2C[i].courbure_discret;

			*indice_tableau_act = i;
		}
	}
}