
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/delay.h>
#include "gpio_setup.h"



#define SCREEN_TO_MAGIC 'k'
#define SCREEN_CTL1_DISABLE     _IO(SCREEN_TO_MAGIC, 1)
#define SCREEN_CTL2_WRITE     _IOR(SCREEN_TO_MAGIC,2,char)
#define SCREEN_CTL3_READ     _IOW(SCREEN_TO_MAGIC,3,char)
#define SCREEN_CTL4_TOGGLE     _IOW(SCREEN_TO_MAGIC, 4, int)
#define DEVICE_NAME "max_screen"

static dev_t dev;
static struct cdev my_cdev;
static struct class *my_class;
static char brightness_settings = 0x0F;
static int decimal_toggle = 1;

static char translate_char(char c)
{
if(c >= '0' && c <= '9') return c - '0';
if (c == 'E' || c == 'e') return 0x0B;
if (c == 'H' || c == 'h') return 0x0C;
if (c == 'L' || c == 'l') return 0x0D;
if (c == 'P' || c == 'p') return 0x0E;
if (c == '-') return 0x0A;
return 0x0F;
}

static int my_open(struct inode *inode, struct file *file)
{

    char *buff = kzalloc(8*sizeof(char), 1); // check over this and maybe use a directive

    if(!buff)
    {
        return -ENOMEM;
    }

    file->private_data = buff;
    screen_send_bits(0x0F, 0x01); // Display Test
    msleep(5000);
    screen_send_bits(0x0F, 0x00);

    file->f_pos = 0;
    return 0;
}
static int my_release(struct inode *inode, struct file *file)
{
    pr_info("my_release: release");

    for(int i=1;i<=8;i++)
    {
        screen_send_bits(i, 0x0F);
    }
    if(file->private_data){
        kfree(file->private_data);
    }
    return 0;
}

static ssize_t my_read( struct file *filep, char *user_buf, size_t count, loff_t *fpos){ // use private data field and attach the buffer to that // note -- change it to just use buff directly
    int i,ret;
    unsigned char *kernel_buf;

    char *buff = (char *)filep->private_data;
    int cursor = filep->f_pos;

    pr_info("my_read: READ \r\n");

    if(*fpos > 0) return 0; // kills infinte loop as the secound loop would be more than zero after the first one runs

    if (count > filep->f_pos) {
        count = filep->f_pos;
    }
    if (count == 0) return 0;

    kernel_buf=(unsigned char *) kmalloc(sizeof(unsigned char)*count,1);
    if(!kernel_buf) return -ENOMEM;

    for(i=0;i<count;i++) kernel_buf[i]=buff[i];
    if(copy_to_user(user_buf, kernel_buf, i)) ret=-EFAULT;
    else if(i==count) ret=count;
    else ret=0;
    kfree(kernel_buf);

    *fpos = ret;

    return(ret);
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *off) // perhaps make a fixed count Directive for all
{
    int i;
    unsigned char *kernel_buf;
    int curr = file->f_pos;
    char *buffer = (char *)file->private_data;

    pr_info("screen my_write: \r\n");
    kernel_buf=(unsigned char *) kmalloc(sizeof(unsigned char)*count, 1);
    if(!kernel_buf) return -ENOMEM;

    if(copy_from_user(kernel_buf, buf, count)) return -EFAULT;

    screen_send_bits(8 - curr, translate_char(buffer[curr]));

    for(i=0;i<count;i++)
    {
    char c = kernel_buf[i];
    unsigned char val = 0x0F;

    if(c == '\n' || c == '\r')
    {
        continue;
    }
    buffer[curr] = c;
    screen_send_bits(8 - curr, translate_char(c));


    curr++;
    if(curr >= 8) curr = 0;

        pr_info("screen my_write: %c \r\n", kernel_buf[i]);
    }

    char isToggled = translate_char(buffer[curr]);
    if(decimal_toggle)
    {
        isToggled |= 0x80;
    }
    screen_send_bits(8 - curr, isToggled);

    kfree(kernel_buf);

    file->f_pos = curr;

    return count;

}
static long my_ioctl( struct file *filep, unsigned int command, unsigned long arg)
{

    unsigned char status=0x0;
    int ret=0;
    int curr = filep->f_pos;
    char *buffer = (char *)filep->private_data;
    char val = 0x0F;


    switch(command)
    {
        case SCREEN_CTL1_DISABLE:
        pr_info("EXECUTING SCREEN_CTL1 DISABLE\n");
        for(int i = 1; i <= 8; i++)
        {
        screen_send_bits(i, 0x0F);
        buffer[i-1] = ' '; // clear the internal private data buffer
        }
            filep->f_pos = 0;
            if(decimal_toggle)
            {
                screen_send_bits(8, translate_char(' ') | 0x80);
            }
            break;
        case SCREEN_CTL2_WRITE:
            ret=__get_user(status,(unsigned char *) arg);

            if( status > 0x0F) status = 0x0F;
            brightness_settings = status;
            screen_send_bits(0x0A, brightness_settings);
            pr_info("EXECUTING SCREEN_CTL2 (0x%x)\n", brightness_settings);
            break;
        case SCREEN_CTL3_READ:
            pr_info("EXECUTING SCREEN_CTL3, RETURNING (0x%x)\n", brightness_settings);
            ret=__put_user(brightness_settings,(unsigned char *)arg);
            break;
        case SCREEN_CTL4_TOGGLE:
            ret=__get_user(decimal_toggle, (int *)arg);
            pr_info("EXECUTING SCREEN_CTL4, CURSOR TOGGLED TO: %d\n", decimal_toggle);

            if (decimal_toggle)
            {
                screen_send_bits(8 - curr, translate_char(buffer[curr]) | 0x80);
            } else {
                screen_send_bits(8 - curr, translate_char(buffer[curr]));
            }

            break;
        default:
            ret= -ENOTTY;
    }
    return ret;
}

static loff_t my_llseek( struct file *filep, loff_t offset, int whence){ // dummy function needs to implement offset calc without creating global vars
    unsigned char curr = filep->f_pos;
    char *buffer = (char *)filep->private_data;
    screen_send_bits(8 - curr, translate_char(buffer[curr]));
    // check the possible seek methods
    switch (whence)
        {
        case 0: // SEEKSET
                pr_info("my_seek: Seek set to offset \r\n");
                curr = offset;
            break ;

        case 1: // SEEKCURRENT
                pr_info("my_seek: Seek set to current position + offset\r\n");
                curr = filep->f_pos + offset;
            break ;

        case 2: // SEEKEND
                pr_info("my_seek: Seek set to end-of-file minus offset\r\n");
                curr = 8 + offset;
            break ;
        default:
            return(-EINVAL) ; // naughty argument
        }

    if(curr > 7) curr = 7;
    if(curr < 0) curr = 0;
    char isToggled = translate_char(buffer[curr]);
    if(decimal_toggle)
    {
        isToggled |= 0x80;
    }
    screen_send_bits(8 - curr, isToggled);
    filep->f_pos=curr;
    return(curr) ;
}


static struct file_operations fops =
{
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .read = my_read,
    .write = my_write,
    .unlocked_ioctl = my_ioctl,
    .llseek     = my_llseek,
};


static int __init screen_init(void)
{

    if(gpio_init() != 0)
    {
        pr_err("failed to start gpio\n");
        return -1;
    }

    start_display();

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
    screen_send_bits(0x0C, 0x00);
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

