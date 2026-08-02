#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include "gpio_setup.h"



#define SCREEN_TO_MAGIC 'k'
#define SCREEN_CTL1     _IO(SCREEN_TO_MAGIC, 1)
#define SCREEN_CTL2     _IOR(SCREEN_TO_MAGIC,2,char)
#define SCREEN_CTL3     _IOW(SCREEN_TO_MAGIC,3,char)
#define DEVICE_NAME "max_screen"

static dev_t dev;
static struct cdev my_cdev;
static struct class *my_class;

static int my_open(struct inode *inode, struct file *file) { return 0; }
static int my_release(struct inode *inode, struct file *file) { return 0; }
static ssize_t my_read(struct file *file, char __user *buf, size_t len, loff_t *off) { return 0; }
static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *off)
{
    int i;
    unsigned char *kernel_buf;

    pr_info("screen my_write: \r\n");
    kernel_buf=(unsigned char *) kmalloc(sizeof(unsigned char)*count, 1);
    if(!kernel_buf) return -ENOMEM;

    if(copy_from_user(kernel_buf, buf, count)) return -EFAULT;

    for(i=0;i<count;i++)
    {
        pr_info("screen my_write: %c \r\n", kernel_buf[i]);
    }
    kfree(kernel_buf);
    return(i);

}
static long my_ioctl( struct file *filep, unsigned int command, unsigned long arg)
{

    unsigned char status=0x0;
    int ret=0;

    switch(command)
    {
        case SCREEN_CTL1:
            pr_info("EXECUTING SCREEN_CTL1\n");
            break;
        case SCREEN_CTL2: // read
            ret=__get_user(status,(unsigned char *) arg);
            pr_info("EXECUTING SCREEN_CTL2 (0x%x)\n", status);
            break;
        case SCREEN_CTL3: // write
            pr_info("EXECUTING SCREEN_CTL3, RETURNING 0X30\n");
            status=0x30;
            ret=__put_user(status,(unsigned char *)arg);
            break;
        default:
            ret= -ENOTTY;
    }
    return ret;
}

static struct file_operations fops =
{
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .read = my_read,
    .write = my_write,
    .unlocked_ioctl = my_ioctl,
};


static int __init screen_init(void)
{

    if(gpio_init() != 0)
    {
        pr_err("failed to start gpio\n");
        return -1;
    }

    // register major/minor number
    if(alloc_chrdev_region(&dev, 0,1, DEVICE_NAME) < 0) return -1;

    // initialise instructions for driver
    cdev_init(&my_cdev, &fops);

    // register device with kernel
    if(cdev_add(&my_cdev, dev, 1) < 0)
    {
        unregister_chrdev_region(dev, 1);
        return -1;
    }

    // create class and device node
    my_class = class_create("my_class");
    device_create(my_class, NULL, dev, NULL, DEVICE_NAME);

    return 0;

}

static void __exit screen_exit(void)
{
    device_destroy(my_class, dev);
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev, 1);

    gpio_exit();
}

module_init(screen_init);
module_exit(screen_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("HADI");
MODULE_DESCRIPTION("device char");
MODULE_VERSION("0.01");
