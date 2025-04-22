/* Mutex in Kernel Drivers

A mutex (mutual exclusion) is a locking mechanism to protect critical resources from concurrent access by multiple processes. 
Unlike semaphores, mutexes are optimized for single ownership and simpler synchronization.

Common Kernel Mutex APIs:

    mutex_lock(&lock): Blocks until lock is acquired.
    mutex_lock_interruptible(&lock): Blocks, but returns -EINTR if interrupted by a signal.
    mutex_lock_killable(&lock): Similar, but returns -EINTR only if it’s a fatal signal.
    mutex_trylock(&lock): Tries to acquire the lock, returns 0 on success, non-zero if already locked.
    mutex_is_locked(&lock): Returns non-zero if locked.
    mutex_unlock(&lock): Releases the lock 

Driver Features

    Exposes /dev/mutex_demo
    Supports IOCTLs to:
        Lock with different APIs
        Check lock status
        Unlock
    Includes read/write with mutex protection

Summary of Test Results
Lock Type	                        Signal Sent	        Interrupted?	        Comment
mutex_lock_interruptible()	        SIGINT	            ✅ Yes	               Returns -EINTR
mutex_lock_killable()	            SIGINT	            ❌ No	               Continues waiting
	                                SIGKILL	            ✅ Yes	               Interrupted, returns -EINTR

*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/ioctl.h>
#include <linux/delay.h>

#define DEVICE_NAME "mutex_demo"
#define CLASS_NAME  "mutexcls"

#define IOCTL_LOCK              _IO('M', 1)
#define IOCTL_LOCK_INTERRUPTIBLE _IO('M', 2)
#define IOCTL_LOCK_KILLABLE     _IO('M', 3)
#define IOCTL_TRYLOCK           _IO('M', 4)
#define IOCTL_IS_LOCKED         _IOR('M', 5, int)
#define IOCTL_UNLOCK            _IO('M', 6)

static int major;
static struct class *cls;
static struct cdev my_cdev;

static DEFINE_MUTEX(my_mutex);
static char shared_buffer[100] = "Init";

static int dev_open(struct inode *inode, struct file *file) {
    pr_info("Device opened\n");
    return 0;
}

static int dev_release(struct inode *inode, struct file *file) {
    pr_info("Device released\n");
    return 0;
}

static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *off) {
    mutex_lock(&my_mutex);
    pr_info("Reading protected data\n");
    ssize_t ret = simple_read_from_buffer(buf, len, off, shared_buffer, sizeof(shared_buffer));
    mutex_unlock(&my_mutex);
    return ret;
}

static ssize_t dev_write(struct file *file, const char __user *buf, size_t len, loff_t *off) {
    mutex_lock(&my_mutex);
    pr_info("Writing protected data\n");
    ssize_t ret = simple_write_to_buffer(shared_buffer, sizeof(shared_buffer), off, buf, len);
    shared_buffer[ret] = '\0';
    mutex_unlock(&my_mutex);
    return ret;
}

static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int status = 0;

    switch (cmd) {
    case IOCTL_LOCK:
        pr_info("[IOCTL] mutex_lock()\n");
        mutex_lock(&my_mutex);
        break;

        case IOCTL_LOCK_INTERRUPTIBLE:
        pr_info("Trying mutex_lock_interruptible (5s sleep)...\n");
        msleep(5000);  // Give time to interrupt
        int ret = mutex_lock_interruptible(&my_mutex);
        if (ret) {
            pr_warn("mutex_lock_interruptible interrupted by signal\n");
            return -EINTR;
        }
        pr_info("Got mutex with mutex_lock_interruptible\n");
        break;
    
    case IOCTL_LOCK_KILLABLE:
        pr_info("Trying mutex_lock_killable (5s sleep)...\n");
        msleep(5000);  // Give time to interrupt
        ret = mutex_lock_killable(&my_mutex);
        if (ret) {
            pr_warn("mutex_lock_killable interrupted by fatal signal\n");
            return -EINTR;
        }
        pr_info("Got mutex with mutex_lock_killable\n");
        break;    

    case IOCTL_TRYLOCK:
        pr_info("[IOCTL] mutex_trylock()\n");
        if (!mutex_trylock(&my_mutex))
            return -EBUSY;
        break;

    case IOCTL_IS_LOCKED:
        status = mutex_is_locked(&my_mutex);
        if (copy_to_user((int __user *)arg, &status, sizeof(int)))
            return -EFAULT;
        break;

    case IOCTL_UNLOCK:
        pr_info("[IOCTL] mutex_unlock()\n");
        if (mutex_is_locked(&my_mutex))
            mutex_unlock(&my_mutex);
        break;

    default:
        return -ENOTTY;
    }

    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .read = dev_read,
    .write = dev_write,
    .unlocked_ioctl = dev_ioctl,
};

static int __init mutex_demo_init(void) {
    dev_t dev;
    alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    major = MAJOR(dev);

    cdev_init(&my_cdev, &fops);
    cdev_add(&my_cdev, dev, 1);

    cls = class_create(CLASS_NAME);
    device_create(cls, NULL, dev, NULL, DEVICE_NAME);

    mutex_init(&my_mutex);

    pr_info("Mutex driver loaded\n");
    return 0;
}

static void __exit mutex_demo_exit(void) {
    device_destroy(cls, MKDEV(major, 0));
    class_destroy(cls);
    cdev_del(&my_cdev);
    unregister_chrdev_region(MKDEV(major, 0), 1);
    pr_info("Mutex driver unloaded\n");
}

module_init(mutex_demo_init);
module_exit(mutex_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ChatGPT");
MODULE_DESCRIPTION("Char driver using various mutex locking techniques");