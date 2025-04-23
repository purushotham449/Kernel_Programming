/*  Plan: Simulated Interrupt using hrtimer
We’ll:

    Use hrtimer to periodically simulate an IRQ-like behavior.
    Each "tick" will call a handler similar to an ISR.
    Show counts in /proc/interrupts or via dmesg
*/

#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>

#define SIMULATED_IRQ 18
static unsigned long irq_count = 0;
static struct hrtimer sim_timer;
static ktime_t interval;

enum hrtimer_restart timer_callback(struct hrtimer *timer);

enum hrtimer_restart timer_callback(struct hrtimer *timer)
{
    irq_count++;
    pr_info("Simulated IRQ %d: interrupt count = %lu\n", SIMULATED_IRQ, irq_count);

    // Re-trigger the timer
    hrtimer_forward_now(timer, interval);
    return HRTIMER_RESTART;
}

static int __init irq_sim_init(void)
{
    interval = ktime_set(0, 5000000); // 5 ms

    hrtimer_init(&sim_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    sim_timer.function = &timer_callback;
    hrtimer_start(&sim_timer, interval, HRTIMER_MODE_REL);

    pr_info("Simulated IRQ module loaded\n");
    return 0;
}

static void __exit irq_sim_exit(void)
{
    hrtimer_cancel(&sim_timer);
    pr_info("Simulated IRQ module unloaded\n");
}

module_init(irq_sim_init);
module_exit(irq_sim_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ChatGPT");
MODULE_DESCRIPTION("Simulated periodic IRQ using hrtimer");