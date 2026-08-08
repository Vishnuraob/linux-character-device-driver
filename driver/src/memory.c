#include "../include/prochardev.h"

// Allocate and initialize the driver buffer
int prochardev_alloc_buffer(void)
{
    prochardev.buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);

    if (!prochardev.buffer)
    {
        printk(KERN_ERR "prochardev: buffer allocation failed\n");
        return -ENOMEM;
    }

    memset(prochardev.buffer, 0, BUFFER_SIZE);

    strcpy(prochardev.buffer, "Hello from prochardev!\n");

    mutex_init(&prochardev.lock);

    return 0;
}

// Free the driver buffer
void prochardev_free_buffer(void)
{
    kfree(prochardev.buffer);
    prochardev.buffer = NULL;
}
