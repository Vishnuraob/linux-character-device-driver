#include "../include/prochardev.h"

// Called when the device is opened
int prochardev_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "prochardev: device opened\n");

    return 0;
}

// Called when the device is closed
int prochardev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "prochardev: device closed\n");

    return 0;
}

// Read data from the driver
ssize_t prochardev_read(struct file *file,
                        char __user *buffer,
                        size_t count,
                        loff_t *offset)
{
    char *temp_buffer;
    ssize_t bytes_read;

    if (count == 0)
        return 0;

    temp_buffer = kmalloc(count, GFP_KERNEL);

    if (!temp_buffer)
        return -ENOMEM;

    mutex_lock(&prochardev.lock);

    if (prochardev.data_count == 0)
    {
        mutex_unlock(&prochardev.lock);
        kfree(temp_buffer);
        return 0;
    }

    bytes_read = prochardev_buffer_read(temp_buffer, count);

    mutex_unlock(&prochardev.lock);

    if (copy_to_user(buffer, temp_buffer, bytes_read))
    {
        kfree(temp_buffer);
        return -EFAULT;
    }

    kfree(temp_buffer);

    printk(KERN_INFO
           "prochardev: read %zd bytes\n",
           bytes_read);

    return bytes_read;
}

// Write data to the driver
ssize_t prochardev_write(struct file *file,
                         const char __user *buffer,
                         size_t count,
                         loff_t *offset)
{
    char *temp_buffer;
    ssize_t bytes_written;

    if (count == 0)
        return 0;

    temp_buffer = kmalloc(count, GFP_KERNEL);

    if (!temp_buffer)
        return -ENOMEM;

    if (copy_from_user(temp_buffer,
                       buffer,
                       count))
    {
        kfree(temp_buffer);
        return -EFAULT;
    }

    mutex_lock(&prochardev.lock);

    bytes_written =
        prochardev_buffer_write(temp_buffer, count);

    mutex_unlock(&prochardev.lock);

    kfree(temp_buffer);

    if (bytes_written == 0)
    {
        printk(KERN_WARNING
               "prochardev: buffer is full\n");

        return -ENOSPC;
    }

    printk(KERN_INFO
           "prochardev: wrote %zd bytes\n",
           bytes_written);

    return bytes_written;
}

// Handle control commands
long prochardev_ioctl(struct file *file,
                      unsigned int cmd,
                      unsigned long arg)
{
    int value;

    switch (cmd)
    {
        case PROCHARDEV_CLEAR_BUFFER:

            mutex_lock(&prochardev.lock);

            prochardev_buffer_clear();

            mutex_unlock(&prochardev.lock);

            printk(KERN_INFO
                   "prochardev: buffer cleared\n");

            break;

        case PROCHARDEV_GET_BUFFER_SIZE:

            value = BUFFER_SIZE;

            if (copy_to_user((int __user *)arg,
                             &value,
                             sizeof(value)))
            {
                return -EFAULT;
            }

            break;

        case PROCHARDEV_GET_VERSION:

            value = 1;

            if (copy_to_user((int __user *)arg,
                             &value,
                             sizeof(value)))
            {
                return -EFAULT;
            }

            break;

        default:

            printk(KERN_WARNING
                   "prochardev: unknown ioctl command\n");

            return -EINVAL;
    }

    return 0;
}

// Connect system calls to driver functions
const struct file_operations prochardev_fops =
{
    .owner = THIS_MODULE,
    .open = prochardev_open,
    .read = prochardev_read,
    .write = prochardev_write,
    .release = prochardev_release,
    .unlocked_ioctl = prochardev_ioctl,
};
