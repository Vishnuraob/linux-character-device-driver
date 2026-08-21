#include "../include/prochardev.h"

// Show the number of bytes currently stored
static ssize_t buffer_used_show(struct device *device,
                                struct device_attribute *attr,
                                char *buf)
{
    struct prochardev_device *dev;
    int used;

    dev = dev_get_drvdata(device);

    mutex_lock(&dev->lock);

    used = dev->data_count;

    mutex_unlock(&dev->lock);

    return sprintf(buf, "%d\n", used);
}

// Show the total buffer size
static ssize_t buffer_size_show(struct device *device,
                                struct device_attribute *attr,
                                char *buf)
{
    return sprintf(buf, "%d\n", BUFFER_SIZE);
}

static DEVICE_ATTR_RO(buffer_used);
static DEVICE_ATTR_RO(buffer_size);

// Create the device attributes
int prochardev_sysfs_create(struct prochardev_device *dev)
{
    int ret;

    ret = device_create_file(dev->device,
                             &dev_attr_buffer_used);

    if (ret)
        return ret;

    ret = device_create_file(dev->device,
                             &dev_attr_buffer_size);

    if (ret)
    {
        device_remove_file(dev->device,
                           &dev_attr_buffer_used);

        return ret;
    }

    return 0;
}

// Remove the device attributes
void prochardev_sysfs_remove(struct prochardev_device *dev)
{
    device_remove_file(dev->device,
                       &dev_attr_buffer_used);

    device_remove_file(dev->device,
                       &dev_attr_buffer_size);
}
