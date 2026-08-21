#include "../include/prochardev.h"

struct prochardev_device prochardev[DEVICE_COUNT];
struct class *prochardev_class;

// Initialize the driver
static int __init prochardev_init(void)
{
    int ret;
    int i;

    ret = alloc_chrdev_region(&prochardev[0].dev_num,
                              0,
                              DEVICE_COUNT,
                              DRIVER_NAME);

    if (ret < 0)
        return ret;

    prochardev_class = class_create(DRIVER_CLASS);

    if (IS_ERR(prochardev_class))
    {
        ret = PTR_ERR(prochardev_class);
        goto error_region;
    }

    for (i = 0; i < DEVICE_COUNT; i++)
    {
        prochardev[i].dev_num =
            MKDEV(MAJOR(prochardev[0].dev_num), i);

        ret = prochardev_alloc_buffer(&prochardev[i]);

        if (ret)
            goto error_devices;

        cdev_init(&prochardev[i].cdev,
                  &prochardev_fops);

        prochardev[i].cdev.owner = THIS_MODULE;

        ret = cdev_add(&prochardev[i].cdev,
                       prochardev[i].dev_num,
                       1);

        if (ret)
        {
            prochardev_free_buffer(&prochardev[i]);
            goto error_devices;
        }

        prochardev[i].device =
            device_create(prochardev_class,
                          NULL,
                          prochardev[i].dev_num,
                          NULL,
                          "%s%d",
                          DRIVER_NAME,
                          i);

        if (IS_ERR(prochardev[i].device))
        {
            cdev_del(&prochardev[i].cdev);
            prochardev_free_buffer(&prochardev[i]);

            ret = PTR_ERR(prochardev[i].device);
            goto error_devices;
        }

	dev_set_drvdata(prochardev[i].device,&prochardev[i]);
	
	ret = prochardev_sysfs_create(&prochardev[i]);

	if (ret)
	{
    	device_destroy(prochardev_class,
                   prochardev[i].dev_num);

    	cdev_del(&prochardev[i].cdev);

    	prochardev_free_buffer(&prochardev[i]);

    	goto error_devices;
	}
        ret = prochardev_thread_start(&prochardev[i]);

        if (ret)
            goto error_devices;

        printk(KERN_INFO
               "prochardev%d: registered major=%d minor=%d\n",
               i,
               MAJOR(prochardev[i].dev_num),
               MINOR(prochardev[i].dev_num));
    }

    printk(KERN_INFO "prochardev: driver loaded\n");

    return 0;

error_devices:

    while (--i >= 0)
    {
        prochardev_thread_stop(&prochardev[i]);

        device_destroy(prochardev_class,
                       prochardev[i].dev_num);

        cdev_del(&prochardev[i].cdev);

        prochardev_free_buffer(&prochardev[i]);
    }

    class_destroy(prochardev_class);

error_region:

    unregister_chrdev_region(prochardev[0].dev_num,
                             DEVICE_COUNT);

    return ret;
}

// Clean up the driver
static void __exit prochardev_exit(void)
{
    int i;

    for (i = 0; i < DEVICE_COUNT; i++)
    {
        prochardev_thread_stop(&prochardev[i]);

        prochardev_sysfs_remove(&prochardev[i]);

	device_destroy(prochardev_class,
                       prochardev[i].dev_num);

        cdev_del(&prochardev[i].cdev);

        prochardev_free_buffer(&prochardev[i]);
    }

    class_destroy(prochardev_class);

    unregister_chrdev_region(prochardev[0].dev_num,
                             DEVICE_COUNT);

    printk(KERN_INFO
           "prochardev: driver unloaded\n");
}

module_init(prochardev_init);
module_exit(prochardev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vishnu");
MODULE_DESCRIPTION("Professional Linux Character Device Driver");
MODULE_VERSION("1.0");
