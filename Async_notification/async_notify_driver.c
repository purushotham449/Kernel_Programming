/*  Asynchronous notification:

Asynchronous notification in the Linux kernel allows a user-space process to be notified asynchronously (i.e., via signals) 
when data becomes available on a file descriptor—commonly used in device drivers.

This is particularly useful when a process is blocked or polling a device, 
and the kernel needs to notify the process without the process constantly checking (polling) the device

Concept                 |           Explanation
fasync_helper()         |           Adds/removes a file from async queue
kill_fasync()           |           Sends SIGIO to all interested processes
SIGIO                   |           Sent to user-space when device activity happens
fcntl()                 |           User-space API to enable async notification

Test execution steps:
    1. Terminal 1: sudo ./testapp_async_user
    2. Terminal 2: echo "trigger" > /dev/async_dev

Expected output:
    Waiting for async signal (SIGIO)...
    Received SIGIO: async data is ready!

Pre-requisites:
    Create device file
        sudo mknod /dev/async_dev c 510 0
        sudo chmod 666 /dev/async_dev
*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/fcntl.h>
#include <linux/sched/signal.h>

#define DEVICE_NAME "async_dev"
static int major;

// Kernel maintains a list of processes using fasync_struct linked list.
static struct fasync_struct *async_queue;

static ssize_t async_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    printk(KERN_INFO "async_write: data written\n");

    // Notify user-space via signal
    if (async_queue)
        // When an event happens (e.g., new data), the kernel calls
        kill_fasync(&async_queue, SIGIO, POLL_IN);

    return count;
}

static int async_fasync(int fd, struct file *filp, int mode) {
    return fasync_helper(fd, filp, mode, &async_queue);
}

static int async_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "Device opened\n");
    return 0;
}

static int async_release(struct inode *inode, struct file *file) {
    async_fasync(-1, file, 0); // remove from async list
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = async_open,
    .write = async_write,
    .release = async_release,
    // File supports async notification by implementing below
    .fasync = async_fasync,
};

static int __init async_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    printk(KERN_INFO "Async module loaded with major %d\n", major);
    return 0;
}

static void __exit async_exit(void) {
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "Async module unloaded\n");
}

module_init(async_init);
module_exit(async_exit);
MODULE_LICENSE("GPL");