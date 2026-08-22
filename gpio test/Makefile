MODULE_NAME = gpio_test_mod

obj-m := $(MODULE_NAME).o
$(MODULE_NAME)-y := gpio.o spi.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean
