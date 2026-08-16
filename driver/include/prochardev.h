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
#include <linux/mutex.h>
#include <linux/wait.h>

#include "prochardev_ioctl.h"

#define DRIVER_NAME "prochardev"
#define DRIVER_CLASS "prochardev_class"
#define BUFFER_SIZE 256

struct prochardev_device
{
    dev_t dev_num;
    struct cdev cdev;
    struct class *class;
    struct device *device;

    char *buffer;
    size_t read_pos;
    size_t write_pos;
    size_t data_count;

    struct mutex lock;
    wait_queue_head_t read_queue;
};

extern struct prochardev_device prochardev;

int prochardev_alloc_buffer(void);
void prochardev_free_buffer(void);

ssize_t prochardev_buffer_read(char *buffer,
                               size_t count);

ssize_t prochardev_buffer_write(const char *buffer,
                                size_t count);

void prochardev_buffer_clear(void);

int prochardev_open(struct inode *inode,
                    struct file *file);

int prochardev_release(struct inode *inode,
                       struct file *file);

ssize_t prochardev_read(struct file *file,
                        char __user *buffer,
                        size_t count,
                        loff_t *offset);

ssize_t prochardev_write(struct file *file,
                         const char __user *buffer,
                         size_t count,
                         loff_t *offset);

long prochardev_ioctl(struct file *file,
                      unsigned int cmd,
                      unsigned long arg);

extern const struct file_operations prochardev_fops;

#endif
