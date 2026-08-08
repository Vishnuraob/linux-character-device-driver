obj-m += prochardev.o

prochardev-objs := \
	src/main.o \
	src/file_ops.o \
	src/memory.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
