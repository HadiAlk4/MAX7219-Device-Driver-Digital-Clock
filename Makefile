# Name of your final kernel module
MODULE_NAME = max_screen_driver


# List the object files needed to compose the final module
$(MODULE_NAME)-y := src/char_driver.o src/gpio_setup.o

# Kbuild syntax tells the kernel build system this is a loadable module
obj-m := $(MODULE_NAME).o

# Path to the kernel headers/build directory
KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean
