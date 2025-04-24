
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

void sigio_handler(int signo) {
    printf("Received SIGIO: async data is ready!\n");
}

int main() {
    int fd = open("/dev/async_dev", O_WRONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    signal(SIGIO, sigio_handler);

    // User-space registers for asynchronous notification using below
    fcntl(fd, F_SETOWN, getpid());              // Set owner
    fcntl(fd, F_SETFL, O_ASYNC);                // Enable async notification

    printf("Waiting for async signal (SIGIO)...\n");

    while (1) {
        pause(); // Sleep until signal received
    }

    close(fd);
    return 0;
}