#ifndef SPI_CAPTEUR_HPP
#define SPI_CAPTEUR_HPP

#include "mbed.h"

extern SPI spi; // MOSI, MISO, SCLK
extern DigitalOut ss; //SS

void spi_com_begin();
void spi_com_end();
char spi_read_reg(char reg_addr);
void spi_write_reg(char reg_addr, char data);
void performStartup(void);

#endif
