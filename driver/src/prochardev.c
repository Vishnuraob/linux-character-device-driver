#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>

#define DRIVER_NAME  "prochardev"
#define DRIVER_CLASS "prochardev_class"
#define BUFFER_SIZE  100

static dev_t dev_num;
static struct cdev prochardev_cdev;

static struct class *prochardev_class;
static struct device *prochardev_device;

static char kernel_buffer[BUFFER_SIZE];


/*
 * Called when /dev/prochardev is opened.
 */
static int prochardev_open(struct inode *inode,
                           struct file *file)
{
    printk(KERN_INFO "prochardev: device opened\n");

    return 0;
}


/*
 * Called when the device is closed.
 */
static int prochardev_release(struct inode *inode,
                              struct file *file)
{
    printk(KERN_INFO "prochardev: device closed\n");

    return 0;
}


/*
 * Read data from kernel space to user space.
 */
static ssize_t prochardev_read(struct file *file,
                               char __user *buffer,
                               size_t count,
                               loff_t *offset)
{
    size_t data_length;

    data_length = strlen(kernel_buffer);

    /*
     * Nothing more to read.
     */
    if (*offset >= data_length)
        return 0;

    /*
     * Don't read beyond the available data.
     */
    if (count > data_length - *offset)
        count = data_length - *offset;

    /*
     * Copy data from kernel space
     * to user space.
     */
    if (copy_to_user(buffer,
                     kernel_buffer + *offset,
                     count))
    {
        return -EFAULT;
    }

    /*
     * Update file position.
     */
    *offset += count;

    printk(KERN_INFO
           "prochardev: read %zu bytes\n",
           count);

    return count;
}


/*
 * Write data from user space to kernel space.
 */
static ssize_t prochardev_write(struct file *file,
                                const char __user *buffer,
                                size_t count,
                                loff_t *offset)
{
    /*
     * Reserve one byte for '\0'.
     */
    if (count >= BUFFER_SIZE)
        count = BUFFER_SIZE - 1;

    /*
     * Copy data from user space
     * to kernel space.
     */
    if (copy_from_user(kernel_buffer,
                       buffer,
                       count))
    {
        return -EFAULT;
    }

    /*
     * Terminate the string.
     */
    kernel_buffer[count] = '\0';

    printk(KERN_INFO
           "prochardev: received \"%s\"\n",
           kernel_buffer);

    return count;
}


/*
 * File operations table.
 *
 * This connects system calls from user space
 * with our driver functions.
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
 * Module initialization.
 */
static int __init prochardev_init(void)
{
    int ret;

    /*
     * Allocate major/minor numbers.
     */
    ret = alloc_chrdev_region(&dev_num,
                              0,
                              1,
                              DRIVER_NAME);

    if (ret < 0)
    {
        printk(KERN_ERR
               "prochardev: failed to allocate device number\n");

        return ret;
    }

    printk(KERN_INFO
           "prochardev: major=%d minor=%d\n",
           MAJOR(dev_num),
           MINOR(dev_num));


    /*
     * Initialize cdev with our file operations.
     */
    cdev_init(&prochardev_cdev,
              &prochardev_fops);

    prochardev_cdev.owner = THIS_MODULE;


    /*
     * Add character device to kernel.
     */
    ret = cdev_add(&prochardev_cdev,
                   dev_num,
                   1);

    if (ret < 0)
    {
        printk(KERN_ERR
               "prochardev: cdev_add failed\n");

        unregister_chrdev_region(dev_num, 1);

        return ret;
    }


    /*
     * Create device class.
     */
    prochardev_class =
        class_create(DRIVER_CLASS);

    if (IS_ERR(prochardev_class))
    {
        ret = PTR_ERR(prochardev_class);

        cdev_del(&prochardev_cdev);
        unregister_chrdev_region(dev_num, 1);

        return ret;
    }


    /*
     * Create /dev/prochardev.
     */
    prochardev_device =
        device_create(prochardev_class,
                      NULL,
                      dev_num,
                      NULL,
                      DRIVER_NAME);

    if (IS_ERR(prochardev_device))
    {
        ret = PTR_ERR(prochardev_device);

        class_destroy(prochardev_class);
        cdev_del(&prochardev_cdev);
        unregister_chrdev_region(dev_num, 1);

        return ret;
    }


    /*
     * Initial data.
     */
    strcpy(kernel_buffer,
           "Hello from prochardev!\n");


    printk(KERN_INFO
           "prochardev: driver loaded successfully\n");

    return 0;
}


/*
 * Module cleanup.
 */
static void __exit prochardev_exit(void)
{
    /*
     * Remove /dev/prochardev.
     */
    device_destroy(prochardev_class,
                   dev_num);

    /*
     * Remove class.
     */
    class_destroy(prochardev_class);

    /*
     * Remove cdev.
     */
    cdev_del(&prochardev_cdev);

    /*
     * Release major/minor numbers.
     */
    unregister_chrdev_region(dev_num, 1);

    printk(KERN_INFO
           "prochardev: driver unloaded\n");
}


module_init(prochardev_init);
module_exit(prochardev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vishnu");
MODULE_DESCRIPTION("Professional Linux Character Device Driver");
MODULE_VERSION("1.0");
