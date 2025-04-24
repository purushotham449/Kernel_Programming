/* Advanced control of memory and data alignment in the Linux kernel 

__attribute__((packed)) – removes padding (may affect performance)
__attribute__((aligned(n))) – forces a specific alignment

What Happens Here

Structure                   | Description                               | Likely Size
default_struct              | Compiler adds padding for alignment       | ~12 bytes
packed_struct               | No padding, tightly packed                | ~6 bytes
aligned_struct              | Structure aligned to 8-byte boundary      | 16 bytes

Notes:

    __packed may cause slower access due to unaligned memory reads (especially on ARM).
    __aligned(n) is useful for DMA buffers, cache optimization, or hardware-mapped structures.
    Kernel prefers natural alignment unless performance or hardware requires otherwise.
*/

// File: alignment_advanced.c

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Purushotham");
MODULE_DESCRIPTION("Advanced Alignment Demo in Linux Kernel");
MODULE_VERSION("1.1");

// Default aligned struct
struct default_struct {
    char a;       // 1 byte
    int b;        // 4 bytes
    char c;       // 1 byte
};

// Packed struct (no padding)
struct packed_struct {
    char a;
    int b;
    char c;
} __attribute__((packed));

// Aligned struct (force 8-byte alignment)
struct aligned_struct {
    char a;
    int b;
    char c;
} __attribute__((aligned(8)));

static int __init alignment_advanced_init(void)
{
    pr_info("Advanced Alignment Demo Module Loaded\n");

    pr_info("Size of default_struct: %zu bytes\n", sizeof(struct default_struct));
    pr_info("Size of packed_struct : %zu bytes\n", sizeof(struct packed_struct));
    pr_info("Size of aligned_struct: %zu bytes\n", sizeof(struct aligned_struct));

    return 0;
}

static void __exit alignment_advanced_exit(void)
{
    pr_info("Advanced Alignment Demo Module Unloaded\n");
}

module_init(alignment_advanced_init);
module_exit(alignment_advanced_exit);