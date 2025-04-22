/* 
Semaphore:

down() – blocks unconditionally until the semaphore is available
down_interruptible() – blocks but can be interrupted by signals
down_trylock() – doesn't block; immediately returns if it can't get the lock.

Summary Table
API     	                Blocks?	        Preemptible?	    Interruptible?      Return Value
down()	                      Yes	            No	                No	            void (always blocks)
down_interruptible()	      Yes	            No	                Yes	            0 on success, -ERESTARTSYS if signal
down_trylock()	              No	            N/A	                N/A	            0 success, non-zero fail

test Steps:
echo "Critical Code" | sudo tee /dev/sem_variants 
sudo cat /dev/sem_variants

ps aux | grep 'cat /dev/sem_variants'   # Get PID
sudo kill -SIGINT <pid>                 # Send Ctrl+C equivalent

*/
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/semaphore.h>
#include <linux/sched/signal.h>
#include <linux/delay.h>

#define DEVICE_NAME "sem_variants"
#define CLASS_NAME  "semcls"

static int major;
static struct class* cls;
static struct cdev my_cdev;

static struct semaphore sem;
static char shared_buffer[100] = "Initial Data";

ssize_t read_down(struct file *filep, char __user *buf, size_t len, loff_t *offset);
ssize_t read_interruptible(struct file *filep, char __user *buf, size_t len, loff_t *offset);
ssize_t read_try_lock(struct file *filep, char __user *buf, size_t len, loff_t *offset);

static int dev_open(struct inode *inodep, struct file *filep) {
    pr_info("Device opened\n");
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep) {
    pr_info("Device released\n");
    return 0;
}

// down() variant
ssize_t read_down(struct file *filep, char __user *buf, size_t len, loff_t *offset) {
    pr_info("[down] Trying to acquire semaphore\n");
    down(&sem);  // blocks unconditionally
    pr_info("[down] Got semaphore, reading...\n");
    ssize_t ret = simple_read_from_buffer(buf, len, offset, shared_buffer, sizeof(shared_buffer));
    up(&sem);
    return ret;
}

ssize_t read_interruptible(struct file *filep, char __user *buf, size_t len, loff_t *offset) {
    pr_info("[interruptible] Trying to acquire semaphore\n");

    // Attempt to acquire immediately
    if (down_interruptible(&sem)) {
        pr_warn("[interruptible] Interrupted by signal!\n");
        return -ERESTARTSYS;
    }

    pr_info("[interruptible] Got semaphore, sleeping...\n");
    ssleep(5);  // Sleep after acquiring semaphore

    ssize_t ret = simple_read_from_buffer(buf, len, offset, shared_buffer, sizeof(shared_buffer));
    up(&sem);
    return ret;
}

// down_trylock() variant
ssize_t read_try_lock(struct file *filep, char __user *buf, size_t len, loff_t *offset) {
    pr_info("[trylock] Trying to acquire semaphore\n");
    if (down_trylock(&sem)) {
        pr_warn("[trylock] Could not acquire, resource busy!\n");
        return -EBUSY;
    }
    pr_info("[trylock] Got semaphore, reading...\n");
    ssize_t ret = simple_read_from_buffer(buf, len, offset, shared_buffer, sizeof(shared_buffer));
    up(&sem);
    return ret;
}

static ssize_t dev_write(struct file *filep, const char __user *buf, size_t len, loff_t *offset) {
    pr_info("Write request received\n");
    down(&sem);  // Simple protection
    ssize_t ret = simple_write_to_buffer(shared_buffer, sizeof(shared_buffer), offset, buf, len);
    pr_info("Updated buffer: %s\n", shared_buffer);
    up(&sem);
    return ret;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .write = dev_write,
    .read = read_interruptible // <--- Switch here to test: read_down, read_trylock
    // .read = read_down
    // .read = read_try_lock
};

static int __init sem_demo_init(void) {
    dev_t dev;
    alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    major = MAJOR(dev);

    cdev_init(&my_cdev, &fops);
    cdev_add(&my_cdev, dev, 1);

    cls = class_create(CLASS_NAME);
    device_create(cls, NULL, dev, NULL, DEVICE_NAME);

    sema_init(&sem, 1);
    pr_info("Semaphore demo driver loaded\n");
    return 0;
}

static void __exit sem_demo_exit(void) {
    device_destroy(cls, MKDEV(major, 0));
    class_destroy(cls);
    cdev_del(&my_cdev);
    unregister_chrdev_region(MKDEV(major, 0), 1);
    pr_info("Semaphore demo driver unloaded\n");
}

module_init(sem_demo_init);
module_exit(sem_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ChatGPT");
MODULE_DESCRIPTION("Semaphore variants: down, down_interruptible, down_trylock");