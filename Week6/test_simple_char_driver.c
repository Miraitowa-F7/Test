#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      // 提供 open 函数的头文件
#include <unistd.h>     // 提供 read, write, close 函数的头文件
#include <string.h>     // 提供 strlen 函数的头文件

#define DEVICE_PATH "/dev/sample_device"
#define BUFFER_SIZE 1024

int main() {
    int fd; // 文件描述符
    char write_buffer[BUFFER_SIZE];
    char read_buffer[BUFFER_SIZE];
    int bytes_read, bytes_written;

    // 打开设备文件
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        printf("无法打开设备文件 %s\n", DEVICE_PATH);
        return -1;
    }

    // 向设备写入数据
    strcpy(write_buffer, "Hello, device!");
    bytes_written = write(fd, write_buffer, strlen(write_buffer));
    if (bytes_written < 0) {
        printf("写入设备失败\n");
        close(fd);
        return -1;
    }
    printf("成功写入 %d 字节: %s\n", bytes_written, write_buffer);

    // 从设备读取数据
    bytes_read = read(fd, read_buffer, BUFFER_SIZE-1);
    if (bytes_read < 0) {
        printf("从设备读取数据失败\n");
        close(fd);
        return -1;
    }
    read_buffer[bytes_read] = '\0'; // 确保字符串以 null 结尾
    printf("从设备读取了 %d 字节: %s\n", bytes_read, read_buffer);

    // 关闭设备文件
    close(fd);

    return 0;
}
