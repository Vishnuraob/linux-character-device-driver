#include "../include/prochardev.h"

// Allocate the circular buffer
int prochardev_alloc_buffer(void)
{
    prochardev.buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);

    if (!prochardev.buffer)
    {
        printk(KERN_ERR "prochardev: buffer allocation failed\n");
        return -ENOMEM;
    }

    memset(prochardev.buffer, 0, BUFFER_SIZE);

    prochardev.read_pos = 0;
    prochardev.write_pos = 0;
    prochardev.data_count = 0;

    mutex_init(&prochardev.lock);

    return 0;
}

// Free the circular buffer
void prochardev_free_buffer(void)
{
    kfree(prochardev.buffer);

    prochardev.buffer = NULL;
    prochardev.read_pos = 0;
    prochardev.write_pos = 0;
    prochardev.data_count = 0;
}

// Write data into the circular buffer
ssize_t prochardev_buffer_write(const char *buffer,
                                size_t count)
{
    size_t i;
    size_t space;

    space = BUFFER_SIZE - prochardev.data_count;

    if (count > space)
        count = space;

    for (i = 0; i < count; i++)
    {
        prochardev.buffer[prochardev.write_pos] = buffer[i];

        prochardev.write_pos++;

        if (prochardev.write_pos == BUFFER_SIZE)
            prochardev.write_pos = 0;
    }

    prochardev.data_count += count;

    return count;
}

// Read data from the circular buffer
ssize_t prochardev_buffer_read(char *buffer,
                               size_t count)
{
    size_t i;

    if (count > prochardev.data_count)
        count = prochardev.data_count;

    for (i = 0; i < count; i++)
    {
        buffer[i] = prochardev.buffer[prochardev.read_pos];

        prochardev.read_pos++;

        if (prochardev.read_pos == BUFFER_SIZE)
            prochardev.read_pos = 0;
    }

    prochardev.data_count -= count;

    return count;
}

// Clear all data from the buffer
void prochardev_buffer_clear(void)
{
    memset(prochardev.buffer, 0, BUFFER_SIZE);

    prochardev.read_pos = 0;
    prochardev.write_pos = 0;
    prochardev.data_count = 0;
}
