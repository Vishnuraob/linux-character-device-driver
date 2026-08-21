#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include "../../driver/include/prochardev_ioctl.h"

#define BUFFER_SIZE 256

static void write_data(int fd)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_written;

    printf("Enter data: ");

    if (fgets(buffer, sizeof(buffer), stdin) == NULL)
        return;

    buffer[strcspn(buffer, "\n")] = '\0';

    bytes_written = write(fd, buffer, strlen(buffer));

    if (bytes_written < 0)
    {
        perror("write");
        return;
    }

    printf("Written %zd bytes\n", bytes_written);
}

static void read_data(int fd)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    bytes_read = read(fd, buffer, sizeof(buffer));

    if (bytes_read < 0)
    {
        perror("read");
        return;
    }

    printf("Driver data: ");

    fwrite(buffer, 1, bytes_read, stdout);

    printf("\n");
}

static void clear_buffer(int fd)
{
    if (ioctl(fd, PROCHARDEV_CLEAR_BUFFER) < 0)
    {
        perror("ioctl");
        return;
    }

    printf("Buffer cleared\n");
}

static void get_buffer_size(int fd)
{
    int size;

    if (ioctl(fd, PROCHARDEV_GET_BUFFER_SIZE, &size) < 0)
    {
        perror("ioctl");
        return;
    }

    printf("Buffer size: %d bytes\n", size);
}

static void get_version(int fd)
{
    int version;

    if (ioctl(fd, PROCHARDEV_GET_VERSION, &version) < 0)
    {
        perror("ioctl");
        return;
    }

    printf("Driver version: %d\n", version);
}

static void show_menu(void)
{
    printf("\n");
    printf("===== ProCharDev =====\n");
    printf("1. Write data\n");
    printf("2. Read data\n");
    printf("3. Clear buffer\n");
    printf("4. Get buffer size\n");
    printf("5. Get driver version\n");
    printf("6. Exit\n");
    printf("======================\n");
    printf("Enter choice: ");
}

int main(int argc, char *argv[])
{
    int fd;
    int choice;
    char device_path[64];

    if (argc != 2)
    {
        printf("Usage: %s <device_number>\n", argv[0]);
        printf("Example: %s 0\n", argv[0]);
        return 1;
    }

    snprintf(device_path,
             sizeof(device_path),
             "/dev/prochardev%s",
             argv[1]);

    fd = open(device_path, O_RDWR);

    if (fd < 0)
    {
        perror("open");
        return 1;
    }

    printf("Opened %s\n", device_path);

    while (1)
    {
        show_menu();

        if (scanf("%d", &choice) != 1)
        {
            printf("Invalid input\n");

            while (getchar() != '\n')
                ;

            continue;
        }

        while (getchar() != '\n')
            ;

        switch (choice)
        {
            case 1:
                write_data(fd);
                break;

            case 2:
                read_data(fd);
                break;

            case 3:
                clear_buffer(fd);
                break;

            case 4:
                get_buffer_size(fd);
                break;

            case 5:
                get_version(fd);
                break;

            case 6:
                close(fd);
                printf("Exiting...\n");
                return 0;

            default:
                printf("Invalid choice\n");
        }
    }
}
