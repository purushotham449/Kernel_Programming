#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/capability.h>

#define DEVICE_NAME "cap_char_dev"
#define CLASS_NAME "cap_class"
#define MEM_SIZE 4096

#define IOCTL_BASE 'W'
#define IOCTL_RETRIEVE      _IOR(IOCTL_BASE, 1, struct my_data *)
#define IOCTL_FILL_UP       _IOW(IOCTL_BASE, 2, struct my_data *)
#define IOCTL_SEND_TO_DEV   _IOW(IOCTL_BASE, 3, struct my_data *)
#define IOCTL_REREAD_DEV    _IOR(IOCTL_BASE, 4, struct my_data *)

struct my_data {
    int i;
    long x;
    char s[100];
};

static dev_t dev_num;
static struct class *dev_class;
static struct cdev my_cdev;
static struct device *dev_device;

static struct my_data device_data;
static char *memory_buffer;
static size_t buffer_size = 0;

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

static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
    if (*offset >= buffer_size)
        return 0;

    if (len > buffer_size - *offset)
        len = buffer_size - *offset;

    if (copy_to_user(buf, memory_buffer + *offset, len))
        return -EFAULT;

    *offset += len;
    pr_info("Read %zu bytes\n", len);
    return len;
}

static ssize_t dev_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
    if (len > MEM_SIZE)
        len = MEM_SIZE;

    if (copy_from_user(memory_buffer, buf, len))
        return -EFAULT;

    buffer_size = len;
    *offset += len;
    pr_info("Wrote %zu bytes\n", len);
    return len;
}

static loff_t dev_llseek(struct file *file, loff_t offset, int whence)
{
    loff_t new_pos = 0;

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

    if (new_pos < 0 || new_pos > MEM_SIZE)
        return -EINVAL;

    file->f_pos = new_pos;
    return new_pos;
}

static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct my_data temp;

    switch (cmd) {
    case IOCTL_RETRIEVE:
        if (copy_to_user((struct my_data *)arg, &device_data, sizeof(struct my_data)))
            return -EFAULT;
        pr_info("IOCTL: Retrieve data\n");
        break;

    case IOCTL_FILL_UP:
        if (!capable(CAP_SYS_ADMIN))
            return -EPERM;

        if (copy_from_user(&temp, (struct my_data *)arg, sizeof(struct my_data)))
            return -EFAULT;

        device_data = temp;
        pr_info("IOCTL: Fill up data\n");
        break;

    case IOCTL_SEND_TO_DEV:
        if (!capable(CAP_SYS_ADMIN))
            return -EPERM;

        if (copy_from_user(&temp, (struct my_data *)arg, sizeof(struct my_data)))
            return -EFAULT;

        snprintf(memory_buffer, MEM_SIZE, "i=%d, x=%ld, s=%s", temp.i, temp.x, temp.s);
        buffer_size = strlen(memory_buffer);
        pr_info("IOCTL: Sent to device\n");
        break;

    case IOCTL_REREAD_DEV:
        sscanf(memory_buffer, "i=%d, x=%ld, s=%99s", &temp.i, &temp.x, temp.s);
        if (copy_to_user((struct my_data *)arg, &temp, sizeof(struct my_data)))
            return -EFAULT;
        pr_info("IOCTL: Reread device data\n");
        break;

    default:
        return -EINVAL;
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
    .llseek = dev_llseek,
};

static int __init mod_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret)
        return ret;

    cdev_init(&my_cdev, &fops);
    ret = cdev_add(&my_cdev, dev_num, 1);
    if (ret)
        goto r_cdev;

    dev_class = class_create(CLASS_NAME);
    if (IS_ERR(dev_class)) {
        ret = PTR_ERR(dev_class);
        goto r_class;
    }

    dev_device = device_create(dev_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(dev_device)) {
        ret = PTR_ERR(dev_device);
        goto r_device;
    }

    memory_buffer = kmalloc(MEM_SIZE, GFP_KERNEL);
    if (!memory_buffer) {
        ret = -ENOMEM;
        goto r_mem;
    }

    pr_info("Driver loaded: %s\n", DEVICE_NAME);
    return 0;

r_mem:
    device_destroy(dev_class, dev_num);
r_device:
    class_destroy(dev_class);
r_class:
    cdev_del(&my_cdev);
r_cdev:
    unregister_chrdev_region(dev_num, 1);
    return ret;
}

static void __exit mod_exit(void)
{
    kfree(memory_buffer);
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("Driver unloaded: %s\n", DEVICE_NAME);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Purushotham");
MODULE_DESCRIPTION("Dynamic Char Driver with IOCTL and CAP checks");