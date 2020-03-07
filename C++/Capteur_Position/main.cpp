#include "Capteur_Position.hpp"
#include "I2C_Capteur.hpp"


Coordonnee prec = {0.0, 0.0, 0.0, 0.0, 0.0};
Coordonnee act = {0.0, 0.0, 0.0, 0.0, 0.0};

Coordonnee prec_bis = {0.0, 0.0, 0.0, 0.0, 0.0};
Coordonnee act_bis = {0.0, 0.0, 0.0, 0.0, 0.0};

int main()
{
    setup();

    vitesseTimer.start();
    mesureTimer.start();

    while(1)
    {
        switch(nb_tour_circuit)
        {
            case 1:

                tab_cord[0].x = 0;
                tab_cord[0].y = 0;
                tab_cord[0].theta = 0;
                tab_cord[0].distance = 0;
                tab_cord[0].courbure = 0;

                pc.printf("D %f %f %f\n", act.x, act.y, act.courbure); //Discret

                while(act.distance < 900.0f) // FAIRE AVEC BOOLEAN LIGNE ARRIVE FELIX
                {

                    if(mesureTimer.read_ms() > 100) //Le robot tourne a 1000Hz
                    {
                        int type = UpdatePointer(&prec, &act);
                        if(type == 0)
                        {
                            pc.printf("C %f %f\n", act.x, act.y); //continue
                        }

                        if(type == 1) //Point d'interet
                        {
                            pc.printf("D %f %f %f\n", tab_cord[indice].x, tab_cord[indice].y, tab_cord[indice].courbure); //Discret
                            indice++; //numero indice discret
                        }

                        prec = act;
                        mesureTimer.reset();
                    }

                    i2c();
                }

                remplir_tab_I2C(tab_I2C ,tab_cord, indice);

                while(mesureTimer.read() > 1.0f)
                {
                    bt.printf("stop\n"); //Arret des mesures
                }

                break;

            case 2:
                distance_act = 0; //Distance a envoyer
                courbure_act = 0; //Courbure a envoyer

                int indice_tableau_act = 0; //Permet une optimisation lors de la recherche du futur point a prendre en compte

                while(1) //TANT LE ROBOT EST EN FONCTIONNEMENT
                {
                    if(mesureTimer.read_ms() > 1) //Le robot tourne a 1000Hz
                    {
                        UpdatePointer(&prec_bis, &act_bis, &distance_act); //Mise a jour de la distance

                        next_info_I2C(tab_I2C, &distance_act, &courbure_act, &indice_tableau_act); //Renvoit la prochaine courbure et distance

                        prec_bis = act_bis;

                        mesureTimer.reset();
                    }

                    i2c();
                }

                break;
        }
    }

    return 0;
}