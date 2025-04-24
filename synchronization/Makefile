obj-m =dynamic_char_dev_ioctl_spinlock.o
# obj-m =dynamic_char_dev_ioctl_mutex.o
# obj-m =dynamic_char_dev_semaphore.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

