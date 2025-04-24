#include <linux/module.h> // kmod interface (Constants & DataTypes)
#include <linux/version.h> // kernel version stamp
#include <linux/kernel.h> // Resolve kernel symbol calls (kernel global functions)
#include <linux/init.h> // Inline functions of initialization and exit routines of module functions

extern void func(void);

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

module_init(myinit);
module_exit(myexit);

/* kernel module comments */
MODULE_AUTHOR("Purushotham");
MODULE_DESCRIPTION("Dependency Module");
MODULE_LICENSE("GPL");

