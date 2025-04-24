obj-m =kernel_module_memory_and_data_alignment.o
#obj-m =kernel_module_advanced_alignment.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean