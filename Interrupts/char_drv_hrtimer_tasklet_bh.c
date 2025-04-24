/* simulate periodic interrupt-like behavior using hrtimer, 
we can mimic hardware interrupts by triggering a software handler on a high-resolution timer expiry. Then, 
from this handler, we can schedule a tasklet — exactly like you would from a real hardware IRQ.

1. Sets up an hrtimer to expire periodically (e.g., every 500ms)
2. On each expiry, it mimics an interrupt by calling a software handler
3. That handler then schedules a tasklet (just like in a real interrupt context)

*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/interrupt.h>

#define TIMER_INTERVAL_MS 500

static struct hrtimer my_hrtimer;
static struct tasklet_struct my_tasklet;
static char *devname = "hrtimer_tasklet";

// Tasklet function
static void tasklet_fn(unsigned long data)
{
    pr_info("%s: Tasklet executed — Bottom Half, data = %lu\n", devname, data);
}

// Simulated interrupt handler triggered by hrtimer
static enum hrtimer_restart timer_handler(struct hrtimer *timer)
{
    pr_info("%s: Simulated IRQ — scheduling tasklet\n", devname);
    tasklet_schedule(&my_tasklet);

    // Re-arm the timer for next expiry
    hrtimer_forward_now(timer, ms_to_ktime(TIMER_INTERVAL_MS));
    return HRTIMER_RESTART;
}

static int __init hrtimer_tasklet_init(void)
{
    pr_info("%s: Initializing...\n", devname);

    // Initialize tasklet (callback + data)
    tasklet_init(&my_tasklet, tasklet_fn, 999);

    // Setup the hrtimer
    hrtimer_init(&my_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    my_hrtimer.function = timer_handler;
    hrtimer_start(&my_hrtimer, ms_to_ktime(TIMER_INTERVAL_MS), HRTIMER_MODE_REL);

    return 0;
}

static void __exit hrtimer_tasklet_exit(void)
{
    pr_info("%s: Exiting...\n", devname);
    hrtimer_cancel(&my_hrtimer);   // Cancel timer
    tasklet_kill(&my_tasklet);     // Kill tasklet
}

module_init(hrtimer_tasklet_init);
module_exit(hrtimer_tasklet_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ChatGPT");
MODULE_DESCRIPTION("Simulated IRQ via hrtimer with tasklet");