#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h> // 包含errno的定义

#define DEVICE_PATH "/dev/simple_char_driver2"
#define TEST_STRING "Hello, simple char driver with dynamic buffer!"
#define BUFFER_SIZE 256

// 定义IOCTL命令，需要与内核模块中的定义保持一致
#define IOCTL_SET_BUFFER_SIZE _IOR('p', 1, int)

int main() {
    int fd; // 文件描述符
    char read_buffer[BUFFER_SIZE];
    int new_size = 1024; // 新的缓冲区大小
    int ret_val;

    // 打开设备文件
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device...");
        return errno;
    }

    // 使用ioctl调整缓冲区大小
    printf("Setting buffer size to %d bytes...\n", new_size);
    ret_val = ioctl(fd, IOCTL_SET_BUFFER_SIZE, &new_size);
    if (ret_val < 0) {
        perror("Failed to set buffer size via ioctl.");
        close(fd);
        return errno;
    }
    printf("Buffer size set successfully.\n");

    // 写入数据到设备
    printf("Writing message to %s: '%s'\n", DEVICE_PATH, TEST_STRING);
    ret_val = write(fd, TEST_STRING, strlen(TEST_STRING));
    if (ret_val < 0) {
        perror("Failed to write the message to the device.");
        close(fd);
        return errno;
    }

    // 清空读缓冲区并从设备读取数据
    memset(read_buffer, 0, sizeof(read_buffer));
    ret_val = read(fd, read_buffer, strlen(TEST_STRING));
    if (ret_val < 0) {
        perror("Failed to read the message from the device.");
        close(fd);
        return errno;
    }

    // 显示从设备读取的数据
    printf("Received message from %s: '%s'\n", DEVICE_PATH, read_buffer);

    // 检查读取的数据是否与写入的数据匹配
    if (strncmp(TEST_STRING, read_buffer, strlen(TEST_STRING)) == 0) {
        printf("Test passed, data matches.\n");
    } else {
        printf("Test failed, data does not match.\n");
    }

    // 关闭设备文件
    close(fd);
    return 0;
}
