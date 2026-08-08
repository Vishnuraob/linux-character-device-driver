#include "../include/prochardev.h"

/*
 * Driver context.
 */
struct prochardev_device prochardev;


/*
 * Open
 */
static int prochardev_open(struct inode *inode,
                           struct file *file)
{
    printk(KERN_INFO
           "prochardev: device opened\n");

    return 0;
}


/*
 * Release
 */
static int prochardev_release(struct inode *inode,
                              struct file *file)
{
    printk(KERN_INFO
           "prochardev: device closed\n");

    return 0;
}


/*
 * Read
 */
static ssize_t prochardev_read(struct file *file,
                               char __user *buffer,
                               size_t count,
                               loff_t *offset)
{
    size_t data_length;

    data_length = strlen(prochardev.buffer);

    if (*offset >= data_length)
        return 0;

    if (count > data_length - *offset)
        count = data_length - *offset;

    if (copy_to_user(buffer,
                     prochardev.buffer + *offset,
                     count))
    {
        return -EFAULT;
    }

    *offset += count;

    printk(KERN_INFO
           "prochardev: read %zu bytes\n",
           count);

    return count;
}


/*
 * Write
 */
static ssize_t prochardev_write(struct file *file,
                                const char __user *buffer,
                                size_t count,
                                loff_t *offset)
{
    if (count >= BUFFER_SIZE)
        count = BUFFER_SIZE - 1;

    if (copy_from_user(prochardev.buffer,
                       buffer,
                       count))
    {
        return -EFAULT;
    }

    prochardev.buffer[count] = '\0';

    printk(KERN_INFO
           "prochardev: received \"%s\"\n",
           prochardev.buffer);

    return count;
}


/*
 * File operations.
 */
static const struct file_operations prochardev_fops =
{
    .owner   = THIS_MODULE,
    .open    = prochardev_open,
    .read    = prochardev_read,
    .write   = prochardev_write,
    .release = prochardev_release,
};


/*
 * Driver initialization.
 */
static int __init prochardev_init(void)
{
    int ret;

    /*
     * Allocate kernel buffer dynamically.
     */
    prochardev.buffer =
        kmalloc(BUFFER_SIZE, GFP_KERNEL);

    if (!prochardev.buffer)
    {
        printk(KERN_ERR
               "prochardev: buffer allocation failed\n");

        return -ENOMEM;
    }

    memset(prochardev.buffer,
           0,
           BUFFER_SIZE);

    strcpy(prochardev.buffer,
           "Hello from prochardev!\n");


    /*
     * Allocate major/minor numbers.
     */
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


    /*
     * Initialize cdev.
     */
    cdev_init(&prochardev.cdev,
              &prochardev_fops);

    prochardev.cdev.owner = THIS_MODULE;


    /*
     * Add cdev.
     */
    ret = cdev_add(&prochardev.cdev,
                   prochardev.dev_num,
                   1);

    if (ret < 0)
    {
        printk(KERN_ERR
               "prochardev: cdev_add failed\n");

        goto error_device_number;
    }


    /*
     * Create device class.
     */
    prochardev.class =
        class_create(DRIVER_CLASS);

    if (IS_ERR(prochardev.class))
    {
        ret = PTR_ERR(prochardev.class);

        printk(KERN_ERR
               "prochardev: class creation failed\n");

        goto error_cdev;
    }


    /*
     * Create device node.
     */
    prochardev.device =
        device_create(prochardev.class,
                      NULL,
                      prochardev.dev_num,
                      NULL,
                      DRIVER_NAME);

    if (IS_ERR(prochardev.device))
    {
        ret = PTR_ERR(prochardev.device);

        printk(KERN_ERR
               "prochardev: device creation failed\n");

        goto error_class;
    }


    printk(KERN_INFO
           "prochardev: driver loaded\n");

    return 0;


/*
 * Error cleanup.
 */
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


/*
 * Driver cleanup.
 */
static void __exit prochardev_exit(void)
{
    device_destroy(prochardev.class,
                   prochardev.dev_num);

    class_destroy(prochardev.class);

    cdev_del(&prochardev.cdev);

    unregister_chrdev_region(prochardev.dev_num, 1);

    kfree(prochardev.buffer);

    prochardev.buffer = NULL;

    printk(KERN_INFO
           "prochardev: driver unloaded\n");
}


module_init(prochardev_init);
module_exit(prochardev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vishnu");
MODULE_DESCRIPTION("Professional Linux Character Device Driver");
MODULE_VERSION("1.0");
