// test_sem_variants.c
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main() {
    int fd = open("/dev/sem_variants", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    // Write something
    const char *msg = "Critical Code";
    if (write(fd, msg, strlen(msg)) < 0) {
        perror("write");
        close(fd);
        return 1;
    }

    printf("Written to device. Press ENTER to begin read...\n");
    getchar();  // Allow time to run signal test from another terminal

    char buf[100];
    ssize_t bytes = read(fd, buf, sizeof(buf));
    if (bytes < 0) {
        perror("read");
    } else {
        buf[bytes] = '\0';
        printf("Read from device: %s\n", buf);
    }

    close(fd);
    return 0;
}
