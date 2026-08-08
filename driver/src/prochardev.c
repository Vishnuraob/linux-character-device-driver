#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>


 /* Device information */
#define DRIVER_NAME "prochardev"


/* Device number
 * Contains:
 *      Major number
 *      Minor number */
static dev_t dev_num;


 /* Character device structure */
static struct cdev prochardev_cdev;



 /* Module initialization */
static int __init prochardev_init(void)
{
    int ret;

    /*  Dynamically allocate a major number and one minor number. */
    ret = alloc_chrdev_region(&dev_num, 0, 1, DRIVER_NAME);

    if (ret < 0)
    {
        printk(KERN_ERR
               "prochardev: failed to allocate device number\n");

        return ret;
    }

    printk(KERN_INFO
           "prochardev: registered with major=%d minor=%d\n",
           MAJOR(dev_num),
           MINOR(dev_num));

    /*  Initialize the character device. */
    cdev_init(&prochardev_cdev, NULL);

    /* Tell the kernel which device number belongs to this cdev */
    prochardev_cdev.owner = THIS_MODULE;

   /* Add the character device to the kernel */
    ret = cdev_add(&prochardev_cdev, dev_num, 1);

    if (ret < 0)
    {
        printk(KERN_ERR
               "prochardev: failed to add cdev\n");

        unregister_chrdev_region(dev_num, 1);

        return ret;
    }

    printk(KERN_INFO
           "prochardev: character device added\n");

    return 0;
}


/* Module cleanup */
static void __exit prochardev_exit(void)
{
    /* Remove character device.*/
    cdev_del(&prochardev_cdev);

    /* Release major/minor numbers.*/
    unregister_chrdev_region(dev_num, 1);

    printk(KERN_INFO
           "prochardev: character device removed\n");
}


module_init(prochardev_init);
module_exit(prochardev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vishnu");
MODULE_DESCRIPTION("Professional Linux Character Device Driver");
MODULE_VERSION("1.0");
