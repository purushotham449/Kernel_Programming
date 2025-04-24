#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>

#define DEV_PATH "/dev/asyncpoll"

static void sigio_handler(int signo) {
    printf("Received SIGIO signal (asynchronous notification).\n");
}

int main() {
    int fd = open(DEV_PATH, O_RDWR | O_NONBLOCK);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    // Setup signal handler
    signal(SIGIO, sigio_handler);

    // Set owner and enable async mode
    fcntl(fd, F_SETOWN, getpid());
    fcntl(fd, F_SETFL, O_ASYNC | O_NONBLOCK);

    printf("Monitoring device using poll and async signal...\n");

    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;

    while (1) {
        int ret = poll(&pfd, 1, 5000); // 5 second timeout
        if (ret == -1) {
            perror("poll");
            break;
        } else if (ret == 0) {
            printf("Poll timeout, still waiting for data...\n");
        } else if (pfd.revents & POLLIN) {
            char buf[1024] = {0};
            read(fd, buf, sizeof(buf));
            printf("Poll read data: %s\n", buf);
        }
    }

    close(fd);
    return 0;
}