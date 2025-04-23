/* Simple interrupts using below

1. tasklet_hi_schedule()
2. Spinlock Protection

i.e, High-priority tasklet demo with spinlock protection and hrtimer IRQ simulation

Feature                             |       Code Element
High-priority tasklet               |       tasklet_hi_schedule()
Spinlock protection                 |       spin_lock_irqsave() / spin_unlock_irqrestore()
Simulated IRQ                       |       hrtimer
Shared resource                     |       shared_counter

*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>

#define TIMER_INTERVAL_MS 500

static struct hrtimer my_hrtimer;
static struct tasklet_struct my_tasklet;
static spinlock_t my_spinlock;
static int shared_counter = 0;
static char *devname = "hi_tasklet_spin";

// Tasklet function (High-priority)
static void tasklet_fn(unsigned long data)
{
    unsigned long flags;

    // Lock to simulate shared resource protection
    spin_lock_irqsave(&my_spinlock, flags);

    shared_counter++;
    pr_info("%s: Tasklet executed, shared_counter = %d, data = %lu\n",
            devname, shared_counter, data);

    spin_unlock_irqrestore(&my_spinlock, flags);
}

// Simulated interrupt handler triggered by hrtimer
static enum hrtimer_restart timer_handler(struct hrtimer *timer)
{
    pr_info("%s: Simulated IRQ — scheduling HIGH priority tasklet\n", devname);
    tasklet_hi_schedule(&my_tasklet);  // High-priority tasklet scheduling

    hrtimer_forward_now(timer, ms_to_ktime(TIMER_INTERVAL_MS));
    return HRTIMER_RESTART;
}

static int __init hrtimer_tasklet_init(void)
{
    pr_info("%s: Init...\n", devname);

    // Init spinlock
    spin_lock_init(&my_spinlock);

    // Init tasklet
    tasklet_init(&my_tasklet, tasklet_fn, 999); // Pass data = 999

    // Init and start hrtimer
    hrtimer_init(&my_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    my_hrtimer.function = timer_handler;
    hrtimer_start(&my_hrtimer, ms_to_ktime(TIMER_INTERVAL_MS), HRTIMER_MODE_REL);

    return 0;
}

static void __exit hrtimer_tasklet_exit(void)
{
    pr_info("%s: Exit...\n", devname);
    hrtimer_cancel(&my_hrtimer);
    tasklet_kill(&my_tasklet);
}

module_init(hrtimer_tasklet_init);
module_exit(hrtimer_tasklet_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ChatGPT");
MODULE_DESCRIPTION("High-priority tasklet demo with spinlock protection and hrtimer IRQ simulation");