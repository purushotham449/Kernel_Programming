obj-m =dynamic_char_dev.o
#obj-m =char_drv.o
#obj-m =mod.o
#obj-m +=depmod.o
#obj-m +=mod_params.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

