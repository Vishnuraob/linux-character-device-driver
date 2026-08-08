#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>


 /* Module initialization function */
static int __init prochardev_init(void)
{
    printk(KERN_INFO "prochardev: module loaded\n");

    return 0;
}


 /* Module cleanup function */
static void __exit prochardev_exit(void)
{
    printk(KERN_INFO "prochardev: module unloaded\n");
}

module_init(prochardev_init);
module_exit(prochardev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vishnu");
MODULE_DESCRIPTION("Professional Linux Character Device Driver");
MODULE_VERSION("1.0");
