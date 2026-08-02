#include <linux/gpio/consumer.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include "gpio_setup.h"


// MODULE_LICENSE("GPL");
// MODULE_AUTHOR("HADI");
// MODULE_DESCRIPTION("A simple Hello");
// MODULE_VERSION("0.01");


static struct gpio_desc *din_gpio26 = NULL;
static struct gpio_desc *clk_gpio19 = NULL;
static struct gpio_desc *cs_gpio13 = NULL;

int gpio_init(void)
{
    pr_info("GPIO_DIN_26_TEST: Load Module \n");
    din_gpio26 = gpio_to_desc(538);
    if(!din_gpio26)
    {
        pr_err("GPIO_TEST: Failed to accquire gpio 26 \n");
        return -ENODEV;
    }
    gpiod_direction_output(din_gpio26, 0);
    pr_info("GPIO_26_DIN_TEST: GPIO 26 configured \n"); // remember to add the \n so that it doenst get held in the buffer
    
    
    pr_info("GPIO_CLKN_19_TEST: Load Module \n");
    clk_gpio19 = gpio_to_desc(531);
    if(!clk_gpio19)
    {
        pr_err("GPIO_TEST: Failed to accquire gpio 19 \n");
        return -ENODEV;
    }
    gpiod_direction_output(clk_gpio19, 0);
    pr_info("GPIO_19_CLK_TEST: GPIO 19 configured \n"); // remember to add the \n so that it doenst get held in the buffer


    pr_info("GPIO_CS_13_TEST: Load Module \n");
    cs_gpio13 = gpio_to_desc(525);
    if(!cs_gpio13)
    {
        pr_err("GPIO_TEST: Failed to accquire gpio 13 \n");
        return -ENODEV;
    }
    gpiod_direction_output(cs_gpio13, 1);
    pr_info("GPIO_13_CS_TEST: GPIO 13 configured \n"); // remember to add the \n so that it doenst get held in the buffer


    pr_info("ALL (CS, CLK, DIN) CONFIGURED SUCCESSFULLY\n"); // remember to add the \n so that it doenst get held in the buffer
    return 0;
}

void gpio_exit(void)
{
    pr_info("GPIO_DIN_26_TEST: Unloading Module \n");
    if(din_gpio26)
    {
        gpiod_set_value(din_gpio26, 0);
        pr_info("GPIO_TEST: GPIO 26 driven to Low \n");
    }

    pr_info("GPIO_CLK_19_TEST: Unloading Module \n");
    if(clk_gpio19)
    {
        gpiod_set_value(clk_gpio19, 0);
        pr_info("GPIO_19_CLK_TEST: GPIO 19 driven to Low \n");
    }

    pr_info("GPIO_CS_13_TEST: Unloading Module \n");
    if(cs_gpio13)
    {
        gpiod_set_value(cs_gpio13, 1);
        pr_info("GPIO_13_CS_TEST: GPIO 13 driven to high \n");
    }
}


void screen_send_bits(unsigned char address, unsigned char data)
{
    int bits_to_send = (address << 8) | data;

    gpiod_set_value(cs_gpio13, 0); // start chip select

    for(int i=0;i<=15;i++)
    {
        int curr_bit = (bits_to_send >> (15 - i)) & 1;

        gpiod_set_value(din_gpio26, curr_bit);

        gpiod_set_value(clk_gpio19, 1);
        gpiod_set_value(clk_gpio19, 0);
    }

    gpiod_set_value(cs_gpio13, 1);
}

void start_display(void)
{
    pr_info("MAX7219: Waking up display...\n");

    // 1. Decode Mode (Table 4) - Standard numbers for all 8 digits
    screen_send_bits(0x09, 0xFF);

    // 2. Intensity (Table 7) - Medium brightness
    screen_send_bits(0x0A, 0x08);

    // 3. Scan Limit (Table 8) - Activate all 8 digits
    screen_send_bits(0x0B, 0x07);

    // 4. Shutdown Register (Table 3) - Wake up / Normal operation
    screen_send_bits(0x0C, 0x01);

    // 5. Display Test (Table 10) - Ensure test mode is OFF
    screen_send_bits(0x0F, 0x00);

    pr_info("MAX7219: Display initialized and ready.\n");
}
// module_init(gpio_init);
// module_exit(gpio_exit);
