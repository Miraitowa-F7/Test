#include <linux/module.h>       // 包含module_init, module_exit宏定义
#include <linux/fs.h>           // 提供文件系统相关的函数和数据结构
#include <linux/uaccess.h>      // 提供copy_to_user和copy_from_user函数
#include <linux/init.h>         // 包含初始化宏定义
#include <linux/cdev.h>         // 提供字符设备相关的函数和数据结构
#include <linux/device.h>       // 提供设备创建相关的函数
#include <linux/ioctl.h>        // 提供ioctl相关的定义

#define DEVICE_NAME "sample_device"
#define CLASS_NAME "sample_device_class"
#define BUFFER_SIZE (PAGE_SIZE) // 初始缓冲区大小
#define IOCTL_SET_BUFFER_SIZE _IOR('p', 1, int)  //用于调整缓冲区大小的ioctl命令

// 全局变量声明
static dev_t dev_num;                   // 设备编号
static struct cdev *mcdev;              // 字符设备结构体指针
static struct class *device_class;      // 设备类结构体指针
static char *device_buffer = NULL;      // 使用指针以便动态分配
static size_t buffer_size = BUFFER_SIZE;// 初始缓冲区大小
static size_t buffer_pointer = 0;       // 缓冲区指针，用于追踪当前写入位置

// 文件操作函数原型声明
static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char __user *, size_t, loff_t *);
static long dev_ioctl(struct file *, unsigned int, unsigned long);

// 文件操作结构体，关联了上面声明的函数
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = dev_read,
    .write = dev_write,
    .open = dev_open,
    .release = dev_release,
    .unlocked_ioctl = dev_ioctl, // ioctl处理函数
};

// 模块加载函数
static int __init chrdev_init(void) {
    int err;

    printk(KERN_INFO "chrdev: Starting initialization\n");
    
    //动态分配缓冲区
    device_buffer = kmalloc(buffer_size, GFP_KERNEL);
    if (!device_buffer) {
        printk(KERN_ALERT "chrdev: Failed to allocate memory for buffer\n");
        return -ENOMEM;
    }

    // 分配设备号
    err = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (err < 0) {
        printk(KERN_ALERT "chrdev: Failed to allocate a device number (%d)\n", err);
        return err;
    }
    printk(KERN_INFO "chrdev: Allocated device number (%d,%d)\n", MAJOR(dev_num), MINOR(dev_num));

    // 初始化字符设备结构体
    mcdev = cdev_alloc();
    if (!mcdev) {
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ALERT "chrdev: Failed to allocate memory for character device\n");
        return -ENOMEM;
    }
    mcdev->ops = &fops;
    mcdev->owner = THIS_MODULE;

    // 添加字符设备到内核
    err = cdev_add(mcdev, dev_num, 1);
    if (err) {
        cdev_del(mcdev);
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ALERT "chrdev: Failed to add character device to the system (%d)\n", err);
        return err;
    }

    // 创建设备类
    device_class = class_create(CLASS_NAME);
    if (IS_ERR(device_class)) {
        cdev_del(mcdev);
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ALERT "chrdev: Failed to create device class (%ld)\n", PTR_ERR(device_class));
        return PTR_ERR(device_class);
    }

    // 创建设备文件
    if (device_create(device_class, NULL, dev_num, NULL, DEVICE_NAME) == NULL) {
        class_destroy(device_class);
        cdev_del(mcdev);
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ALERT "chrdev: Failed to create device file\n");
        return -EINVAL;
    }

    printk(KERN_INFO "chrdev: Device created successfully\n");
    return 0;
}

// 模块卸载函数
static void __exit chrdev_exit(void) {
    printk(KERN_INFO "chrdev: Starting cleanup\n");

    device_destroy(device_class, dev_num);               // 销毁设备文件
    class_destroy(device_class);                         // 销毁设备类
    cdev_del(mcdev);                                     // 删除字符设备
    unregister_chrdev_region(dev_num, 1);                // 注销设备号
    kfree(device_buffer);                                // 释放动态分配的缓冲区

    printk(KERN_INFO "chrdev: Cleanup complete\n");
}

// 打开设备时调用
static int dev_open(struct inode *inodep, struct file *filep){
    printk(KERN_INFO "chrdev: Device opened\n");
    return 0;
}

// 读取设备时调用
static ssize_t dev_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset){
    int bytes_read = 0;
    while (bytes_read < len && buffer_pointer > 0) {
        if (copy_to_user(buffer + bytes_read, device_buffer, 1) != 0) break;
        bytes_read++;
        memmove(device_buffer, device_buffer + 1, --buffer_pointer);
    }
    printk(KERN_INFO "chrdev: Sent %d characters to user\n", bytes_read);
    return bytes_read;
}

// 写入设备时调用
static ssize_t dev_write(struct file *filep, const char __user *buffer, size_t len, loff_t *offset){
    int bytes_written = 0;
    while (bytes_written < len && buffer_pointer < BUFFER_SIZE) {
        if (copy_from_user(&device_buffer[buffer_pointer++], buffer + bytes_written, 1) != 0) break;
        bytes_written++;
    }
    printk(KERN_INFO "chrdev: Received %d characters from user\n", bytes_written);
    return bytes_written;
}

// 关闭设备时调用
static int dev_release(struct inode *inodep, struct file *filep){
    printk(KERN_INFO "chrdev: Device closed\n");
    return 0;
}

// ioctl处理函数
static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int new_size;
    switch (cmd) {
        case IOCTL_SET_BUFFER_SIZE:
            if (get_user(new_size, (int __user *)arg)) {
                return -EFAULT;
            }
            if (new_size <= 0) {
                return -EINVAL;
            }
            if (new_size != buffer_size) {//如果新大小与当前大小不同，则重新分配
               char *new_buffer = krealloc(device_buffer, new_size, GFP_KERNEL);
                if (!new_buffer) {
                    return -ENOMEM;
                }
                device_buffer = new_buffer;
                buffer_size = new_size;
            }
            return 0;
        default:
            return -ENOTTY;
    }
}

// 声明模块入口和出口点
module_init(chrdev_init);
module_exit(chrdev_exit);

// 模块信息
MODULE_LICENSE("GPL");
MODULE_AUTHOR("F7");
MODULE_DESCRIPTION("A simple Linux char driver for QEMU environment");
MODULE_VERSION("0.1");
