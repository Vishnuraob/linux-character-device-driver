#include "../include/prochardev.h"

// Open the selected device
int prochardev_open(struct inode *inode, struct file *file)
{
    struct prochardev_device *dev;

    dev = container_of(inode->i_cdev,
                       struct prochardev_device,
                       cdev);

    file->private_data = dev;

    printk(KERN_INFO
           "prochardev%d: device opened\n",
           MINOR(dev->dev_num));

    return 0;
}

// Close the device
int prochardev_release(struct inode *inode, struct file *file)
{
    struct prochardev_device *dev = file->private_data;

    printk(KERN_INFO
           "prochardev%d: device closed\n",
           MINOR(dev->dev_num));

    return 0;
}

// Read data from the device
ssize_t prochardev_read(struct file *file,
                        char __user *buffer,
                        size_t count,
                        loff_t *offset)
{
    struct prochardev_device *dev = file->private_data;
    char *temp_buffer;
    ssize_t bytes_read;
    int ret;

    if (count == 0)
        return 0;

    temp_buffer = kmalloc(count, GFP_KERNEL);

    if (!temp_buffer)
        return -ENOMEM;

    if (dev->data_count == 0 &&
        (file->f_flags & O_NONBLOCK))
    {
        kfree(temp_buffer);
        return -EAGAIN;
    }

    ret = wait_event_interruptible(
        dev->read_queue,
        dev->data_count > 0
    );

    if (ret)
    {
        kfree(temp_buffer);
        return ret;
    }

    mutex_lock(&dev->lock);

    bytes_read =
        prochardev_buffer_read(dev,
                               temp_buffer,
                               count);

    mutex_unlock(&dev->lock);

    if (copy_to_user(buffer,
                     temp_buffer,
                     bytes_read))
    {
        kfree(temp_buffer);
        return -EFAULT;
    }

    kfree(temp_buffer);

    printk(KERN_INFO
           "prochardev%d: read %zd bytes\n",
           MINOR(dev->dev_num),
           bytes_read);

    return bytes_read;
}

// Write data to the device
ssize_t prochardev_write(struct file *file,
                         const char __user *buffer,
                         size_t count,
                         loff_t *offset)
{
    struct prochardev_device *dev = file->private_data;
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

    mutex_lock(&dev->lock);

    bytes_written =
        prochardev_buffer_write(dev,
                                temp_buffer,
                                count);

    mutex_unlock(&dev->lock);

    kfree(temp_buffer);

    if (bytes_written == 0)
        return -ENOSPC;

    wake_up_interruptible(&dev->read_queue);

    printk(KERN_INFO
           "prochardev%d: wrote %zd bytes\n",
           MINOR(dev->dev_num),
           bytes_written);

    return bytes_written;
}

// Handle control commands
long prochardev_ioctl(struct file *file,
                      unsigned int cmd,
                      unsigned long arg)
{
    struct prochardev_device *dev = file->private_data;
    int value;

    switch (cmd)
    {
        case PROCHARDEV_CLEAR_BUFFER:

            mutex_lock(&dev->lock);

            prochardev_buffer_clear(dev);

            mutex_unlock(&dev->lock);

            printk(KERN_INFO
                   "prochardev%d: buffer cleared\n",
                   MINOR(dev->dev_num));

            break;

        case PROCHARDEV_GET_BUFFER_SIZE:

            value = BUFFER_SIZE;

            if (copy_to_user((int __user *)arg,
                             &value,
                             sizeof(value)))
                return -EFAULT;

            break;

        case PROCHARDEV_GET_VERSION:

            value = 1;

            if (copy_to_user((int __user *)arg,
                             &value,
                             sizeof(value)))
                return -EFAULT;

            break;

        default:

            return -EINVAL;
    }

    return 0;
}

// Check whether the device has data
static __poll_t prochardev_poll(struct file *file,
                                struct poll_table_struct *wait)
{
    struct prochardev_device *dev = file->private_data;
    __poll_t mask = 0;

    poll_wait(file, &dev->read_queue, wait);

    mutex_lock(&dev->lock);

    if (dev->data_count > 0)
        mask |= POLLIN | POLLRDNORM;

    mutex_unlock(&dev->lock);

    return mask;
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
    .poll = prochardev_poll,
};
