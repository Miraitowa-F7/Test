# 驱动植入linux系统的过程(in-tree驱动)

1. 修改内核源代码的Kconfig文件，添加配置项，使其出现在 make menuconfig 配置界面中。

2. 在 make menuconfig 界面中选择设备驱动。

3. 修改内核源代码  `drivers/char` 的Makefile文件，添加驱动：

   ```
   obj-y += <driver.o>
   ```

4. 配置内核：通过 `make menuconfig` 进入内核配置界面，在`Device Drivers` -> `Character Devices`路径下启用新添加的驱动配置项。

5. 重新编译内核。

6. 启动QEMU虚拟机，此时新的内核镜像被使用。

