#include <linux/gpio/consumer.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include "spi.h"

static struct gpio_desc *din_gpio26 = NULL;
static struct gpio_desc *clk_gpio19 = NULL;
static struct gpio_desc *cs_gpio13 = NULL;

int spi_init(void)
{
    pr_info("SPI: Initializing GPIOs...\n");
    
    din_gpio26 = gpio_to_desc(538);
    clk_gpio19 = gpio_to_desc(531);
    cs_gpio13  = gpio_to_desc(525);

    if(!din_gpio26 || !clk_gpio19 || !cs_gpio13) {
        pr_err("SPI: Failed to acquire one or more GPIOs\n");
        return -ENODEV;
    }

    gpiod_direction_output(din_gpio26, 0); // DIN idles LOW
    gpiod_direction_output(clk_gpio19, 0); // CLK idles LOW
    gpiod_direction_output(cs_gpio13, 1);  // CS idles HIGH

    return 0;
}

void spi_exit(void)
{
    if(din_gpio26) gpiod_set_value(din_gpio26, 0);
    if(clk_gpio19) gpiod_set_value(clk_gpio19, 0);
    if(cs_gpio13)  gpiod_set_value(cs_gpio13, 1);
    
    pr_info("SPI: GPIOs released and set to safe states.\n");
}

void spi_send_16bit(unsigned short data)
{
    int i;
    
    gpiod_set_value(cs_gpio13, 0); // Wake up display
    udelay(1);

    for(i = 15; i >= 0; i--) {
        int curr_bit = (data >> i) & 1;
        
        gpiod_set_value(din_gpio26, curr_bit); // Set Data bit
        udelay(1);
        
        gpiod_set_value(clk_gpio19, 1); // Clock HIGH
        udelay(1);
        
        gpiod_set_value(clk_gpio19, 0); // Clock LOW
        udelay(1);
    }

    gpiod_set_value(cs_gpio13, 1); // Latch data
    udelay(1);
}
