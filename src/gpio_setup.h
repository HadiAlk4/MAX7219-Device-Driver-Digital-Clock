#ifndef GPIO_SETUP_H
#define GPIO_SETUP_H

int gpio_init(void);
void gpio_exit(void);
void start_display(void);
void screen_send_bits(unsigned char address, unsigned char data);

#endif
