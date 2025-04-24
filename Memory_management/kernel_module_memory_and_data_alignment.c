/* Memory Alignment alignment in Linux Kernel Internals

struct aligned_example {
    char a;     // 1 byte
    int b;      // 4 bytes, alignment padding needed after 'a'
};

Offset:  0    1  2  3  4  5  6  7
         +----+--------+
Field:   | a  | padding | b (4 bytes)

Total size: 8 bytes (due to 3 bytes of padding for alignment)

Why It Matters:

    Performance: Some architectures (like ARM) perform faster or only support aligned accesses.
    Hardware Constraints: Misaligned access may cause faults or require additional instructions.

Data Alignment in Linux Kernel Internals

struct optimized_example {
    int b;     // 4 bytes
    char a;    // 1 byte
    // 3 bytes padding at the end, instead of in-between
};

Offset:  0  1  2  3  4
         +----+--+
Field:   | b (4) | a |

Total size: 8 bytes (still padded, but more efficient use if multiple such structs exist in an array)
*/

// File: alignment_demo.c

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Purushotham");
MODULE_DESCRIPTION("Alignment Demo in Linux Kernel");
MODULE_VERSION("1.0");

struct unaligned_struct {
    char a;      // 1 byte
    int b;       // 4 bytes (padding added after 'a')
    char c;      // 1 byte (padding added after 'c' too)
};

struct aligned_struct {
    int b;       // 4 bytes
    char a;      // 1 byte
    char c;      // 1 byte
    // minimal padding at the end
};

static int __init alignment_demo_init(void)
{
    pr_info("Alignment Demo Module Loaded\n");

    pr_info("Size of unaligned_struct: %zu bytes\n", sizeof(struct unaligned_struct));
    pr_info("Size of aligned_struct  : %zu bytes\n", sizeof(struct aligned_struct));

    return 0;
}

static void __exit alignment_demo_exit(void)
{
    pr_info("Alignment Demo Module Unloaded\n");
}

module_init(alignment_demo_init);
module_exit(alignment_demo_exit);