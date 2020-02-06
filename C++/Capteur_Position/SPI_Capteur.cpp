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

void spi_upload_firmware()
{
    // send the firmware to the chip, cf p.18 of the datasheet
    //pc.printf("Uploading firmware...\r\n");

    //Write 0 to Rest_En bit of Config2 register to disable Rest mode.
    spi_write_reg(Config2, 0x00);

    // write 0x1d in SROM_enable reg for initializing
    spi_write_reg(SROM_Enable, 0x1d);

    // wait for more than one frame period
    wait_ms(10); // assume that the frame rate is as low as 100fps... even if it should never be that low

    // write 0x18 to SROM_enable to start SROM download
    spi_write_reg(SROM_Enable, 0x18);

    spi_com_begin();

    // write the SROM file (=firmware data)
    spi.write(SROM_Load_Burst | 0b10000000); // write burst destination adress
    wait_us(15);

    // send all chars of the firmware
    unsigned char c;
    
    for(uint16_t i = 0; i < firmware_length; i++) 
    {
        c = firmware_data[i];
        spi.write(c);
        wait_us(15);
    }

    spi_com_end();
    wait_us(160);

    // perform SROM CRC test
    spi_write_reg(SROM_Enable, 0x15); // start CRC test
    wait_ms(10);
    int16_t crc = 0;
    crc |= (spi_read_reg(Data_Out_Upper) & 0xFF);
    crc = crc << 8;
    crc |= (spi_read_reg(Data_Out_Lower) & 0xFF);
    //pc.printf("CRC Check: %04X\r\n", crc);


    //Read the SROM_ID register to verify the ID before any other register reads or writes.
    char result = spi_read_reg(SROM_ID);
    //printf("SROM_ID: %02X\r\n", result);

    //Write 0x00 to Config2 register for wired mouse or 0x20 for wireless mouse design.
    spi_write_reg(Config2, 0x00);

    // set initial CPI resolution
    spi_write_reg(Config1, 0x77); //RESOLUTION A 12000 CPI
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
    
    spi_upload_firmware();
    wait_ms(10);
}