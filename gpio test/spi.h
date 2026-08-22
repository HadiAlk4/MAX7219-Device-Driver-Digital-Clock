#ifndef SPI_H
#define SPI_H

int spi_init(void);
void spi_exit(void);
void spi_send_16bit(unsigned short data);

#endif
