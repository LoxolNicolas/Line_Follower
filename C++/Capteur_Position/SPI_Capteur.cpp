#include "SPI_Capteur.hpp"
#include "Config_capteur.hpp"

extern Serial pc;

SPI spi(D11, D12, D13); // MOSI, MISO, SCLK
DigitalOut ss(A3); //SS

void spi_com_begin()
{
    //spi.lock();
    ss = 0; //NCS Non Chip Select
}

void spi_com_end()
{
    //spi.unlock();
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

void performStartup(void)
{
    spi_com_end(); // ensure that the serial port is reset
    spi_com_begin(); // ensure that the serial port is reset
    spi_com_end(); // ensure that the serial port is reset
    spi_write_reg(Power_Up_Reset, 0x5a); // force reset
    wait_us(1000 * 50); // wait for it to reboot
    // read registers 0x02 to 0x06 (and discard the data)
    spi_read_reg(Motion);
    spi_read_reg(Delta_X_L);
    spi_read_reg(Delta_X_H);
    spi_read_reg(Delta_Y_L);
    spi_read_reg(Delta_Y_H);
    // upload the firmware

}