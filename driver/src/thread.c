#include "../include/prochardev.h"

// Generate data periodically
static int prochardev_thread(void *data)
{
    struct prochardev_device *dev = data;
    char message[64];
    int counter = 0;
    ssize_t bytes_written;

    while (!kthread_should_stop())
    {
        snprintf(message,
                 sizeof(message),
                 "Device %d data %d\n",
                 MINOR(dev->dev_num),
                 counter++);

        mutex_lock(&dev->lock);

        bytes_written =
            prochardev_buffer_write(dev,
                                    message,
                                    strlen(message));

        mutex_unlock(&dev->lock);

        if (bytes_written > 0)
            wake_up_interruptible(&dev->read_queue);

        msleep(5000);
    }

    printk(KERN_INFO
           "prochardev%d: kernel thread stopped\n",
           MINOR(dev->dev_num));

    return 0;
}

// Start the kernel thread
int prochardev_thread_start(struct prochardev_device *dev)
{
    dev->thread =
        kthread_run(prochardev_thread,
                    dev,
                    "prochardev%d",
                    MINOR(dev->dev_num));

    if (IS_ERR(dev->thread))
    {
        printk(KERN_ERR
               "prochardev%d: thread creation failed\n",
               MINOR(dev->dev_num));

        dev->thread = NULL;

        return -ENOMEM;
    }

    printk(KERN_INFO
           "prochardev%d: kernel thread started\n",
           MINOR(dev->dev_num));

    return 0;
}

// Stop the kernel thread
void prochardev_thread_stop(struct prochardev_device *dev)
{
    if (dev->thread)
    {
        kthread_stop(dev->thread);
        dev->thread = NULL;
    }
}

