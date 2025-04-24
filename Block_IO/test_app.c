#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

#define DEVICE "/dev/wait_demo"

#define MAGIC 'x'
#define IOCTL_DOWN_INTERRUPTIBLE _IO(MAGIC, 1)
#define IOCTL_WAIT_COMPLETION    _IO(MAGIC, 2)
#define IOCTL_COMPLETE           _IO(MAGIC, 3)
#define IOCTL_WAIT_EVENT         _IO(MAGIC, 4)
#define IOCTL_WAKE_EVENT         _IO(MAGIC, 5)
#define IOCTL_WAIT_EVENT_EXCL    _IO(MAGIC, 6)

int main(int argc, char *argv[]) {
    int fd;

    fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    if (argc < 2) {
        printf("Usage: %s <option>\n", argv[0]);
        printf("Options: down, comp, wait_comp, wait_event, wake_event, wait_excl\n");
        return 1;
    }

    if (strcmp(argv[1], "down") == 0)
        ioctl(fd, IOCTL_DOWN_INTERRUPTIBLE);
    else if (strcmp(argv[1], "comp") == 0)
        ioctl(fd, IOCTL_COMPLETE);
    else if (strcmp(argv[1], "wait_comp") == 0)
        ioctl(fd, IOCTL_WAIT_COMPLETION);
    else if (strcmp(argv[1], "wait_event") == 0)
        ioctl(fd, IOCTL_WAIT_EVENT);
    else if (strcmp(argv[1], "wake_event") == 0)
        ioctl(fd, IOCTL_WAKE_EVENT);
    else if (strcmp(argv[1], "wait_excl") == 0)
        ioctl(fd, IOCTL_WAIT_EVENT_EXCL);
    else
        printf("Unknown option.\n");

    close(fd);
    return 0;
}