#include <linux/module.h> // kmod interface (Constants & DataTypes)
#include <linux/moduleparam.h> // Module Parameters
#include <linux/version.h> // kernel version stamp 
#include <linux/kernel.h> // Resolve kernel symbol calls (kernel global functions)
#include <linux/init.h> // Inline functions of initialization and exit routines of module functions

int val =200;
void func(void);

//module_param(val, int, S_IRUGO); // Readonly
module_param(val, int, S_IRUGO | S_IWUSR); // Read + write only
MODULE_PARM_DESC(val, "Initialise me at Insertion time");

void func(void)
{
	printk("function invoked!!!!!!!!!!!!!\n");
	printk("val = %d\n", val);
}

static __init int myinit(void) // init kernel attribute
{
	printk("Calling Kernel Symbol!!!!!!!!!!!\n");
	func();
	return 0;
}

static __exit void myexit(void) // exit kernel attribute
{
	printk("Cleanup invoked!!!!!!!!!!!!\n");
}

module_init(myinit); // Register the module as initialization routine
module_exit(myexit); // Register the module as exit routine

/* kernel module comments */
MODULE_AUTHOR("Purushotham");
MODULE_DESCRIPTION("Dependency Module");
MODULE_LICENSE("GPL");
