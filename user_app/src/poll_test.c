#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>

int main(void)
{
    int fd;
    int ret;
    struct pollfd pfd;

    fd = open("/dev/prochardev", O_RDONLY);

    if (fd < 0)
    {
        perror("open");
        return 1;
    }

    pfd.fd = fd;
    pfd.events = POLLIN;

    printf("Waiting for data...\n");

    ret = poll(&pfd, 1, -1);

    if (ret < 0)
    {
        perror("poll");
        close(fd);
        return 1;
    }

    if (pfd.revents & POLLIN)
        printf("Device has data\n");

    close(fd);

    return 0;
}
