obj-m =char_drv_hrtimer_hi_tasklet_spinlock.o
# obj-m =char_drv_hrtimer_tasklet_bh.o
# obj-m =char-drv_timer-interrupts.o
# obj-m =char_drv_basic_interrupts.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

