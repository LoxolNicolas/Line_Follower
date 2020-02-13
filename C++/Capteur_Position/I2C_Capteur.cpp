#include "I2C_Capteur.hpp"
#include "Capteur_Position.hpp"

I2CSlave i2cslave(D4, D5); //SLAVE DECLARATION
char i2cInfoRequested = 0;

void i2cSetup()
{
	i2cslave.address(SLAVE_ADDRESS);
}

void i2c()
{
	int operation = i2cslave.receive(); //IF THE SLAVE HAS BEEN ADRESSED

	switch(operation)
	{
		case I2CSlave::ReadAddressed: //MASTER IS READING AT THE SLAVE ADRESS
		{
			//bt.printf("read\n");
			switch(i2cInfoRequested)
			{
				case 0: // SPEED selected
					i2cslave.write((char *) &vitesse , sizeof(vitesse));
					//bt.printf("writen speed: %f\n", vitesse);
				break;

				case 1: // ANGLE selected
					i2cslave.write((char *) &currentAngle , sizeof(currentAngle));
					//bt.printf("writen angle: %f\n", currentAngle);
				break;

				case 2:
					// TODO changer les infos renvoyées par la distance avant le prochain point et la prochaine courbure
					i2cslave.write((char *) tab_cord , sizeof(tab_cord[0]));
					//pc.printf("writen map info\n");
				break;
			}

			break;
		}

		case I2CSlave::WriteAddressed: //MASTER IS WRITING TO THE SLAVE ADRESS
		{
			//bt.printf("write\n");
			i2cslave.read(&i2cInfoRequested, 1);

			break;
		}

		default:
			break;
	}
}

