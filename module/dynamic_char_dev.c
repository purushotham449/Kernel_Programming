#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "dynamic_char_dev"
#define BUF_SIZE 1024

static dev_t dev_num;              // Allocated device number
static struct cdev my_cdev;        // Character device structure
static struct class *dev_class;    // Device class
static struct device *dev_device;  // Device

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

    // Allocate device number
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("Failed to allocate device number\n");
        return ret;
    }

    // Create device class
    dev_class = class_create(DEVICE_NAME);
    if (IS_ERR(dev_class)) {
        unregister_chrdev_region(dev_num, 1);
        pr_err("Failed to create class\n");
        return PTR_ERR(dev_class);
    }

    // Create device
    dev_device = device_create(dev_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(dev_device)) {
        class_destroy(dev_class);
        unregister_chrdev_region(dev_num, 1);
        pr_err("Failed to create device\n");
        return PTR_ERR(dev_device);
    }

    // Initialize the cdev structure and link it with file operations
    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;

    // Attach char device with device node and allow the file operations
    ret = cdev_add(&my_cdev, dev_num, 1);
    if (ret < 0) {
        device_destroy(dev_class, dev_num);
        class_destroy(dev_class);
        unregister_chrdev_region(dev_num, 1);
        pr_err("Failed to add char device\n");
        return ret;
    }

    pr_info("Dynamic char device registered: Major %d, Minor %d\n", MAJOR(dev_num), MINOR(dev_num));
    return 0;
}

static void __exit char_exit(void)
{
    cdev_del(&my_cdev);
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    unregister_chrdev_region(dev_num, 1);
    pr_info("Dynamic char device unregistered\n");
}

module_init(char_init);
module_exit(char_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Purushotham");
MODULE_DESCRIPTION("Dynamic Character Device using alloc_chrdev_region()");
