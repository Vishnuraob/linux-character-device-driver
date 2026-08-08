#ifndef PROCHARDEV_H
#define PROCHARDEV_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/string.h>

#define DRIVER_NAME     "prochardev"
#define DRIVER_CLASS    "prochardev_class"
#define BUFFER_SIZE     100

/* Driver context.
  This structure contains all information belonging to our device.*/
struct prochardev_device
{
    dev_t dev_num;

    struct cdev cdev;

    struct class *class;

    struct device *device;

    char *buffer;
};

/* Global driver instance.*/
extern struct prochardev_device prochardev;

#endif
