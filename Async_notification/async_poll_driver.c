/* ✅ Features:

    1. Async Notification (SIGIO)
    2. Poll/Select System Call Support
    3. Multi-process Awareness

Test execution steps:
    1. Terminal 1: sudo ./testapp_async_user
    2. Terminal 2: echo "trigger" > /dev/async_dev

Pre-requisites:
Create device file
    sudo mknod /dev/asyncpoll c 510 0
    sudo chmod 666 /dev/asyncpoll
*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/sched/signal.h>
#include <linux/fcntl.h>

#define DEVICE_NAME "asyncpoll"
#define CLASS_NAME "asyncpollclass"
#define BUFFER_SIZE 1024

static dev_t dev_num;
static struct cdev cdev;
static struct class *cl;
static DECLARE_WAIT_QUEUE_HEAD(wq);
static char device_buffer[BUFFER_SIZE];
static int buffer_has_data = 0;

static struct fasync_struct *async_queue;

static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off) {
    if (!buffer_has_data)
        return 0;

    if (copy_to_user(buf, device_buffer, len))
        return -EFAULT;

    buffer_has_data = 0;
    return len;
}

static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off) {
    if (len > BUFFER_SIZE)
        return -EINVAL;

    if (copy_from_user(device_buffer, buf, len))
        return -EFAULT;

    buffer_has_data = 1;

    // Wake up poll/select
    wake_up_interruptible(&wq);

    // Async notify
    kill_fasync(&async_queue, SIGIO, POLL_IN);

    return len;
}

static unsigned int my_poll(struct file *f, poll_table *wait) {
    poll_wait(f, &wq, wait);
    return buffer_has_data ? POLLIN | POLLRDNORM : 0;
}

static int my_open(struct inode *i, struct file *f) {
    return 0;
}

static int my_release(struct inode *i, struct file *f) {
    fasync_helper(-1, f, 0, &async_queue);
    return 0;
}

static int my_fasync(int fd, struct file *f, int mode) {
    return fasync_helper(fd, f, mode, &async_queue);
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = my_read,
    .write = my_write,
    .poll = my_poll,
    .open = my_open,
    .release = my_release,
    .fasync = my_fasync,
};

static int __init my_init(void) {
    alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    cdev_init(&cdev, &fops);
    cdev_add(&cdev, dev_num, 1);
    cl = class_create(CLASS_NAME);
    device_create(cl, NULL, dev_num, NULL, DEVICE_NAME);
    printk(KERN_INFO "AsyncPoll: Module loaded\n");
    return 0;
}

static void __exit my_exit(void) {
    device_destroy(cl, dev_num);
    class_destroy(cl);
    cdev_del(&cdev);
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_INFO "AsyncPoll: Module unloaded\n");
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");