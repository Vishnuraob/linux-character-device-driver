#include "../include/prochardev.h"

// Allocate the circular buffer
int prochardev_alloc_buffer(struct prochardev_device *dev)
{
    dev->buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);

    if (!dev->buffer)
    {
        printk(KERN_ERR "prochardev: buffer allocation failed\n");
        return -ENOMEM;
    }

    memset(dev->buffer, 0, BUFFER_SIZE);

    dev->read_pos = 0;
    dev->write_pos = 0;
    dev->data_count = 0;

    mutex_init(&dev->lock);
    init_waitqueue_head(&dev->read_queue);

    return 0;
}

// Free the circular buffer
void prochardev_free_buffer(struct prochardev_device *dev)
{
    kfree(dev->buffer);

    dev->buffer = NULL;
    dev->read_pos = 0;
    dev->write_pos = 0;
    dev->data_count = 0;
}

// Write data into the circular buffer
ssize_t prochardev_buffer_write(struct prochardev_device *dev,
                                const char *buffer,
                                size_t count)
{
    size_t i;
    size_t space;

    space = BUFFER_SIZE - dev->data_count;

    if (count > space)
        count = space;

    for (i = 0; i < count; i++)
    {
        dev->buffer[dev->write_pos] = buffer[i];

        dev->write_pos++;

        if (dev->write_pos == BUFFER_SIZE)
            dev->write_pos = 0;
    }

    dev->data_count += count;

    return count;
}

// Read data from the circular buffer
ssize_t prochardev_buffer_read(struct prochardev_device *dev,
                               char *buffer,
                               size_t count)
{
    size_t i;

    if (count > dev->data_count)
        count = dev->data_count;

    for (i = 0; i < count; i++)
    {
        buffer[i] = dev->buffer[dev->read_pos];

        dev->read_pos++;

        if (dev->read_pos == BUFFER_SIZE)
            dev->read_pos = 0;
    }

    dev->data_count -= count;

    return count;
}

// Clear all data from the buffer
void prochardev_buffer_clear(struct prochardev_device *dev)
{
    memset(dev->buffer, 0, BUFFER_SIZE);

    dev->read_pos = 0;
    dev->write_pos = 0;
    dev->data_count = 0;
}
