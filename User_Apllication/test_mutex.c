#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#define DEVICE_PATH "/dev/mutex_demo"

#define IOCTL_MUTEX_LOCK            _IO('M', 1)
#define IOCTL_MUTEX_LOCK_INTERRUPTIBLE _IO('M', 2)
#define IOCTL_MUTEX_LOCK_KILLABLE   _IO('M', 3)
#define IOCTL_MUTEX_TRYLOCK         _IO('M', 4)
#define IOCTL_MUTEX_IS_LOCKED       _IOR('M', 5, int)
#define IOCTL_MUTEX_UNLOCK          _IO('M', 6)

void perform_ioctl(int fd, unsigned int cmd, const char *cmd_name) {
    printf("\nInvoking IOCTL: %s\n", cmd_name);

    if (cmd == IOCTL_MUTEX_IS_LOCKED) {
        int is_locked = -1;
        if (ioctl(fd, cmd, &is_locked) < 0) {
            perror("ioctl - mutex_is_locked");
        } else {
            printf("Mutex is currently: %s\n", is_locked ? "LOCKED" : "UNLOCKED");
        }
    } else {
        if (ioctl(fd, cmd) < 0) {
            if (errno == EINTR)
                printf("IOCTL %s was interrupted by a signal.\n", cmd_name);
            else if (errno == EBUSY)
                printf("IOCTL %s failed: mutex is busy.\n", cmd_name);
            else
                perror("ioctl");
        } else {
            printf("IOCTL %s succeeded.\n", cmd_name);
        }
    }
}

int main() {
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    // Call each IOCTL one by one
    perform_ioctl(fd, IOCTL_MUTEX_LOCK, "mutex_lock");
    perform_ioctl(fd, IOCTL_MUTEX_IS_LOCKED, "mutex_is_locked");
    perform_ioctl(fd, IOCTL_MUTEX_UNLOCK, "mutex_unlock");

    printf("10 seconds delay\n");
    sleep(20);
    printf("IOCTL_MUTEX_LOCK_INTERRUPTIBLE started \n");
    perform_ioctl(fd, IOCTL_MUTEX_LOCK_INTERRUPTIBLE, "mutex_lock_interruptible");
    perform_ioctl(fd, IOCTL_MUTEX_UNLOCK, "mutex_unlock");

    printf("10 seconds delay\n");
    sleep(20);
    printf("IOCTL_MUTEX_LOCK_KILLABLE started \n");
    perform_ioctl(fd, IOCTL_MUTEX_LOCK_KILLABLE, "mutex_lock_killable");
    perform_ioctl(fd, IOCTL_MUTEX_UNLOCK, "mutex_unlock");

    perform_ioctl(fd, IOCTL_MUTEX_TRYLOCK, "mutex_trylock");
    perform_ioctl(fd, IOCTL_MUTEX_IS_LOCKED, "mutex_is_locked");
    perform_ioctl(fd, IOCTL_MUTEX_UNLOCK, "mutex_unlock");

    close(fd);
    return 0;
}