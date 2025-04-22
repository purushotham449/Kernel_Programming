/* Spinlock Concepts

Spinlocks are used in the Linux kernel to protect shared resources in SMP systems, especially in atomic or interrupt contexts.
Variants:

    spin_lock(): Basic lock; assumes no interrupt context.
    spin_lock_irqsave(): Disables local interrupts and saves current state.
    spin_lock_irq(): Disables local interrupts (no save).
    spin_lock_bh(): Disables bottom halves (softirq).

Explanation of Spinlock Variants

Spinlock Variant	        Use Case	                                    Interrupt Handling
--------------------------------------------------------------------------------------------------------
spin_lock()	                Simple critical section protection	            No interrupt control

spin_lock_irq()	            Disables local CPU interrupts                   Disables local IRQs
                            (no restore) to avoid deadlocks with ISRs

spin_lock_irqsave()	        Same as above, but saves current flags          Saves & disables IRQs
                            for later restore

spin_lock_bh()	            Disables bottom halves (soft IRQs) on           Disables soft IRQs
                            current CPU

*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <linux/ioctl.h>
#include <linux/delay.h>

#define DEVICE_NAME "spinlock_dev"
#define CLASS_NAME  "spincls"

#define SPIN_IOCTL_BASE 0xF0
#define SPIN_LOCK            _IO(SPIN_IOCTL_BASE, 1)
#define SPIN_LOCK_IRQSAVE    _IO(SPIN_IOCTL_BASE, 2)
#define SPIN_LOCK_IRQ        _IO(SPIN_IOCTL_BASE, 3)
#define SPIN_LOCK_BH         _IO(SPIN_IOCTL_BASE, 4)

static int major;
static struct class* cls;
static struct cdev my_cdev;

static char shared_data[100] = "Spinlock Protected";
static spinlock_t my_lock;

static int dev_open(struct inode *inode, struct file *file) {
    pr_info("Device opened\n");
    return 0;
}

static int dev_release(struct inode *inode, struct file *file) {
    pr_info("Device closed\n");
    return 0;
}

static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    unsigned long flags;

    switch (cmd) {
        case SPIN_LOCK:
            pr_info("[spin_lock] Acquiring lock\n");
            spin_lock(&my_lock);
            msleep(2000);
            pr_info("[spin_lock] Data = %s\n", shared_data);
            spin_unlock(&my_lock);
            break;

        case SPIN_LOCK_IRQSAVE:
            pr_info("[irqsave] Acquiring lock\n");
            spin_lock_irqsave(&my_lock, flags);
            msleep(2000);
            pr_info("[irqsave] Data = %s\n", shared_data);
            spin_unlock_irqrestore(&my_lock, flags);
            break;

        case SPIN_LOCK_IRQ:
            pr_info("[irq] Acquiring lock\n");
            local_irq_disable();  // manually disable before locking
            spin_lock(&my_lock);
            msleep(2000);
            pr_info("[irq] Data = %s\n", shared_data);
            spin_unlock(&my_lock);
            local_irq_enable();
            break;

        case SPIN_LOCK_BH:
            pr_info("[bh] Acquiring lock\n");
            spin_lock_bh(&my_lock);
            msleep(2000);
            pr_info("[bh] Data = %s\n", shared_data);
            spin_unlock_bh(&my_lock);
            break;

        default:
            return -EINVAL;
    }
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = dev_ioctl,
    .open = dev_open,
    .release = dev_release,
};

static int __init spin_init(void) {
    dev_t dev;
    alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    major = MAJOR(dev);

    cdev_init(&my_cdev, &fops);
    cdev_add(&my_cdev, dev, 1);

    cls = class_create(CLASS_NAME);
    device_create(cls, NULL, dev, NULL, DEVICE_NAME);

    spin_lock_init(&my_lock);
    pr_info("Spinlock driver loaded\n");
    return 0;
}

static void __exit spin_exit(void) {
    device_destroy(cls, MKDEV(major, 0));
    class_destroy(cls);
    cdev_del(&my_cdev);
    unregister_chrdev_region(MKDEV(major, 0), 1);
    pr_info("Spinlock driver unloaded\n");
}

module_init(spin_init);
module_exit(spin_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ChatGPT");
MODULE_DESCRIPTION("Simple spinlock char driver with IOCTL control");