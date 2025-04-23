/*  a simple Linux kernel module that registers an interrupt handler on IRQ 18, 
demonstrates how to use IRQ flags to set priority or behavior (such as shared or edge-triggered), 
and logs basic messages when the interrupt is triggered.

Flag                            |                       Description
---------------------------------------------------------------------------------------------------------------------
IRQF_SHARED	                    |                       Allow sharing the IRQ line among devices.
IRQF_TIMER	                    |                       Marks a timer interrupt (higher priority in some cases).
IRQF_NOBALANCING	            |                       Prevent CPU IRQ balancing for this IRQ.
IRQF_PERCPU	IRQ                 |                       is per-CPU (useful for high-performance drivers).
IRQF_NO_THREAD	                |                       Force hard IRQ handling (no threaded IRQ).
IRQF_EARLY_RESUME	            |                       Early resume from suspend (used by critical drivers).
IRQF_TRIGGER_RISING             |                       Trigger on rising edge
IRQF_TRIGGER_FALLING            |                       Trigger on falling edge
IRQF_TRIGGER_HIGH               |                       Trigger on high level
IRQF_TRIGGER_LOW                |                       Trigger on low level

*/

#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/init.h>

#define MY_IRQ 183      // Change this according to your available IRQ
#define IRQ_NAME "my_irq_handler"

static int irq_count = 0;

// IRQ handler function
static irqreturn_t my_irq_handler(int irq, void *dev_id)
{
    irq_count++;
    pr_info("Interrupt Received on IRQ %d, count = %d\n", irq, irq_count);
    return IRQ_HANDLED;
}

static int __init irq_module_init(void)
{
    int ret;

    pr_info("Registering interrupt on IRQ %d\n", MY_IRQ);

    // Request IRQ with priority flags
    ret = request_irq(MY_IRQ,                       // IRQ number
                      my_irq_handler,               // Handler
                      IRQF_SHARED |                 // IRQ can be shared
                      IRQF_TRIGGER_RISING,          // Triggered on rising edge
                      IRQ_NAME,                     // Device name
                      (void *)(my_irq_handler));    // Shared ID

    if (ret) {
        pr_err("Failed to request IRQ %d\n", MY_IRQ);
        return ret;
    }

    pr_info("IRQ module loaded successfully on IRQ %d\n", MY_IRQ);
    return 0;
}

static void __exit irq_module_exit(void)
{
    free_irq(MY_IRQ, (void *)(my_irq_handler));
    pr_info("IRQ module unloaded, total interrupts = %d\n", irq_count);
}

module_init(irq_module_init);
module_exit(irq_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ChatGPT");
MODULE_DESCRIPTION("Simple IRQ handler on IRQ 18 using priority flags");