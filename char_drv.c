#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "simple_char_dev"
#define BUF_SIZE 1024
#define MAJOR_DEV 300
#define MINOR_DEV 0
int count = 1;

static dev_t dev_num;             // Device number (major + minor)
static struct cdev my_cdev;       // Character device structure
static char kernel_buffer[BUF_SIZE];

static int dev_open(struct inode *inode, struct file *file)
{
    pr_info("Device opened\n");
    return 0;
}

static ssize_t dev_read(struct file *file, char __user *user_buffer, size_t len, loff_t *offset)
{
    int bytes_read = 0;

    if (*offset >= BUF_SIZE)
        return 0;

    if (len > BUF_SIZE - *offset)
        len = BUF_SIZE - *offset;

    if (copy_to_user(user_buffer, kernel_buffer + *offset, len))
        return -EFAULT;

    *offset += len;
    bytes_read = len;

    pr_info("Received read request from user space\n");
    pr_info("Read %d bytes\n", bytes_read);
    return bytes_read;
}

static ssize_t dev_write(struct file *file, const char __user *user_buffer, size_t len, loff_t *offset)
{
    int bytes_written = 0;

    if (*offset >= BUF_SIZE)
        return -ENOMEM;

    if (len > BUF_SIZE - *offset)
        len = BUF_SIZE - *offset;

    if (copy_from_user(kernel_buffer + *offset, user_buffer, len))
        return -EFAULT;

    *offset += len;
    bytes_written = len;

    pr_info("Received write request from user space\n");
    pr_info("Wrote %d bytes\n", bytes_written);
    return bytes_written;
}

static int dev_release(struct inode *inode, struct file *file)
{
    pr_info("Device closed\n");
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
};

static int __init char_init(void)
{
    int ret;
    dev_t first;

    // Create a device node
    first = MKDEV(MAJOR_DEV, MINOR_DEV);  // Major = 300 (experimental range), Minor = 0
    dev_num = first;

    // Device node should be reserve with the device file name
    ret = register_chrdev_region(dev_num, count, DEVICE_NAME);
    if (ret < 0) {
        pr_err("Failed to register device number\n");
        return ret;
    }

    // Allocate space for char device in VFS and register file operations with VFS
    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;
	
    // Attach char device with device node and allow file operations
    ret = cdev_add(&my_cdev, dev_num, 1);
    if (ret < 0) {
        pr_err("Failed to add char device\n");
        unregister_chrdev_region(dev_num, 1);
        return ret;
    }

    pr_info("Char device registered: Major %d Minor %d\n", MAJOR(dev_num), MINOR(dev_num));
    return 0;
}

static void __exit char_exit(void)
{
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("Char device unregistered\n");
}

module_init(char_init);
module_exit(char_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Purushotham");
MODULE_DESCRIPTION("Simple Character Device using mkdev & file operations");
