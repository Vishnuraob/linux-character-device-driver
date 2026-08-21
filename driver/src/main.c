#include "../include/prochardev.h"

struct prochardev_device prochardev;

// Initialize the driver
static int __init prochardev_init(void)
{
    int ret;

    ret = prochardev_alloc_buffer();

    if (ret)
        return ret;

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

    printk(KERN_INFO
           "prochardev: major=%d minor=%d\n",
           MAJOR(prochardev.dev_num),
           MINOR(prochardev.dev_num));

    cdev_init(&prochardev.cdev,
              &prochardev_fops);

    prochardev.cdev.owner = THIS_MODULE;

    ret = cdev_add(&prochardev.cdev,
                   prochardev.dev_num,
                   1);

    if (ret < 0)
    {
        printk(KERN_ERR
               "prochardev: cdev_add failed\n");

        goto error_device_number;
    }

    prochardev.class = class_create(DRIVER_CLASS);

    if (IS_ERR(prochardev.class))
    {
        ret = PTR_ERR(prochardev.class);

        goto error_cdev;
    }

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

    ret = prochardev_thread_start();

	if (ret)
	{
    	printk(KERN_ERR
           	"prochardev: failed to start kernel thread\n");

    		goto error_device;
	}

	printk(KERN_INFO
       		"prochardev: driver loaded\n");

    return 0;

error_device:
	device_destroy(prochardev.class,prochardev.dev_num);

error_class:
    class_destroy(prochardev.class);

error_cdev:
    cdev_del(&prochardev.cdev);

error_device_number:
    unregister_chrdev_region(prochardev.dev_num, 1);

error_buffer:
    prochardev_free_buffer();

    return ret;
}

// Clean up the driver
static void __exit prochardev_exit(void)
{
    prochardev_thread_stop();
    device_destroy(prochardev.class,
                   prochardev.dev_num);

    class_destroy(prochardev.class);

    cdev_del(&prochardev.cdev);

    unregister_chrdev_region(prochardev.dev_num, 1);

    prochardev_free_buffer();

    printk(KERN_INFO
           "prochardev: driver unloaded\n");
}

module_init(prochardev_init);
module_exit(prochardev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vishnu");
MODULE_DESCRIPTION("Professional Linux Character Device Driver");
MODULE_VERSION("1.0");
