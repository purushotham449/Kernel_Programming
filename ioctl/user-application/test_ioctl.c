#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>

#define DEVICE "/dev/dynamic_ioctl_dev"

// IOCTL command definitions
#define MAGIC_NUM      0xF0
#define IOCTL_FILL_ZERO    _IO(MAGIC_NUM, 1)
#define IOCTL_FILL_CHAR    _IOW(MAGIC_NUM, 2, char)
#define IOCTL_SET_SIZE     _IOW(MAGIC_NUM, 3, int)
#define IOCTL_GET_SIZE     _IOR(MAGIC_NUM, 4, int)
#define IOCTL_MAX_CMD      _IO(MAGIC_NUM, 5)

int main() {
    int fd;
    char write_buf[] = "Hello Kernel!";
    char read_buf[50];
    int new_size = 1000, current_size;
    char ch = '#';

    fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return errno;
    }

    // Write data
    write(fd, write_buf, strlen(write_buf));
    printf("Wrote: %s\n", write_buf);

    // Seek to beginning
    lseek(fd, 0, SEEK_SET);

    // Read data back
    read(fd, read_buf, sizeof(read_buf));
    printf("Read: %s\n", read_buf);

    // Fill with zeros
    ioctl(fd, IOCTL_FILL_ZERO);
    printf("IOCTL: Filled with zeroes\n");

    // Fill with char '#'
    ioctl(fd, IOCTL_FILL_CHAR, &ch);
    printf("IOCTL: Filled with char '%c'\n", ch);

    // Set new size
    ioctl(fd, IOCTL_SET_SIZE, &new_size);
    printf("IOCTL: Buffer size set to %d\n", new_size);

    // Get current size
    ioctl(fd, IOCTL_GET_SIZE, &current_size);
    printf("IOCTL: Buffer size is %d\n", current_size);

    // Max command test
    ioctl(fd, IOCTL_MAX_CMD);
    printf("IOCTL: Max command test done\n");

    close(fd);
    return 0;
}