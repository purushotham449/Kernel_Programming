obj-m =dynamic_char_dev_ioctl_spinlock.o
# obj-m =dynamic_char_dev_ioctl_mutex.o
# obj-m =dynamic_char_dev_semaphore.o
# obj-m =dynamic_ioctl_cap_char_dev.o
#obj-m =dynamic_ioctl_char_dev.o
#obj-m =dynamic_char_dev.o
#obj-m =char_drv.o
#obj-m =mod.o
#obj-m +=depmod.o
#obj-m +=mod_params.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

