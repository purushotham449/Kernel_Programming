/*  simple Linux kernel module using a workqueue

What We'll Implement

    Kernel module that:

        1. Initializes a work_struct.
        2. Schedules it from a timer (simulating an event like IRQ).
        3. Executes work in process context.
        4. Logs messages and maintains a counter.
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/timer.h>

static char *devname = "workqueue_demo";
static struct workqueue_struct *my_wq;

struct my_work_t {
    struct work_struct work;
    int data;
};

static struct timer_list my_timer;
static int counter = 0;

// Workqueue handler
static void my_work_handler(struct work_struct *work)
{
    struct my_work_t *my_work = container_of(work, struct my_work_t, work);

    pr_info("%s: Workqueue running in process context with data: %d\n", devname, my_work->data);

    // Clean up dynamically allocated work
    kfree(my_work);
}

// Timer callback to schedule work
static void my_timer_callback(struct timer_list *t)
{
    struct my_work_t *work;

    pr_info("%s: Timer callback triggered, scheduling work\n", devname);

    // Allocate and prepare work item
    work = kmalloc(sizeof(struct my_work_t), GFP_KERNEL);
    if (!work)
        return;

    INIT_WORK(&work->work, my_work_handler);
    work->data = ++counter;

    queue_work(my_wq, &work->work);

    // Restart timer for continuous triggering
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(2000));  // every 2 seconds
}

static int __init workqueue_demo_init(void)
{
    pr_info("%s: Init\n", devname);

    // Create single-threaded workqueue (use alloc_workqueue for multi-threaded)
    my_wq = create_singlethread_workqueue("my_wq");
    if (!my_wq)
        return -ENOMEM;

    // Setup and start timer
    timer_setup(&my_timer, my_timer_callback, 0);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(2000));

    return 0;
}

static void __exit workqueue_demo_exit(void)
{
    pr_info("%s: Exit\n", devname);

    del_timer_sync(&my_timer);
    flush_workqueue(my_wq);
    destroy_workqueue(my_wq);
}

module_init(workqueue_demo_init);
module_exit(workqueue_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ChatGPT");
MODULE_DESCRIPTION("Simple Linux kernel module using workqueue");