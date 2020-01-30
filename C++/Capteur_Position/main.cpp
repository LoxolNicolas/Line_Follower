#include "Capteur_Position.hpp"
#include "I2C_Capteur.hpp"

int main()
{
    setup();
    i2cSetup();

    vitesseTimer.start();
    mesureTimer.start();

    int indice = 1;

    Coordonnee prec = {0.0, 0.0, 0.0, 0.0, 0.0};

    Coordonnee act = {0.0, 0.0, 0.0, 0.0, 0.0};

    tab_cord[0].x = 0;
    tab_cord[0].y = 0;
    tab_cord[0].theta = 0;
    tab_cord[0].distance = 0;
    tab_cord[0].courbure = 0;


    bt.printf("%f %f %f\n", act.x, act.y, act.distance);

    while(act.distance < 1000.0f)
    {
        if(mesureTimer.read_ms() > 1) {
            int type = UpdatePointer(&prec, &act);

            if(type == 0)
            {
                //bt.printf("%f %f %f C\n", act.x, act.y, act.distance);
            }

            if(type == 1)
            {
                //bt.printf("%f %f %f %f D\n", tab_cord[indice].x, tab_cord[indice].y, tab_cord[indice].distance, tab_cord[indice].courbure);
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