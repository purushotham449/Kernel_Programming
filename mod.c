/* kmod subsystem --> Which is responsible for initializing/removing/maintaining the kernel modules */

#include <linux/module.h> // kmod interface (Constants & DataTypes)
#include <linux/version.h> // kernel version stamp 
#include <linux/kernel.h> // Resolve kernel symbol calls (kernel global functions)
#include <linux/init.h> // Inline functions of initialization and exit routines of module functions

int init_mod(void);
void cleanup_mod(void);

int init_mod(void) // Initialization routine/Constructor
{
	printk("Module inserted!!!!!!!!!!!\n");
	return 0;
}

void cleanup_mod(void) // Cleanup routine/destructor
{
	printk("Module Removed!!!!!!!!!!!!\n");
}

module_init(init_mod); // Register the module as initialization routine
module_exit(cleanup_mod); // Register the module as exit routine

/* kernel module comments */
MODULE_AUTHOR("Purushotham");
MODULE_DESCRIPTION("First Module");
MODULE_LICENSE("GPL");
