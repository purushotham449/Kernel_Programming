#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main()
{
    int fd = open("/dev/blockio", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    char buf[128];

    printf("Writing to device...\n");
    write(fd, "Hello from user", 16);

    // printf("Reading from device (will block if no write)...\n");
    // int r = read(fd, buf, sizeof(buf));
    // buf[r] = '\0';

    // printf("Received: %s\n", buf);
    close(fd);
    return 0;
}
