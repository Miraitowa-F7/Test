#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/slab.h>

#define DEVICE_NAME "simple_char_driver3"
#define CLASS_NAME "sample_device_class_1"
#define BUFFER_SIZE (PAGE_SIZE)
#define IOCTL_SET_BUFFER_SIZE _IOR('p', 1, int)

// 设备私有数据结构
struct char_device_data {
    dev_t dev_num;
    struct cdev mcdev;
    struct class *device_class;
    char *device_buffer;
    size_t buffer_size;
    size_t buffer_pointer;
};

// 文件操作函数声明
static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char __user *, size_t, loff_t *);
static long dev_ioctl(struct file *, unsigned int, unsigned long);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = dev_read,
    .write = dev_write,
    .open = dev_open,
    .release = dev_release,
    .unlocked_ioctl = dev_ioctl,
};

// 平台驱动的probe函数
static int char_dev_probe(struct platform_device *pdev) {
    struct char_device_data *dev_data;
    int ret;

    // 分配设备私有数据
    dev_data = devm_kzalloc(&pdev->dev, sizeof(*dev_data), GFP_KERNEL);
    if (!dev_data) {
        dev_err(&pdev->dev, "Failed to allocate device data\n");
        return -ENOMEM;
    }

    // 初始化缓冲区
    dev_data->buffer_size = BUFFER_SIZE;
    dev_data->device_buffer = devm_kzalloc(&pdev->dev, dev_data->buffer_size, GFP_KERNEL);
    if (!dev_data->device_buffer) {
        dev_err(&pdev->dev, "Failed to allocate buffer\n");
        return -ENOMEM;
    }

    // 分配设备号
    ret = alloc_chrdev_region(&dev_data->dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        dev_err(&pdev->dev, "Failed to allocate device number\n");
        return ret;
    }

    // 初始化字符设备
    cdev_init(&dev_data->mcdev, &fops);
    dev_data->mcdev.owner = THIS_MODULE;

    // 添加字符设备到系统
    ret = cdev_add(&dev_data->mcdev, dev_data->dev_num, 1);
    if (ret) {
        unregister_chrdev_region(dev_data->dev_num, 1);
        dev_err(&pdev->dev, "Failed to add cdev\n");
        return ret;
    }

    // 创建设备类
    dev_data->device_class = class_create(CLASS_NAME);
    if (IS_ERR(dev_data->device_class)) {
        cdev_del(&dev_data->mcdev);
        unregister_chrdev_region(dev_data->dev_num, 1);
        dev_err(&pdev->dev, "Failed to create class\n");
        return PTR_ERR(dev_data->device_class);
    }

    // 创建设备节点
    if (!device_create(dev_data->device_class, NULL, dev_data->dev_num, NULL, DEVICE_NAME)) {
        class_destroy(dev_data->device_class);
        cdev_del(&dev_data->mcdev);
        unregister_chrdev_region(dev_data->dev_num, 1);
        dev_err(&pdev->dev, "Failed to create device\n");
        return -ENOMEM;
    }

    // 保存设备私有数据到平台设备
    platform_set_drvdata(pdev, dev_data);

    dev_info(&pdev->dev, "Device probed successfully\n");
    return 0;
}

// 平台驱动的remove函数
static int char_dev_remove(struct platform_device *pdev) {
    struct char_device_data *dev_data = platform_get_drvdata(pdev);

    device_destroy(dev_data->device_class, dev_data->dev_num);
    class_destroy(dev_data->device_class);
    cdev_del(&dev_data->mcdev);
    unregister_chrdev_region(dev_data->dev_num, 1);

    dev_info(&pdev->dev, "Device removed\n");
    return 0;
}

// 设备树匹配表
static const struct of_device_id char_dev_of_match[] = {
    { .compatible = "my-company,simple_char_driver3" },
    {},
};
MODULE_DEVICE_TABLE(of, char_dev_of_match);

// 平台驱动结构体
static struct platform_driver char_dev_driver = {
    .probe = char_dev_probe,
    .remove = char_dev_remove,
    .driver = {
        .name = "simple_char_driver3",
        .owner = THIS_MODULE,
        .of_match_table = char_dev_of_match,
    },
};

module_platform_driver(char_dev_driver);

// 文件操作函数实现
static int dev_open(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "chrdev: Device opened\n");
    return 0;
}

static ssize_t dev_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset) {
    struct char_device_data *dev_data = container_of(filep->private_data, struct char_device_data, mcdev);
    int bytes_read = 0;

    while (bytes_read < len && dev_data->buffer_pointer > 0) {
        if (copy_to_user(buffer + bytes_read, dev_data->device_buffer, 1) != 0) break;
        bytes_read++;
        memmove(dev_data->device_buffer, dev_data->device_buffer + 1, --dev_data->buffer_pointer);
    }
    printk(KERN_INFO "chrdev: Sent %d characters to user\n", bytes_read);
    return bytes_read;
}

static ssize_t dev_write(struct file *filep, const char __user *buffer, size_t len, loff_t *offset) {
    struct char_device_data *dev_data = container_of(filep->private_data, struct char_device_data, mcdev);
    int bytes_written = 0;

    while (bytes_written < len && dev_data->buffer_pointer < dev_data->buffer_size) {
        if (copy_from_user(&dev_data->device_buffer[dev_data->buffer_pointer++], buffer + bytes_written, 1) != 0) break;
        bytes_written++;
    }
    printk(KERN_INFO "chrdev: Received %d characters from user\n", bytes_written);
    return bytes_written;
}

static int dev_release(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "chrdev: Device closed\n");
    return 0;
}

static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    struct char_device_data *dev_data = container_of(file->private_data, struct char_device_data, mcdev);
    int new_size;

    switch (cmd) {
        case IOCTL_SET_BUFFER_SIZE:
            if (get_user(new_size, (int __user *)arg)) {
                return -EFAULT;
            }
            if (new_size <= 0) {
                return -EINVAL;
            }
            if (new_size != dev_data->buffer_size) {
                char *new_buffer = krealloc(dev_data->device_buffer, new_size, GFP_KERNEL);
                if (!new_buffer) {
                    return -ENOMEM;
                }
                dev_data->device_buffer = new_buffer;
                dev_data->buffer_size = new_size;
                if (dev_data->buffer_pointer > new_size) {
                    dev_data->buffer_pointer = new_size;
                }
            }
            return 0;
        default:
            return -ENOTTY;
    }
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("F7");
MODULE_DESCRIPTION("A DTS-enabled platform character driver");
MODULE_VERSION("0.2");
