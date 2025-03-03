# Week7 Task2 Linux内核启动流程

## 一、总体流程

`start_kernel` → 各子系统初始化 → `rest_init` → `kernel_init` → 用户空间

## 二、函数调用过程

### 1. start_kernel()

#### 1.1 硬件和基础初始化

```c
// init/main.c
set_task_stack_end_magic(&init_task);  // 初始化任务栈保护（防溢出）
smp_setup_processor_id();             // 设置当前 CPU ID
boot_cpu_init();                      // 激活启动 CPU
page_address_init();                  // 初始化页地址映射
setup_arch(&command_line);            // 架构相关初始化（如解析设备树）
mm_init();                            // 内存管理子系统初始化
sched_init();                         // 调度器初始化
time_init();                          // 时钟源初始化
```

对应日志：

```
Booting Linux on physical CPU 0x0
CPU: ARMv7 Processor [410fc090] revision 0 (ARMv7)
OF: fdt: Machine model: V2P-CA9
Memory: 488692K/524288K available
```

#### 1.2 中断和时钟初始化

```c
// init/main.c
early_irq_init();       // 早期中断控制器初始化
init_IRQ();             // 中断描述符表（IDT）设置
tick_init();            // 时钟中断框架初始化
hrtimers_init();        // 高精度定时器初始化
```

对应日志：

```
GIC CPU mask not found - kernel will fail to boot.  // GIC 初始化警告
Calibrating delay loop... 2026.70 BogoMIPS         // CPU 频率校准
clocksource: arm,sp804: mask: 0xffffffff           // 时钟源注册
```

#### 1.3  架构初始化`setup_arch()`

```c
// arch/arm/kernel/setup.c
void __init setup_arch(char **cmdline_p) {
    // 解析设备树，获取硬件信息
    setup_machine_fdt(__fdt_pointer);
    // 初始化内存区域（如保留内存、CMA）
    arm_memblock_init();
    // 设置内核命令行参数
    *cmdline_p = boot_command_line;
}
```

### 2. 设备驱动和子系统初始化 `do_initcalls()`

调用层级（按优先级从高到低）：

```c
static initcall_entry_t *initcall_levels[] __initdata = {
	__initcall0_start,    //pure (0)：无依赖的早期驱动（如内存分配器）
	__initcall1_start,    //core (1)：核心子系统（如 `rcu_init`, `workqueue_init_early`）
	__initcall2_start,    //postcore(2): 依赖核心的模块（如 `acpi_early_init`）
	__initcall3_start,    //arch(3): 架构特定驱动（如 ARM 平台设备）  
	__initcall4_start,    //subsys(4): 子系统（如网络栈 `net_ns_init`）
	__initcall5_start,    //fs(5): 文件系统（如 `vfs_caches_init`）  
	__initcall6_start,    //device(6): 设备驱动（如 `pl011_register_port` 串口驱动）
	__initcall7_start,    //late(7): 最后阶段（如用户空间辅助线程）  
	__initcall_end,
};
```

```c
static const char *initcall_level_names[] __initdata = {
	"pure",
	"core",
	"postcore",
	"arch",
	"subsys",
	"fs",
	"device",
	"late",
};
```

对应日志：

```
Serial: AMBA PL011 UART driver          // 串口驱动初始化（device 级别）
mmc0: new SD card at address 6da8       // MMC 驱动初始化（device 级别）
drm-clcd-pl111 10020000.clcd: initializing...  // 显示驱动初始化（device 级别）
```

### 3. 创建用户进程：`rest_init()`

```
![Linux内核启动流程](E:\Arm_Linux_Work\Test\Week7\Task2\Linux内核启动流程.jpg)![Kernel启动函数调用流程图](C:\Users\17716\Downloads\Kernel启动函数调用流程图.png)// init/main.c
noinline void __ref rest_init(void) {
    // 创建 init 进程（PID=1）
    pid = user_mode_thread(kernel_init, NULL, CLONE_FS);
    // 创建内核线程 kthreadd（PID=2）
    pid = kernel_thread(kthreadd, NULL, CLONE_FS | CLONE_FILES);
    // 启动空闲线程（进入 cpu_idle 循环）
    cpu_startup_entry(CPUHP_ONLINE);
}
```

### 4. 用户空间初始化：`kernel_init()`

```
// init/main.c
static int __ref kernel_init(void *unused) {
    // 挂载根文件系统
    prepare_namespace();
    // 执行用户空间 init 程序
    if (execute_command)
        run_init_process(execute_command);
    else
        run_init_process("/sbin/init");
    return 0;
}
```

对应日志：

```
EXT4-fs (mmcblk0): mounted filesystem 59e7573b-36b8-49c5-a7c4-8f62abe15ccc
VFS: Mounted root (ext4 filesystem) on device 179:0.
Run /sbin/init as init process
random: crng init done                // 随机数生成器就绪
```

## 三、流程图

![](E:\Arm_Linux_Work\Test\Week7\Task2\Linux内核启动流程.jpg)
