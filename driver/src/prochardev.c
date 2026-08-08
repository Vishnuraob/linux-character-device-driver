#include "../include/prochardev.h"

struct prochardev_device prochardev;

// Called when the device is opened
static int prochardev_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "prochardev: device opened\n");

    return 0;
}

// Called when the device is closed
static int prochardev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "prochardev: device closed\n");

    return 0;
}

// Read data from the driver
static ssize_t prochardev_read(struct file *file,
                               char __user *buffer,
                               size_t count,
                               loff_t *offset)
{
    size_t data_length;

    // Lock the buffer before accessing it
    mutex_lock(&prochardev.lock);

    data_length = strlen(prochardev.buffer);

    // Nothing left to read
    if (*offset >= data_length)
    {
        mutex_unlock(&prochardev.lock);
        return 0;
    }

    // Don't read past the available data
    if (count > data_length - *offset)
        count = data_length - *offset;

    // Copy data to user space
    if (copy_to_user(buffer,
                     prochardev.buffer + *offset,
                     count))
    {
        mutex_unlock(&prochardev.lock);
        return -EFAULT;
    }

    *offset += count;

    // Unlock after accessing the buffer
    mutex_unlock(&prochardev.lock);

    printk(KERN_INFO "prochardev: read %zu bytes\n", count);

    return count;
}

// Write data to the driver
static ssize_t prochardev_write(struct file *file,
                                const char __user *buffer,
                                size_t count,
                                loff_t *offset)
{
    // Lock the buffer before modifying it
    mutex_lock(&prochardev.lock);

    // Keep one byte for the null terminator
    if (count >= BUFFER_SIZE)
        count = BUFFER_SIZE - 1;

    // Copy data from user space
    if (copy_from_user(prochardev.buffer,
                       buffer,
                       count))
    {
        mutex_unlock(&prochardev.lock);
        return -EFAULT;
    }

    prochardev.buffer[count] = '\0';

    // Unlock after modifying the buffer
    mutex_unlock(&prochardev.lock);

    printk(KERN_INFO "prochardev: received data\n");

    return count;
}

// Connect system calls to driver functions
static const struct file_operations prochardev_fops =
{
    .owner = THIS_MODULE,
    .open = prochardev_open,
    .read = prochardev_read,
    .write = prochardev_write,
    .release = prochardev_release,
};

// Driver initialization
static int __init prochardev_init(void)
{
    int ret;

    // Allocate the driver buffer
    prochardev.buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);

    if (!prochardev.buffer)
    {
        printk(KERN_ERR "prochardev: buffer allocation failed\n");
        return -ENOMEM;
    }

    memset(prochardev.buffer, 0, BUFFER_SIZE);

    strcpy(prochardev.buffer, "Hello from prochardev!\n");

    // Initialize the mutex
    mutex_init(&prochardev.lock);

    // Allocate a major and minor number
    ret = alloc_chrdev_region(&prochardev.dev_num,
                              0,
                              1,
                              DRIVER_NAME);

    if (ret < 0)
    {
        printk(KERN_ERR
               "prochardev: failed to allocate device number\n");

        goto error_buffer;
    }

    // Initialize the character device
    cdev_init(&prochardev.cdev, &prochardev_fops);

    prochardev.cdev.owner = THIS_MODULE;

    // Add the character device to the kernel
    ret = cdev_add(&prochardev.cdev,
                   prochardev.dev_num,
                   1);

    if (ret < 0)
    {
        printk(KERN_ERR "prochardev: cdev_add failed\n");

        goto error_device_number;
    }

    // Create the device class
    prochardev.class = class_create(DRIVER_CLASS);

    if (IS_ERR(prochardev.class))
    {
        ret = PTR_ERR(prochardev.class);

        goto error_cdev;
    }

    // Create /dev/prochardev
    prochardev.device = device_create(prochardev.class,
                                      NULL,
                                      prochardev.dev_num,
                                      NULL,
                                      DRIVER_NAME);

    if (IS_ERR(prochardev.device))
    {
        ret = PTR_ERR(prochardev.device);

        goto error_class;
    }

    printk(KERN_INFO "prochardev: driver loaded\n");

    return 0;

error_class:
    class_destroy(prochardev.class);

error_cdev:
    cdev_del(&prochardev.cdev);

error_device_number:
    unregister_chrdev_region(prochardev.dev_num, 1);

error_buffer:
    kfree(prochardev.buffer);
    prochardev.buffer = NULL;

    return ret;
}

// Driver cleanup
static void __exit prochardev_exit(void)
{
    // Remove the device node
    device_destroy(prochardev.class, prochardev.dev_num);

    // Remove the device class
    class_destroy(prochardev.class);

    // Remove the character device
    cdev_del(&prochardev.cdev);

    // Release the major and minor numbers
    unregister_chrdev_region(prochardev.dev_num, 1);

    // Free the driver buffer
    kfree(prochardev.buffer);
    prochardev.buffer = NULL;

    printk(KERN_INFO "prochardev: driver unloaded\n");
}

module_init(prochardev_init);
module_exit(prochardev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vishnu");
MODULE_DESCRIPTION("Professional Linux Character Device Driver");
MODULE_VERSION("1.0");
