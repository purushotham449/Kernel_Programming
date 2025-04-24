/* Linux kernel synchronization primitives and implement a simple kernel module with a userspace test application to demonstrate their usage.

1. down_interruptible()

This tries to acquire a semaphore. If it cannot, the process sleeps in an interruptible state.

2. wait_for_completion()

Used when one part of code must wait until another part signals completion. Very useful for thread synchronizations.

3. wait_event_interruptible()

Puts the calling process to sleep in an interruptible state until a condition becomes true.

4. wait_event_interruptible_exclusive()

Similar to wait_event_interruptible, but wakes only one process when the condition is met.

*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/semaphore.h>
#include <linux/wait.h>
#include <linux/completion.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/sched.h>

#define DEVICE_NAME "wait_demo"
#define CLASS_NAME "wait_demo_class"
#define MAGIC 'x'

#define IOCTL_DOWN_INTERRUPTIBLE _IO(MAGIC, 1)
#define IOCTL_WAIT_COMPLETION    _IO(MAGIC, 2)
#define IOCTL_COMPLETE           _IO(MAGIC, 3)
#define IOCTL_WAIT_EVENT         _IO(MAGIC, 4)
#define IOCTL_WAKE_EVENT         _IO(MAGIC, 5)
#define IOCTL_WAIT_EVENT_EXCL    _IO(MAGIC, 6)

static int major;
static struct class *wait_class;
static struct device *wait_device;
static struct cdev my_cdev;

static DECLARE_WAIT_QUEUE_HEAD(my_wq);
static DECLARE_WAIT_QUEUE_HEAD(my_exclusive_wq);
static DECLARE_COMPLETION(my_comp);
static struct semaphore sem;
static int condition = 0;

static long sync_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
    case IOCTL_DOWN_INTERRUPTIBLE:
        printk(KERN_INFO "Trying to acquire semaphore using down_interruptible...\n");
        if (down_interruptible(&sem))
            printk(KERN_INFO "Semaphore wait interrupted\n");
        else
            printk(KERN_INFO "Semaphore acquired\n");
        break;

    case IOCTL_WAIT_COMPLETION:
        printk(KERN_INFO "Waiting for completion event...\n");
        wait_for_completion(&my_comp);
        printk(KERN_INFO "Completion event received.\n");
        break;

    case IOCTL_COMPLETE:
        printk(KERN_INFO "Signaling completion event...\n");
        complete(&my_comp);
        break;

    case IOCTL_WAIT_EVENT:
        printk(KERN_INFO "Waiting for condition (interruptible)...\n");
        wait_event_interruptible(my_wq, condition != 0);
        printk(KERN_INFO "Condition met.\n");
        break;

    case IOCTL_WAKE_EVENT:
        printk(KERN_INFO "Waking condition waiters...\n");
        condition = 1;
        wake_up_interruptible(&my_wq);
        wake_up_interruptible(&my_exclusive_wq);
        break;

    case IOCTL_WAIT_EVENT_EXCL:
        printk(KERN_INFO "Waiting for condition (exclusive)...\n");
        wait_event_interruptible_exclusive(my_exclusive_wq, condition != 0);
        printk(KERN_INFO "Exclusive condition met.\n");
        break;

    default:
        return -EINVAL;
    }
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = sync_ioctl,
};

static int __init sync_init(void)
{
    dev_t dev;
    sema_init(&sem, 1);

    if (alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME) < 0)
        return -1;
    major = MAJOR(dev);

    cdev_init(&my_cdev, &fops);
    if (cdev_add(&my_cdev, dev, 1) < 0)
        return -1;

    wait_class = class_create(CLASS_NAME);
    if (IS_ERR(wait_class)) {
        unregister_chrdev_region(MKDEV(major, 0), 1);
        return PTR_ERR(wait_class);
    }
    wait_device = device_create(wait_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    
    printk(KERN_INFO "sync_demo loaded\n");
    return 0;
}

static void __exit sync_exit(void)
{
    device_destroy(wait_class, MKDEV(major, 0));
    class_destroy(wait_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(MKDEV(major, 0), 1);

    printk(KERN_INFO "sync_demo unloaded\n");
}

module_init(sync_init);
module_exit(sync_exit);
MODULE_LICENSE("GPL");