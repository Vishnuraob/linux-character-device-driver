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
#include <linux/poll.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#include "prochardev_ioctl.h"

#define DRIVER_NAME "prochardev"
#define DRIVER_CLASS "prochardev_class"
#define BUFFER_SIZE 256
#define DEVICE_COUNT 2

struct prochardev_device
{
    dev_t dev_num;
    struct cdev cdev;
    struct device *device;

    char *buffer;
    size_t read_pos;
    size_t write_pos;
    size_t data_count;

    struct mutex lock;
    wait_queue_head_t read_queue;

    struct task_struct *thread;
};

extern struct class *prochardev_class;
extern struct prochardev_device prochardev[DEVICE_COUNT];

int prochardev_alloc_buffer(struct prochardev_device *dev);
void prochardev_free_buffer(struct prochardev_device *dev);

ssize_t prochardev_buffer_read(struct prochardev_device *dev,
                               char *buffer,
                               size_t count);

ssize_t prochardev_buffer_write(struct prochardev_device *dev,
                                const char *buffer,
                                size_t count);

void prochardev_buffer_clear(struct prochardev_device *dev);

int prochardev_thread_start(struct prochardev_device *dev);
void prochardev_thread_stop(struct prochardev_device *dev);

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
