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

program_test_1: all
	-sudo rmmod max_screen_driver
	sudo insmod max_screen_driver.ko
	sudo chmod 666 /dev/max_screen
	gcc program_test_1.c -o program_test_1
	./program_test_1

digital_clock: all
	-sudo rmmod max_screen_driver
	sudo insmod max_screen_driver.ko
	sudo chmod 666 /dev/max_screen
	gcc digital_clock.c -o digital_clock
	./digital_clock
