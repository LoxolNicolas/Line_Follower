#ifndef I2C_CAPTEUR_HPP
#define I2C_CAPTEUR_HPP

#include "mbed.h"

extern I2CSlave i2cslave; //SLAVE DECLARATION
const int SLAVE_ADDRESS = 0xA0; //SLAVE ADRESS

extern char i2cInfoRequested;

void i2c();
 
#endif