/* Here's a simple userspace test application in C that works with the dynamic character driver and interacts with it using ioctl().
This app:

    1. Fills the structure and sends it to the device.
    2. Retrieves the structure from the device.
    3. Sends to device (formats to buffer).
    4. Rereads from device (reads buffer back into structure).
*/
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#define DEVICE_PATH "/dev/cap_char_dev"

#define IOCTL_BASE 'W'
#define IOCTL_RETRIEVE      _IOR(IOCTL_BASE, 1, struct my_data *)
#define IOCTL_FILL_UP       _IOW(IOCTL_BASE, 2, struct my_data *)
#define IOCTL_SEND_TO_DEV   _IOW(IOCTL_BASE, 3, struct my_data *)
#define IOCTL_REREAD_DEV    _IOR(IOCTL_BASE, 4, struct my_data *)

struct my_data {
    int i;
    long x;
    char s[100];
};

void print_data(const char *label, struct my_data *data) {
    printf("%s:\n", label);
    printf("  i = %d\n", data->i);
    printf("  x = %ld\n", data->x);
    printf("  s = %s\n", data->s);
}

int main() {
    int fd;
    struct my_data data;

    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return EXIT_FAILURE;
    }

    // Fill data and write to kernel
    data.i = 100;
    data.x = 123456789;
    strcpy(data.s, "Hello from userspace");

    printf("Sending data to driver (IOCTL_FILL_UP)...\n");
    if (ioctl(fd, IOCTL_FILL_UP, &data) < 0) {
        perror("IOCTL_FILL_UP failed (need CAP_SYS_ADMIN)");
    }

    // Retrieve data back
    memset(&data, 0, sizeof(data));
    printf("\nRetrieving data from driver (IOCTL_RETRIEVE)...\n");
    if (ioctl(fd, IOCTL_RETRIEVE, &data) == 0) {
        print_data("Retrieved data", &data);
    } else {
        perror("IOCTL_RETRIEVE failed");
    }

    // Send to device (write to buffer)
    data.i = 555;
    data.x = 99999;
    strcpy(data.s, "Buffer payload test");

    printf("\nSending to device buffer (IOCTL_SEND_TO_DEV)...\n");
    if (ioctl(fd, IOCTL_SEND_TO_DEV, &data) < 0) {
        perror("IOCTL_SEND_TO_DEV failed (need CAP_SYS_ADMIN)");
    }

    // Reread device (parse buffer back to structure)
    memset(&data, 0, sizeof(data));
    printf("\nRereading from device buffer (IOCTL_REREAD_DEV)...\n");
    if (ioctl(fd, IOCTL_REREAD_DEV, &data) == 0) {
        print_data("Reread data", &data);
    } else {
        perror("IOCTL_REREAD_DEV failed");
    }

    close(fd);
    return 0;
}