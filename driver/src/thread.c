#include "../include/prochardev.h"

// Generate data periodically
static int prochardev_thread(void *data)
{
    char message[64];
    int counter = 0;
    ssize_t bytes_written;

    while (!kthread_should_stop())
    {
        snprintf(message,
                 sizeof(message),
                 "Kernel data %d\n",
                 counter++);

        mutex_lock(&prochardev.lock);

        bytes_written =
            prochardev_buffer_write(message,
                                    strlen(message));

        mutex_unlock(&prochardev.lock);

        if (bytes_written > 0)
        {
            wake_up_interruptible(&prochardev.read_queue);

            printk(KERN_INFO
                   "prochardev: kernel thread generated data\n");
        }

        msleep(5000);
    }

    printk(KERN_INFO
           "prochardev: kernel thread stopped\n");

    return 0;
}

// Start the kernel thread
int prochardev_thread_start(void)
{
    prochardev.thread =
        kthread_run(prochardev_thread,
                    NULL,
                    "prochardev_thread");

    if (IS_ERR(prochardev.thread))
    {
        printk(KERN_ERR
               "prochardev: failed to create kernel thread\n");

        prochardev.thread = NULL;

        return -ENOMEM;
    }

    printk(KERN_INFO
           "prochardev: kernel thread started\n");

    return 0;
}

// Stop the kernel thread
void prochardev_thread_stop(void)
{
    if (prochardev.thread)
    {
        kthread_stop(prochardev.thread);
        prochardev.thread = NULL;
    }
}
