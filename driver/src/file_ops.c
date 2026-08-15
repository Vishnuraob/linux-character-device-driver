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
    mutex_lock(&prochardev.lock);

    if (*offset >= prochardev.data_length)
    {
        mutex_unlock(&prochardev.lock);
        return 0;
    }

    if (count > prochardev.data_length - *offset)
        count = prochardev.data_length - *offset;

    if (copy_to_user(buffer,
                     prochardev.buffer + *offset,
                     count))
    {
        mutex_unlock(&prochardev.lock);
        return -EFAULT;
    }

    *offset += count;

    mutex_unlock(&prochardev.lock);

    printk(KERN_INFO "prochardev: read %zu bytes\n", count);

    return count;
}

// Write data to the driver
ssize_t prochardev_write(struct file *file,
                         const char __user *buffer,
                         size_t count,
                         loff_t *offset)
{
    mutex_lock(&prochardev.lock);

    if (count >= BUFFER_SIZE)
        count = BUFFER_SIZE - 1;

    if (copy_from_user(prochardev.buffer,
                       buffer,
                       count))
    {
        mutex_unlock(&prochardev.lock);
        return -EFAULT;
    }

    prochardev.buffer[count] = '\0';

    prochardev.data_length = count;

    mutex_unlock(&prochardev.lock);

    printk(KERN_INFO "prochardev: data written\n");

    return count;
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

            memset(prochardev.buffer,
                   0,
                   BUFFER_SIZE);

            prochardev.data_length = 0;

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
