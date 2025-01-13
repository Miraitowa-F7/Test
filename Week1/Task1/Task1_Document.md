# Task1

## 一. Linux简介

​		Linux，一般指GNU/Linux（单独的Linux内核并不可直接使用，一般搭配GNU套件，故得此称呼），是一种免费使用和自由传播的类UNIX操作系统，其内核由林纳斯·本纳第克特·托瓦兹（Linus Benedict Torvalds）于1991年10月5日首次发布，它主要受到Minix和Unix思想的启发，是一个基于POSIX的多用户、多任务、支持多线程和多CPU的操作系统。它支持32位和64位硬件，能运行主要的Unix工具软件、应用程序和网络协议。
​		Linux继承了Unix以网络为核心的设计思想，是一个性能稳定的多用户网络操作系统。Linux有上百种不同的发行版，如基于社区开发的debian、archlinux，和基于商业开发的Red Hat Enterprise Linux、SUSE、Oracle Linux等。

### 1. 诞生背景

- **1991年**：当时，大多数高性能的操作系统都是商业产品，价格昂贵且源代码封闭。而自由软件基金会（FSF）的 GNU 项目旨在创建一个完全自由的类 Unix 操作系统，但缺少一个关键组件——内核。
- **Linus Torvalds**：作为一名计算机科学学生，林纳斯·托瓦兹在赫尔辛基大学对 MINIX（一种用于教育目的的小型 Unix 系统）感兴趣，但他希望有一个更强大、更适合个人电脑的替代品。因此，他开始编写自己的操作系统内核。

### 2. 发展历史

- **1991年9月17日**：Linus 在 Usenet 上发布了 Linux 内核的第一个版本 0.01，这是一个非常基础的内核，仅支持 Intel 80386 处理器，并且只有有限的功能。
- **GNU/Linux**：随着 Linux 内核的发展，它逐渐与 GNU 工具集结合，形成了完整的操作系统。这个组合通常被称为 GNU/Linux。
- **开源运动**：Linux 成为了开源软件运动的标志性项目，吸引了全球开发者参与贡献。这种开放的合作方式加速了 Linux 的发展和改进。
- **广泛采用**：随着时间的推移，Linux 不仅成为了服务器市场的主导者，还在嵌入式系统、移动设备（如 Android）、超级计算机等领域取得了显著的成功。



## 二. Linux内核架构

​		Linux内核是操作系统的核心部分，**它负责管理系统的资源、进程调度、设备驱动程序等核心功能**。

![](C:\Users\17716\WPSDrive\1123784315\WPS云盘\实习\GaoTong\Task1\Figures\Figure2.png)

​		Linux内核的主要组件有：系统调用接口、进程管理、内存管理、虚拟文件系统、网络堆栈、设备驱动程序、硬件架构等。

**1. 系统调用接口**

​        SCI 层提供了某些机制执行从用户空间到内核的函数调用。这个接口依赖于体系结构，甚至在相同的处理器家族内也是如此。SCI 实际上是一个非常有用的函数调用多路复用和多路分解服务。在` ./linux/kernel` 中可以找到 SCI 的实现，并在 `./linux/arch` 中找到依赖于体系结构的部分。

**2. 进程管理**

​         进程管理的重点是进程的执行。在内核中，这些进程称为线程，代表了单独的处理器虚拟化（线程代码、数据、堆栈和 CPU 寄存器）。在用户空间，通常使用进程 这个术语，不过 Linux 实现并没有区分这两个概念（进程和线程）。内核通过 SCI 提供了一个应用程序编程接口（API）来创建一个新进程（`fork`、`exec` 或 `Portable Operating System Interface [POSIX]` 函数），停止进程（`kill`、`exit`），并在它们之间进行通信和同步（signal 或者 POSIX 机制）。

​        进程管理还包括处理活动进程之间共享 CPU 的需求。内核实现了一种新型的调度算法，不管有多少个线程在竞争 CPU，这种算法都可以在固定时间内进行操作。

**3. 内存管理**

​        为了提高效率，如果由硬件管理虚拟内存，内存是按照所谓的内存页方式进行管理的（对于大部分体系结构来说都是 4KB）。Linux 包括了管理可用内存的方式，以及物理和虚拟映射所使用的硬件机制。不过内存管理要管理的可不止 4KB 缓冲区。Linux 提供了对 4KB 缓冲区的抽象，例如 slab 分配器。这种内存管理模式使用 4KB 缓冲区为基数，然后从中分配结构，并跟踪内存页使用情况，比如哪些内存页是满的，哪些页面没有完全使用，哪些页面为空。这样就允许该模式根据系统需要来动态调整内存使用。为了支持多个用户使用内存，有时会出现可用内存被消耗光的情况。由于这个原因，页面可以移出内存并放入磁盘中。这个过程称为交换，因为页面会被从内存交换到硬盘上。

**4. 虚拟文件系统**

​        虚拟文件系统（VFS）是 Linux 内核中非常有用的一个方面，因为它为文件系统提供了一个通用的接口抽象。VFS 在 SCI 和内核所支持的文件系统之间提供了一个交换层。

<img src="C:\Users\17716\WPSDrive\1123784315\WPS云盘\实习\GaoTong\Task1\Figures\Figure3.png"  />

​        在 VFS 上面，是对诸如 `open`、`close`、`read` 和 `write` 之类的函数的一个通用 API 抽象。在 VFS 下面是文件系统抽象，它定义了上层函数的实现方式。它们是给定文件系统（超过 50 个）的插件。文件系统层之下是缓冲区缓存，它为文件系统层提供了一个通用函数集（与具体文件系统无关）。这个缓存层通过将数据保留一段时间（或者随即预先读取数据以便在需要是就可用）优化了对物理设备的访问。缓冲区缓存之下是设备驱动程序，它实现了特定物理设备的接口。

**5. 网络堆栈**

​        网络堆栈在设计上遵循模拟协议本身的分层体系结构。回想一下，Internet Protocol (IP) 是传输协议（通常称为传输控制协议或 TCP）下面的核心网络层协议。TCP 上面是 socket 层，它是通过 SCI 进行调用的。socket 层是网络子系统的标准 API，它为各种网络协议提供了一个用户接口。从原始帧访问到 IP 协议数据单元（PDU），再到 TCP 和 User Datagram Protocol (UDP)，socket 层提供了一种标准化的方法来管理连接，并在各个终点之间移动数据。

**6. 设备驱动程序**

​        Linux 内核中有大量代码都在设备驱动程序中，它们能够运转特定的硬件设备。Linux 源码树提供了一个驱动程序子目录，这个目录又进一步划分为各种支持设备，例如 Bluetooth、I2C、serial 等。

**7. 依赖体系结构的代码**

​        尽管 Linux 很大程度上独立于所运行的体系结构，但是有些元素则必须考虑体系结构才能正常操作并实现更高效率。`./linux/arch` 子目录定义了内核源代码中依赖于体系结构的部分，其中包含了各种特定于体系结构的子目录（共同组成了 BSP）。对于一个典型的桌面系统来说，使用的是 x86 目录。每个体系结构子目录都包含了很多其他子目录，每个子目录都关注内核中的一个特定方面，例如引导、内核、内存管理等。

## 三. 调用关系

### 1. 硬件到驱动程序

- **硬件抽象层 (HAL)**：Linux 内核通过 HAL 来处理不同硬件平台之间的差异。HAL 提供了统一的接口给上层使用，使得开发者不需要为每个硬件设备编写特定的代码。
  
- **设备驱动程序**：驱动程序是操作系统与硬件之间通信的桥梁。它们是专门为控制和管理特定硬件而设计的软件组件。例如，一个网卡驱动程序负责初始化网卡、配置其工作模式、发送和接收数据包等。

  - 当系统启动时，内核会探测可用的硬件并加载相应的驱动程序。
  - 驱动程序可以被编译进内核（静态链接）或作为可加载模块（动态加载）。后者允许更灵活地添加或移除功能，而不必重启系统。

### 2. 驱动程序到内核模块

- **内核模块**：可以在运行时动态插入或卸载的内核代码片段。它们扩展了内核的功能，比如支持新的文件系统、网络协议或硬件设备。
  - 每个驱动程序通常都是一个或多个内核模块的形式存在。
  - 内核模块通过一组定义好的接口（如 `module_init()` 和 `module_exit()` 函数）来注册自身以及它所提供的服务或设备。
  - 模块可以直接访问内核空间的数据结构和服务，从而实现对硬件的低级操作。

### 3. 内核模块到用户态

- **系统调用接口 (SCI)**：这是用户空间的应用程序与内核交互的主要方式。应用程序通过系统调用来请求内核执行特权操作，如进程创建、内存分配、文件操作等。

  - 用户空间的代码不能直接访问硬件或执行某些关键任务；它必须通过系统调用向内核发出请求。
  - 系统调用是操作系统提供的一组受控入口点，用于执行需要高权限的操作。当应用程序发起系统调用时，CPU 会切换到内核模式，执行相应的内核函数，完成后返回用户模式。

- **库函数**：为了简化编程，Linux 提供了标准 C 库（glibc）和其他库，它们封装了底层的系统调用。例如，`open()`、`read()`、`write()` 等库函数实际上会转化为相应的系统调用。

### 4. 用户态应用

- **应用程序**：最终用户看到的是运行在用户空间的应用程序。这些程序利用库函数提供的高级 API 进行开发，而无需关心底层细节。

  - 应用程序可以通过库函数间接地调用系统调用，进而让内核代表它们执行必要的操作。
  - 用户态的应用程序运行在一个受限的环境中，它们不能直接访问硬件资源或进行任何可能危害系统稳定性的操作。

### 示例流程

以打开一个文件为例，以下是整个调用链的一个简化版本：

1. **用户态应用程序** 调用 `fopen()` 函数（来自 glibc）。
2. `fopen()` 内部调用了 `open()` 系统调用。
3. 系统调用机制将 CPU 切换到 **内核模式** 并调用内核中的 `sys_open()` 函数。
4. `sys_open()` 可能会与相关的 **文件系统模块** 或 **设备驱动程序** 交互，以确定文件的位置，并获取所需的元数据。
5. 如果涉及到磁盘 I/O，那么 **磁盘驱动程序** 将负责实际的数据读取或写入。
6. 完成后，所有层级逐级返回结果给用户态应用程序，后者可以继续处理文件内容。



# Q&A

 - ## Q1 Linux系统是宏内核还是微内核？

   宏内核，内核服务和用户服务都工作在相同的地址空间。

   

 - ## Q2 Linux中的用户模式和内核模式是什么含义？

   在Linux内核中，用户模式和内核模式是两种不同的CPU操作行模式，它们定义了程序可以执行的操作范围和权限级别。

   ### 用户模式(User Mode）

   权限限制：用户模式下的进程运行在一个受限的环境中，只能访问有限的内存区域和执行非特权指令，这种模式主要用于运行应用程序。

   资源访问：用户模式进程不能直接访问硬件设备，必须通过系统调用请求内核提供服务，如文件操作、网络通信等。

   安全性：由于资源受限，用户模式提供了一层安全保护，防止应用程序直接访问关键系统资源或其他应用程序的数据，风险较低。

   ### 内核模式（Kernel Mode）

   权限提升：内核模式下的进程或代码拥有执行任何CPU指令和访问所有内存的能力，内核代码可以直接与硬件交互，执行如内存管理、进程调度低级任务。

   资源访问：内核模式提供了对系统全资源的完全访问权限，包括直接操作硬件设备的能力。

   风险性：内核模式下的错误或恶意代码可能会导致系统崩溃或安全漏洞，风险较高。

   

 - ## Q3 用户进程间通信主要哪几种方式？

   进程间的通信方式主要有六种：**管道、消息队列、共享内存、信号量、信号、套接字。**

   **管道**：管道本质上是内核中的一个缓存，当进程创建一个管道后，Linux会返回两个文件描述符（写入端和读取端），通过这两个描述符向管道写入或读取数据，管道间通信需要使用fock创建子进程，创建的子进程会复制父进程的文件描述符，此时两个进程各有两个写入端读取段描述符，两个进程通过各自的fd读取写入同一个管道文件实现跨进程通信。

   **消息队列**：消息队列是消息的链接表，包括Posix消息队列和system V消息队列，有足够权限的进程可以向队列中添加消息，被赋予读权限的进程可以读走队列中的消息。消息队列是保存在内核中的消息链表，在发送数据时，会分成一个一个独立的数据单元，即消息体（数据块）。

   **共享内存**：共享内存的机制是拿出一块虚拟空间来，映射到相同物理内存中，当一个进程发生写入操作时另一个进程可以马上读取，不需要进行拷贝操作。

   **信号量**：为了防止多进程共享资源而造成数据混乱，需要一种保护机制使共享资源在某一时刻只能被一个进程访问，该机制被成为信号量。信号量是一个整型的计数器，主要用于实现进程间的互斥与同步，而不是用于缓存进程间通信的数据。

   **信号**：信号用于异常情况。信号是进程通信机制中唯一的异步通信机制，可以在任何时候发送信号给某一进程，可以执行默认操作、捕捉信号或忽略信号。

   **套接字(Socket)**：Socket通信可以实现跨网络与不同主机上的进程进行通信。

   


