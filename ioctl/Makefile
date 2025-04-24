obj-m =dynamic_ioctl_cap_char_dev.o
#obj-m =dynamic_ioctl_char_dev.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean