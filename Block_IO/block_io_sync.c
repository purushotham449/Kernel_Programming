/* Linux kernel module example that implements blocking I/O using read() and write() system calls along with synchronization primitives like wait_event_interruptible() and complete() for efficient handling. It ensures that:

    read() blocks until data is available.
    write() provides the data and wakes up any blocking readers.
 
Test running steps:
    1. Terminal 1: sudo ./test_blockio_read
    2. Terminal 2: sudo ./test_blockio_read
    3. Terminal 3: sudo ./test_blockio_write

Expected output: 

    dmesg | tail -n 10
        [] Block IO sync driver loaded
        [] read() called: waiting for data
        [] Data written by user
        [] Data read by user
        [] read() called: waiting for data
        [] read() called: waiting for data
        [] Data written by user
        [] Data read by user
        [] Data read by user

*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/wait.h>
#include <linux/completion.h>
#include <linux/semaphore.h>

#define DEVICE_NAME "blockio"
#define CLASS_NAME  "blockio_class"
#define BUF_SIZE    128

static int major;
static struct class *blockio_class;
static struct device *blockio_device;
static struct cdev blockio_cdev;

static char shared_buf[BUF_SIZE];
static int data_available = 0;

static DECLARE_WAIT_QUEUE_HEAD(read_wq);
static struct semaphore rw_sem;

ssize_t blockio_read(struct file *file, char __user *buf, size_t count, loff_t *ppos);
ssize_t blockio_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos);

ssize_t blockio_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    int ret;

    printk(KERN_INFO "read() called: waiting for data\n");

    // Wait for data to be available
    ret = wait_event_interruptible(read_wq, data_available != 0);
    if (ret)
        return -ERESTARTSYS;

    // Acquire lock to copy safely
    if (down_interruptible(&rw_sem))
        return -ERESTARTSYS;

    if (count > BUF_SIZE) count = BUF_SIZE;

    if (copy_to_user(buf, shared_buf, count)) {
        up(&rw_sem);
        return -EFAULT;
    }

    data_available = 0;  // Reset after read
    up(&rw_sem);

    printk(KERN_INFO "Data read by user\n");
    return count;
}

ssize_t blockio_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    if (down_interruptible(&rw_sem))
        return -ERESTARTSYS;

    if (count > BUF_SIZE) count = BUF_SIZE;

    if (copy_from_user(shared_buf, buf, count)) {
        up(&rw_sem);
        return -EFAULT;
    }

    data_available = 1;  // Mark data as available
    up(&rw_sem);

    wake_up_interruptible(&read_wq);  // Wake any blocking readers

    printk(KERN_INFO "Data written by user\n");
    return count;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = blockio_read,
    .write = blockio_write
};

static int __init blockio_init(void)
{
    dev_t dev;
    sema_init(&rw_sem, 1);

    alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    major = MAJOR(dev);

    cdev_init(&blockio_cdev, &fops);
    cdev_add(&blockio_cdev, dev, 1);

    blockio_class = class_create(CLASS_NAME);
    blockio_device = device_create(blockio_class, NULL, dev, NULL, DEVICE_NAME);

    printk(KERN_INFO "Block IO sync driver loaded\n");
    return 0;
}

static void __exit blockio_exit(void)
{
    device_destroy(blockio_class, MKDEV(major, 0));
    class_destroy(blockio_class);
    unregister_chrdev_region(MKDEV(major, 0), 1);
    cdev_del(&blockio_cdev);
    printk(KERN_INFO "Block IO sync driver unloaded\n");
}

module_init(blockio_init);
module_exit(blockio_exit);
MODULE_LICENSE("GPL");