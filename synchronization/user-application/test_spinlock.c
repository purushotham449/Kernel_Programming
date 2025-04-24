#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE "/dev/spinlock_dev"
#define SPIN_LOCK         _IO(0xF0, 1)
#define SPIN_LOCK_IRQSAVE _IO(0xF0, 2)
#define SPIN_LOCK_IRQ     _IO(0xF0, 3)
#define SPIN_LOCK_BH      _IO(0xF0, 4)

int main() {
    int fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    printf("Triggering spin_lock...\n");
    ioctl(fd, SPIN_LOCK);

    printf("Triggering spin_lock_irqsave...\n");
    ioctl(fd, SPIN_LOCK_IRQSAVE);

    printf("Triggering spin_lock_irq...\n");
    ioctl(fd, SPIN_LOCK_IRQ);

    printf("Triggering spin_lock_bh...\n");
    ioctl(fd, SPIN_LOCK_BH);

    close(fd);
    return 0;
}