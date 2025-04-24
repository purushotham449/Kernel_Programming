#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/ioctl.h>

#define DEVICE_NAME "dynamic_ioctl_dev"
#define IOCTL_BASE 'D'

#define IOCTL_FILL_ZERO _IO(IOCTL_BASE, 1)
#define IOCTL_FILL_CHAR _IOW(IOCTL_BASE, 2, char)
#define IOCTL_SET_SIZE  _IOW(IOCTL_BASE, 3, int)
#define IOCTL_GET_SIZE  _IOR(IOCTL_BASE, 4, int)
#define IOCTL_MAX_CMDS  _IOR(IOCTL_BASE, 5, int)

#define MAX_MEMORY_SIZE 4000

static dev_t dev_num;
static struct class *dev_class;
static struct cdev my_cdev;

static char *device_buffer;
static size_t buffer_size = 1024;

static int dev_open(struct inode *inode, struct file *file)
{
    pr_info("Device opened\n");
    return 0;
}

static int dev_release(struct inode *inode, struct file *file)
{
    pr_info("Device closed\n");
    return 0;
}

static ssize_t dev_read(struct file *file, char __user *user_buf, size_t len, loff_t *offset)
{
    if (*offset >= buffer_size)
        return 0;
    if (*offset + len > buffer_size)
        len = buffer_size - *offset;

    if (copy_to_user(user_buf, device_buffer + *offset, len))
        return -EFAULT;

    *offset += len;
    pr_info("Read %zu bytes\n", len);
    return len;
}

static ssize_t dev_write(struct file *file, const char __user *user_buf, size_t len, loff_t *offset)
{
    if (*offset >= buffer_size)
        return -ENOSPC;
    if (*offset + len > buffer_size)
        len = buffer_size - *offset;

    if (copy_from_user(device_buffer + *offset, user_buf, len))
        return -EFAULT;

    *offset += len;
    pr_info("Wrote %zu bytes\n", len);
    return len;
}

static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int tmp;
    char fill;

    switch (cmd) {
        case IOCTL_FILL_ZERO:
            memset(device_buffer, 0, buffer_size);
            pr_info("Filled with zeros\n");
            break;

        case IOCTL_FILL_CHAR:
            if (copy_from_user(&fill, (char __user *)arg, sizeof(char)))
                return -EFAULT;
            memset(device_buffer, fill, buffer_size);
            pr_info("Filled with char '%c'\n", fill);
            break;

        case IOCTL_SET_SIZE:
            if (copy_from_user(&tmp, (int __user *)arg, sizeof(int)))
                return -EFAULT;
            if (tmp <= 0 || tmp > MAX_MEMORY_SIZE)
                return -EINVAL;
            buffer_size = tmp;
            pr_info("Buffer size set to %d bytes\n", tmp);
            break;

        case IOCTL_GET_SIZE:
            if (copy_to_user((int __user *)arg, &buffer_size, sizeof(int)))
                return -EFAULT;
            pr_info("Returned buffer size %zu\n", buffer_size);
            break;

        case IOCTL_MAX_CMDS:
            tmp = 5;
            if (copy_to_user((int __user *)arg, &tmp, sizeof(int)))
                return -EFAULT;
            pr_info("Returned max command count: %d\n", tmp);
            break;

        default:
            return -EINVAL;
    }

    return 0;
}

static loff_t dev_llseek(struct file *file, loff_t offset, int whence)
{
    loff_t new_pos;

    switch (whence) {
        case SEEK_SET:
            new_pos = offset;
            break;
        case SEEK_CUR:
            new_pos = file->f_pos + offset;
            break;
        case SEEK_END:
            new_pos = buffer_size + offset;
            break;
        default:
            return -EINVAL;
    }

    if (new_pos < 0 || new_pos > buffer_size)
        return -EINVAL;

    file->f_pos = new_pos;
    pr_info("Seek to %lld\n", new_pos);
    return new_pos;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .read = dev_read,
    .write = dev_write,
    .llseek = dev_llseek,
    .unlocked_ioctl = dev_ioctl
};

static int __init char_dev_init(void)
{
    if (alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME) < 0)
        return -1;

    cdev_init(&my_cdev, &fops);

    if (cdev_add(&my_cdev, dev_num, 1) < 0)
        goto r_cdev;

    dev_class = class_create(DEVICE_NAME);
    if (IS_ERR(dev_class))
        goto r_class;

    if (device_create(dev_class, NULL, dev_num, NULL, DEVICE_NAME) == NULL)
        goto r_device;

    device_buffer = kzalloc(MAX_MEMORY_SIZE, GFP_KERNEL);
    if (!device_buffer)
        goto r_alloc;

    pr_info("Dynamic IOCTL Char Driver Loaded: Major=%d Minor=%d\n",
            MAJOR(dev_num), MINOR(dev_num));
    return 0;

r_alloc:
    device_destroy(dev_class, dev_num);
r_device:
    class_destroy(dev_class);
r_class:
    cdev_del(&my_cdev);
r_cdev:
    unregister_chrdev_region(dev_num, 1);
    return -1;
}

static void __exit char_dev_exit(void)
{
    kfree(device_buffer);
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("Dynamic IOCTL Char Driver Removed\n");
}

module_init(char_dev_init);
module_exit(char_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Purushotham");
MODULE_DESCRIPTION("Dynamic Char Driver with IOCTL and R/W support");