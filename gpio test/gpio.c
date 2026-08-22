#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include "spi.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HADI");
MODULE_DESCRIPTION("Hardware SPI Screen Test");
MODULE_VERSION("1.0");

static int __init gpio_test_init(void)
{
    pr_info("GPIO_TEST: Loading Module and Initializing SPI\n");
    
    if (spi_init() != 0) {
        return -ENODEV;
    }

    msleep(100);

    pr_info("GPIO_TEST: Sending MAX7219 Initialization Sequence...\n");

    // 1. Disable display
    spi_send_16bit(0x0C00);
    
    // 2. Configure the display settings
    spi_send_16bit(0x0900); // Decode Mode: No decode
    spi_send_16bit(0x0A0F); // Intensity: Max brightness
    spi_send_16bit(0x0B07); // Scan Limit: All 8 digits
    
    // 3. Enable the display (Wake up from Shutdown)
    spi_send_16bit(0x0C01);

    pr_info("GPIO_TEST: Firing Display Test ON!\n");
    // Register 0x0F (Display Test), Data 0x01 (ON)
    spi_send_16bit(0x0F01);
    
    return 0;
}

static void __exit gpio_test_exit(void)
{
    pr_info("GPIO_TEST: Turning off screen and unloading...\n");
    
    // Turn off Display Test
    spi_send_16bit(0x0F00);
    // Put the chip back to sleep (Shutdown)
    spi_send_16bit(0x0C00);
    
    spi_exit();
}

module_init(gpio_test_init);
module_exit(gpio_test_exit);




// #include <linux/init.h>
// #include <linux/module.h>
// #include <linux/delay.h>
// #include "spi.h"

// MODULE_LICENSE("GPL");
// MODULE_AUTHOR("HADI");
// MODULE_DESCRIPTION("Hardware SPI 'UUUU' Testing (Appendix D)");
// MODULE_VERSION("1.0");

// static int __init gpio_test_init(void)
// {
//     int i;
    
//     pr_info("GPIO_TEST: Loading Module and Initializing SPI\n");
    
//     if (spi_init() != 0) {
//         return -ENODEV;
//     }

//     pr_info("GPIO_TEST: Sending 'UUUU' pattern 20 times...\n");

//     // Loop 20 times to send the 'UUUU' message as per Appendix D
//     for (i = 0; i < 20; i++) {
//         // 'U' is 0x55 in ASCII (01010101 in binary).
//         // Sending 0x5555 sends two 'U's.
//         // Sending it twice sends four 'U's ("UUUU").
//         spi_send_16bit(0x5555);
//         spi_send_16bit(0x5555);
        
//         // Brief 100-microsecond pause so the pulses are separated
//         // clearly on the oscilloscope screen.
//         udelay(100);
//     }
    
//     pr_info("GPIO_TEST: Hardware test pattern complete.\n");

//     return 0;
// }

// static void __exit gpio_test_exit(void)
// {
//     spi_exit();
//     pr_info("GPIO_TEST: Unloading Module\n");
// }

// module_init(gpio_test_init);
// module_exit(gpio_test_exit);
