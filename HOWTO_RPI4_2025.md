# 在树莓派4B上编译、安装、运行 Jailhouse（Version 25.1）

[TOC]

> 本篇文档将详细介绍在树莓派4B（BCM2711）上编译、安装、运行 Jailhouse 的详细流程，致力于为初学者简化 Jailhouse 环境搭建流程。



## 背景概念

### 树莓派4B

——Your tiny, dual-display, desktop computer

![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/e758c4e9d5d0487cb10c8600c6344eea.png)

**设计背景**：树莓派 4B 的诞生有着深厚的渊源。早在 2006 年，剑桥大学计算机实验室为应对计算机系申请人数下滑和学生技能不足的状况，萌生出开发一款小型廉价计算机以普及编程和计算机科学教育的想法，这便是树莓派的初衷。随着 ARM 架构处理器性能的不断攀升以及电子制造工艺的进步，低成本高性能的单板计算机成为可能，为树莓派 4B 的诞生奠定了技术基础。加之先前几代树莓派在市场上收获了一定成功，用户对其性能和功能有了更高期待与更多创意需求，促使了树莓派 4B 的升级设计。

**设计目的**：树莓派 4B 的设计肩负着多重使命。在教育层面，它致力于为学生打造更优质的编程学习与计算机科学实践平台，助力学生深入理解计算机原理与编程知识，激发对计算机科学的浓厚兴趣。对于 DIY 爱好者和极客们而言，树莓派 4B 凭借强大性能和丰富接口，能够满足他们将创意转化为现实的需求，实现诸如智能家居控制、自制机器人等个性化项目。此外，树莓派 4B 还着眼于拓展应用场景，使其能够在物联网、工业自动化等领域发挥低成本控制核心和数据处理节点的作用，推动设备的互联互通与智能化控制。

**市场反响**：树莓派 4B 在市场上收获了广泛的积极反馈。在教育领域，它已被全球众多学校纳入计算机课程，成为学生学习 Linux 操作系统和 Python 编程的得力工具，深受教育工作者和学生的赞誉。DIY 爱好者和极客社区对树莓派 4B 的热情高涨，其凭借强大性能、丰富接口和低廉成本，成为大家开发新奇有趣项目的首选，各类创意项目成果在网络上广泛分享，进一步推动了其流行。商业应用中，树莓派 4B 也崭露头角，被用于物联网设备开发、智能家居系统搭建以及工业自动化控制等，有效降低了开发与设备成本。媒体对树莓派 4B 的评价颇高，消费者购买热情持续不减，全球销量稳步增长。

### Jailhouse

Jailhouse Hypervisor 是一款基于 Linux 的轻量级分区虚拟机管理程序。

![请添加图片描述](https://i-blog.csdnimg.cn/blog_migrate/321ebb6b41bc6f8ad844595565021516.png)

**诞生背景**：在工业自动化、医疗、电信和高性能计算等领域，实时虚拟化的特殊需求难以被传统通用虚拟化解决方案，如 KVM、Xen 等满足。同时，对于敏感软件系统，需要与不可认证的工作负载进行安全隔离，而当时的虚拟化软件要么过于臃肿，要么隔离效果欠佳。在此情况下，迫切需要一款小型、简单且对开源 Linux 友好的 Hypervisor，Jailhouse 应运而生。

**设计目的**：Jailhouse 致力于提供简单高效的虚拟化方案，它聚焦于简单性与资源隔离，不支持资源超额分配，仅虚拟化硬件平台中那些无法通过硬件分区实现的必需资源，以此达成高效的资源管理。此外，它通过将硬件资源划分为多个独立的 “单元”，让每个单元能够运行不同的操作系统或裸机应用程序，确保各单元间互不干扰，实现硬件资源的安全隔离与高效利用。

![请添加图片描述](https://i-blog.csdnimg.cn/direct/eb315551801d4ae59ce470642e47d86e.png)

**应用场景**：Jailhouse 在多个领域都有重要应用。在嵌入式系统中，如智能交通、工业自动化设备等，可同时运行 Linux 和实时操作系统，分别处理非实时管理任务与实时控制任务。在安全关键领域，像金融、医疗、军事等，能构建起安全隔离的应用环境，保障关键数据和业务的安全可靠。在云计算与服务器虚拟化方面，作为轻量级虚拟化方案，为用户提供安全隔离的计算环境，适用于对性能和资源隔离要求较高的场景。

**发展现状**：Jailhouse 在不断发展进步。功能上，引入 GitHub Actions 进行持续集成，采用 Coverity Scan 进行静态代码分析，以确保代码质量、修复潜在漏洞；同时扩展了对更多 ARM 架构板卡的支持，如 NVIDIA Jetson TX1、TX2 以及 Xilinx ZCU102 等，并改进了配置工具。社区方面，作为开源项目，其在 GitHub 上提供源代码，拥有活跃的社区支持，用户可通过多种渠道获取帮助和资源。随着技术成熟，其应用范围也在不断扩大，在更多领域得到应用，如 OpenHarmony 系统中已使用 Jailhouse 进行 RTOS 虚拟化。

![请添加图片描述](https://i-blog.csdnimg.cn/direct/72ac8959c2ed42089f3284d6e95884da.png)

## 准备工作

### 准备物料

1. RaspberryPi 4B

2. USB-TTL模块及杜邦线（例如：CH340）。通过下位机开发板上的 GPIO 接口（通常为 RXD、TXD、VCC、GND 四个引脚）与上位机进行数据传输。

	![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/73207fc055f34803b45457a4d78861a1.png)

3. SD卡（树莓派使用）、读卡器（访问存储卡内文件）、网线。

### 环境准备

1. 主机：ubuntu 20.04，注意后续烧录工具所支持的系统版本。

2. 将空白SD卡插入读卡器后，连接到电脑USB口。

3. 下载 Raspberry Pi Imager 烧录工具[^1]，建议下载版本v1.7.4（https://github.com/raspberrypi/rpi-imager/releases/download/v1.7.4/rpi-imager_1.7.4_amd64.deb）且无需更新。

	如果你使用 ubuntu22.04 LTS，可能会遇到如下安装问题：（[imager for linux won't install on Ubuntu 22.04.2 LTS](https://github.com/raspberrypi/rpi-imager/issues/587)）

	![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/5ed8f44e823a4182bb6fe6668084087b.png)

	解决方法：`sudo apt install ./*.deb`。

	![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/e975abcf3f004b04a62114efaede55bf.png)

	选择 OS 版本：ubuntu server 20.04，选择存储卡路径，点击烧录即可。

4. 树莓派连接网线、串口，然后上电。

	![](https://i-blog.csdnimg.cn/blog_migrate/5cc1460465f9e44838b4424c783ca6e6.png)

5. 使用 minicom 串口调试工具查看树莓派串口输出：

	![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/f96e17a4e57a4244be66cd3066d68227.png)

	用户名：ubuntu，密码：ubuntu。首次登录后需要修改密码。

	在串口环境中，执行如下命令：

	```bash
	sudo apt-get install -y net-tools
	```

	查看树莓派 ip：`ifconfig`，然后在命令行中使用 `ssh` 命令连接：`ssh ubuntu@192.168.2.80`

	之后我们就可以通过 `ssh` 命令远程连接的方式访问树莓派了。

	树莓派上安装其它工具：

	```bash
	sudo apt-get install -y vim git make gcc
	sudo apt-get install -y build-essential libncurses* libssl-dev bison flex
	```

	你在安装上述工具时，可能会遇到该问题：`E: Unable to fetch some archives, maybe run apt-get update or try with --fix-missing? 的error`。可以做出如下尝试：使用命令 `apt-get update`  更新 source，如果依然报错，那就说明这个source 本身有错误，尝试 `apt-get update --fix-missing` ，如果依旧报错，那么就需要更换换source。替换 `/etc/apt/sources.list` 文件里面的内容（参考：https://www.cnblogs.com/X-knight/p/10598498.html）。

6. 关闭树莓派。

7. （记得备份源文件）准备 cmdline.txt 文件：

	```bash
	elevator=deadline net.ifnames=0 console=serial0,115200 dwc_otg.lpm_enable=0 console=tty1 root=LABEL=writable rootfstype=ext4 rootwait fixrtc quiet splash mem=768M
	```

8. （记得备份源文件）准备 config.txt 文件：

	```bash
	# Please DO NOT modify this file; if you need to modify the boot config, the
	# "usercfg.txt" file is the place to include user changes. Please refer to
	# the README file for a description of the various configuration files on
	# the boot partition.
	
	# The unusual ordering below is deliberate; older firmwares (in particular the
	# version initially shipped with bionic) don't understand the conditional
	# [sections] below and simply ignore them. The Pi4 doesn't boot at all with
	# firmwares this old so it's safe to place at the top. Of the Pi2 and Pi3, the
	# Pi3 uboot happens to work happily on the Pi2, so it needs to go at the bottom
	# to support old firmwares.
	
	[pi4]
	kernel=uboot_rpi_4.bin
	
	[pi2]
	kernel=uboot_rpi_2.bin
	
	[pi3]
	kernel=uboot_rpi_3.bin
	
	[pi0]
	kernel=uboot_rpi_3.bin
	
	[all]
	device_tree_address=0x03000000
	
	[pi4]
	max_framebuffers=2
	arm_boost=1
	
	[all]
	# Enable the audio output, I2C and SPI interfaces on the GPIO header. As these
	# parameters related to the base device-tree they must appear *before* any
	# other dtoverlay= specification
	dtparam=audio=on
	dtparam=i2c_arm=on
	dtparam=spi=on
	
	# Comment out the following line if the edges of the desktop appear outside
	# the edges of your display
	disable_overscan=1
	
	# If you have issues with audio, you may try uncommenting the following line
	# which forces the HDMI output into HDMI mode instead of DVI (which doesn't
	# support audio output)
	#hdmi_drive=2
	
	# Config settings specific to arm64
	arm_64bit=1
	dtoverlay=dwc2
	
	[cm4]
	# Enable the USB2 outputs on the IO board (assuming your CM4 is plugged into
	# such a board)
	dtoverlay=dwc2,dr_mode=host
	
	[all]
	
	# The following settings are "defaults" expected to be overridden by the
	# included configuration. The only reason they are included is, again, to
	# support old firmwares which don't understand the "include" command.
	
	enable_uart=1
	enable_gic=1
	armstub=bl31.bin
	cmdline=cmdline.txt
	
	include syscfg.txt
	include usercfg.txt
	```

	

9. （主机环境）准备 Linux 源码和 bl31.bin 文件。下载 jailhouse-images 项目，使用项目中的 Linux 源码、bl31.bin 固件资源。（本环节可能由于网络环境等诸多原因导致项目源码无法拉取，可以下载备用源码：[**基于树莓派4B的Jailhouse源码资源**](https://download.csdn.net/download/weixin_39541632/87858987?spm=1001.2014.3001.5501)）

  ```bash
  git clone -b v0.12 https://github.com/siemens/jailhouse-images.git
  ```

  可能存在的问题及解决方案：[Git报错： Failed to connect to github.com port 443 解决方案](https://blog.csdn.net/zpf1813763637/article/details/128340109)、[git clone github上的项目时出现报错“Proxy CONNECT aborted”解决方法](https://blog.csdn.net/qq_43481435/article/details/124198130)

  ```bash
  git config --global https.proxy "socks5://127.0.0.1:1080"
  git config --global http.proxy "socks5://127.0.0.1:1080"
  ```

  `git` 代理问题解决后，进入项目目录：`sudo ./build` 

  可能遇到的问题：Failed to connect to github.com port 443: Connection timed out

  解决方法：可能需要临时取消全局代理？

  ```bash
  git config --global --unset http.proxy
  git config --global --unset https.proxy
  ```

  查看代理命令：

  ```bash
  git config --global --get http.proxy
  git config --global --get https.proxy
  ```

  `bl31.bin`文件位置：`jailhouse-images/build/tmp/work/jailhouse-demo-arm64/arm-trusted-firmware-rpi4/2.2-r0/arm-trusted-firmware-2.2/build/rpi4/release/bl31.bin`

  Linux 源码位置：`jailhouse-images/build/tmp/work/jailhouse-demo-arm64/linux-jailhouse-rpi/5.4.16-r0/linux-e569bd2d6d2d7b958973bb8c6e9db9cfc05c790b`。（Linux 源码的另一来源：`https://github.com/siemens/linux.git`）

  内核默认配置文件位置：`jailhouse-images/build/tmp/work/jailhouse-demo-arm64/linux-jailhouse-rpi/5.4.16-r0/rpi4_defconfig_5.4`。如果在树莓派上进行内核编译，可无需此文件。

  

  **将 bl31.bin 固件、cmdline.txt、config.txt 文件拷贝到树莓派启动分区（system-boot），将 Linux 源码、内核配置文件拷贝到用户家目录下。然后重启树莓派。**

  

## 内核编译与替换

为了稳定，树莓派内核将使用 5.4.16(linux-jailhouse-rpi4)，而非树莓派原生内核！故需要重新编译内核并进行内核替换。

查看当前内核版本：

```bash
ubuntu@ubuntu:~$ uname -a
Linux ubuntu 5.4.0-1069-raspi #79-Ubuntu SMP PREEMPT Thu Aug 18 18:15:22 UTC 2022 aarch64 aarch64 aarch64 GNU/Linux
```



### 内核编译

将生成的 rpi4_defconfig_5.4 文件重命名为 .config，保存到内核源码目录下，在内核源码目录中，使用如下命令进行内核编译。为了更快速编译内核，可以使用 /boot 分区下的原生 cmdline.txt 文件，以使用更大内存，或者删除 `mem=768M` 字段。
1. `make menuconfig`，进行内核功能的定制。
2. `make`，默认使用内核源码目录下的.config 配置文件编译内核。
3. `sudo make modules_install`，安装内核对应的模块、驱动程序。
4. `sudo make install`，安装内核。

可能遇到的问题：

```bash
Using DTB: bcm2711-rpi-4-b.dtb
Couldn't find DTB bcm2711-rpi-4-b.dtb on the following paths: /etc/flash-kernel/dtbs /usr/lib/linux-image-5.4.16 /lib/firmware/5.4.16/device-tree/
Installing  into /boot/dtbs/5.4.16/./bcm2711-rpi-4-b.dtb
cp: cannot stat '': No such file or directory
run-parts: /etc/initramfs/post-update.d//flash-kernel exited with return code 1
run-parts: /etc/kernel/postinst.d/initramfs-tools exited with return code 1
make[1]: *** [arch/arm64/boot/Makefile:40: install] Error 1
make: *** [arch/arm64/Makefile:140: install] Error 2
```

问题解决：`sudo cp arch/arm64/boot/dts/broadcom/bcm2711-rpi-4-b.dtb /etc/flash-kernel/dtbs/`

### 替换内核

将源码 `/boot` 目录下的 `vmlinuz`、 `initrd.img` 拷贝到 `/boot/firmware` 目录下， 替换该目录下的对应文件，实现内核替换。

```bash
sudo cp /boot/vmlinuz-5.4.16 /boot/firmware/vmlinuz
sudo cp /boot/initrd.img-5.4.16 /boot/firmware/initrd.img
sudo cp /boot/dtbs/5.4.16/bcm2711-rpi-4-b.dtb /boot/firmware/bcm2711-rpi-4-b.dtb
```

重启树莓派，查看当前内核版本：

```bash
ubuntu@ubuntu:~$ uname -a
Linux ubuntu 5.4.16 #19 SMP PREEMPT Tue Jan 21 07:34:59 UTC 2025 aarch64 aarch64 aarch64 GNU/Linux
```



## Jailhouse 编译安装环节

1. 下载 Jailhouse 源码

```bash
git clone -b v0.12 https://github.com/siemens/jailhouse.git # 尤其注意下载0.12版本，master版本可能不行！ 
```

2. 安装 Python 相关工具

```bash
# 安装python相关工具
sudo apt-get install python
sudo apt-get install aptitude
sudo apt-get install python3-pip
sudo apt-get install python3-mako
aptitude search python
sudo apt-get install python-is-python3 # ubuntu18.04不支持该命令，可以手动实现，类似这样，将python指向python3，q@q-GA-MA785GMT-US2H:~/jailhouse$ ln -s /usr/bin/python3 /usr/bin/python，这样输入 python 命令之后，默认指向 Python3.x
```

3. 编译安装 Jailhouse
```bash
ubuntu@ubuntu:~/jailhouse-0.12$ make
ubuntu@ubuntu:~/jailhouse-0.12$ sudo make install
```

​	

编译安装成功：

![](https://i-blog.csdnimg.cn/direct/deaf4bc68a224cddbbdc4e6878b847a2.png)



## Jailhouse 运行环节

1. 使能 Jailhouse

	```bash
	ubuntu@ubuntu:~/jailhouse-0.12$ sudo insmod driver/jailhouse.ko # 如果 Invalid argument，则make clean，重新make即可，或者是启动了其它内核，修改对应Makefile.lib文件，make && sudo make install 即可
	ubuntu@ubuntu:~/jailhouse-0.12$ sudo jailhouse enable configs/arm64/rpi4.cell
	ubuntu@ubuntu:~/jailhouse-0.12$ jailhouse cell list
	ID      Name                    State             Assigned CPUs           Failed CPUs             
	0       Raspberry-Pi4           running           0-3                                             
	ubuntu@ubuntu:~/jailhouse-0.12$ sudo jailhouse console 
	
	Initializing Jailhouse hypervisor v0.12 on CPU 3
	Code location: 0x0000ffffc0200800
	Page pool usage after early setup: mem 39/994, remap 0/131072
	Initializing processors:
	 CPU 3... OK
	 CPU 0... OK
	 CPU 2... OK
	 CPU 1... OK
	Initializing unit: irqchip
	Initializing unit: ARM SMMU v3
	Initializing unit: PVU IOMMU
	Initializing unit: PCI
	Adding virtual PCI device 00:00.0 to cell "Raspberry-Pi4"
	Adding virtual PCI device 00:01.0 to cell "Raspberry-Pi4"
	Page pool usage after late setup: mem 61/994, remap 5/131072
	Activating hypervisor
	```

	![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/8eb556769c864d95a2d61f43d35b1e4e.png)
	
2. Jailhouse 常用命令

	```bash
	ubuntu@ubuntu:~/jailhouse-0.12$ jailhouse --help 
	Usage: jailhouse { COMMAND | --help | --version }
	
	Available commands:
	   enable SYSCONFIG
	   disable
	   console [-f | --follow]
	   cell create CELLCONFIG
	   cell list
	   cell load { ID | [--name] NAME } { IMAGE | { -s | --string } "STRING" }
	             [-a | --address ADDRESS] ...
	   cell start { ID | [--name] NAME }
	   cell shutdown { ID | [--name] NAME }
	   cell destroy { ID | [--name] NAME }
	   cell linux CELLCONFIG KERNEL [-i | --initrd FILE]
	              [-c | --cmdline "STRING"] [-w | --write-params FILE]
	   cell stats { ID | [--name] NAME }
	   config create [-h] [-g] [-r ROOT] [--mem-inmates MEM_INMATES]
	                 [--mem-hv MEM_HV] FILE
	   config collect FILE.TAR
	   hardware check
	```

3. 创建 Linux cell

	```bash
	ubuntu@ubuntu:~/jailhouse-0.12$ jailhouse cell linux -h
	usage: jailhouse cell linux [-h] [--dtb DTB] [--initrd FILE] [--cmdline "STRING"] [--write-params FILE] [--arch ARCH] [--kernel-decomp-factor N]
	                            CELLCONFIG KERNEL
	
	Boot Linux in a non-root cell.
	
	positional arguments:
	  CELLCONFIG            cell configuration file
	  KERNEL                image of the kernel to be booted
	
	optional arguments:
	  -h, --help            show this help message and exit
	  --dtb DTB, -d DTB     device tree for the kernel [arm/arm64 only]
	  --initrd FILE, -i FILE
	                        initrd/initramfs for the kernel
	  --cmdline "STRING", -c "STRING"
	                        kernel command line
	  --write-params FILE, -w FILE
	                        only parse cell configuration, write out parameters into the specified file and print required jailhouse cell commands to
	                        boot Linux to the console
	  --arch ARCH, -a ARCH  target architecture
	  --kernel-decomp-factor N, -k N
	                        decompression factor of the kernel image, used to reserve space between the kernel and the initramfs
	
	```

	在树莓派4B的实际操作中，`jailhouse cell linux ...` 命令，对于 `-i FILE` 参数中 `FILE` 可以不指定，亦可指定 `/boot/firmware/initrd.img`，亦可自定义根文件系统（.cpio）。

	Linux cell 启动命令：

	```bash
	sudo jailhouse cell linux configs/arm64/rpi4-linux-demo.cell /boot/vmlinuz-5.4.16 -d configs/arm64/dts/inmate-rpi4.dtb -i /home/ubuntu/rootfs.cpio -c "console=ttyS0,115200 ip=192.168.19.2"
	```

	![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/45a6156b045b4c48b83a638ae0b43db1.png)

4. 创建 inmate-demo 裸机程序

	```bash
	ubuntu@ubuntu:~/jailhouse-0.12$ sudo jailhouse cell create configs/arm64/rpi4-inmate-demo.cell
	ubuntu@ubuntu:~/jailhouse-0.12$ sudo jailhouse cell load inmate-demo inmates/demos/arm64/gic-demo.bin
	ubuntu@ubuntu:~/jailhouse-0.12$ sudo jailhouse cell start inmate-demo
	ubuntu@ubuntu:~/jailhouse-0.12$ jailhouse cell list 
	ID      Name                    State             Assigned CPUs           Failed CPUs             
	0       Raspberry-Pi4           running           0                                               
	1       rpi4-linux-demo         running           2-3                                             
	2       inmate-demo             running           1                                               
	ubuntu@ubuntu:~/jailhouse-0.12$ sudo jailhouse cell destroy 
	--name           1                2                inmate-demo      rpi4-linux-demo  
	ubuntu@ubuntu:~/jailhouse-0.12$ sudo jailhouse cell destroy 2
	
	```

	![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/248c36a07c604004b570fd55697ee117.png)

	![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/cf6eb1f9ab984480bd81d2a446314529.png)

	当同时启动了 linux cell 后，串口会复用。

## 自定义文件系统

1. 参考链接：[使用QEMU（x86）模拟运行ARM64架构并进行内核调试](https://blog.csdn.net/weixin_39541632/article/details/129910433?spm=1001.2014.3001.5501#t14)的使用 busybox小节构建 busybox 文件系统。
2. rootfs 中删除 linuxrc 文件，`ln -s bin/busybox init`
3. 执行命令打包：`find -print0 | cpio -0 -oH newc | gzip -9 > ../rootfs.cpio`



## Jailhouse 串口输出内容

使能 Jailhouse，并且运行 Linux cell 过程下的串口输出。

```bash
Initializing Jailhouse hypervisor v0.12 on CPU 3
Code location: 0x0000ffffc0200800
Page pool usage after early setup: mem 39/994, remap 0/131072
Initializing processors:
 CPU 3... OK
 CPU 0... OK
 CPU 2... OK
 CPU 1... OK
Initializing unit: irqchip
Initializing unit: ARM SMMU v3
Initializing unit: PVU IOMMU
Initializing unit: PCI
Adding virtual PCI device 00:00.0 to cell "Raspberry-Pi4"
Adding virtual PCI device 00:01.0 to cell "Raspberry-Pi4"
Page pool usage after late setup: mem 61/994, remap 5/131072
Activating hypervisor
[50960.364779] pci 0001:00:00.0: failed to get arch_dma_ops
[50960.370754] pci 0001:00:01.0: failed to get arch_dma_ops
Adding virtual PCI device 00:00.0 to cell "rpi4-linux-demo"
Shared memory connection established, peer cells:
 "Raspberry-Pi4"
Adding virtual PCI device 00:01.0 to cell "rpi4-linux-demo"
Shared memory connection established, peer cells:
 "Raspberry-Pi4"
Created cell "rpi4-linux-demo"
Page pool usage after cell creation: mem 76/994, remap 5/131072
Cell "rpi4-linux-demo" can be loaded
Started cell "rpi4-linux-demo"
[    0.000000] Booting Linux on physical CPU 0x0000000002 [0x410fd083]
[    0.000000] Linux version 5.4.16 (ubuntu@ubuntu) (gcc version 9.4.0 (Ubuntu 9.4.0-1ubuntu1~20.04.2)) #19 SMP PREEMPT Tue Jan 21 07:34:59 UTC 2025
[    0.000000] Machine model: Jailhouse cell on Raspberry Pi 4
[    0.000000] efi: Getting EFI parameters from FDT:
[    0.000000] efi: UEFI not found.
[    0.000000] cma: Reserved 8 MiB at 0x0000000037800000
[    0.000000] psci: probing for conduit method from DT.
[    0.000000] psci: PSCIv1.1 detected in firmware.
[    0.000000] psci: Using standard PSCI v0.2 function IDs
[    0.000000] psci: MIGRATE_INFO_TYPE not supported.
[    0.000000] psci: SMC Calling Convention v1.1
[    0.000000] percpu: Embedded 31 pages/cpu s87064 r8192 d31720 u126976
[    0.000000] Detected PIPT I-cache on CPU0
[    0.000000] CPU features: detected: EL2 vector hardening
[    0.000000] Built 1 zonelists, mobility grouping on.  Total pages: 32256
[    0.000000] Kernel command line: console=ttyS0,115200 ip=192.168.19.2
[    0.000000] Dentry cache hash table entries: 16384 (order: 5, 131072 bytes, linear)
[    0.000000] Inode-cache hash table entries: 8192 (order: 4, 65536 bytes, linear)
[    0.000000] mem auto-init: stack:off, heap alloc:off, heap free:off
[    0.000000] Memory: 81488K/131072K available (9276K kernel code, 1070K rwdata, 3204K rodata, 1024K init, 1195K bss, 41392K reserved, 8192K cma-res)
[    0.000000] SLUB: HWalign=64, Order=0-3, MinObjects=0, CPUs=2, Nodes=1
[    0.000000] ftrace: allocating 30609 entries in 120 pages
[    0.000000] rcu: Preemptible hierarchical RCU implementation.
[    0.000000] rcu:     RCU restricting CPUs from NR_CPUS=256 to nr_cpu_ids=2.
[    0.000000]  Tasks RCU enabled.
[    0.000000] rcu: RCU calculated value of scheduler-enlistment delay is 25 jiffies.
[    0.000000] rcu: Adjusting geometry for rcu_fanout_leaf=16, nr_cpu_ids=2
[    0.000000] NR_IRQS: 64, nr_irqs: 64, preallocated irqs: 0
[    0.000000] random: get_random_bytes called from start_kernel+0x324/0x4b4 with crng_init=0
[    0.000000] arch_timer: cp15 timer(s) running at 54.00MHz (virt).
[    0.000000] clocksource: arch_sys_counter: mask: 0xffffffffffffff max_cycles: 0xc743ce346, max_idle_ns: 440795203123 ns
[    0.000005] sched_clock: 56 bits at 54MHz, resolution 18ns, wraps every 4398046511102ns
[    0.000145] Console: colour dummy device 80x25
[    0.000192] Calibrating delay loop (skipped), value calculated using timer frequency.. 108.00 BogoMIPS (lpj=216000)
[    0.000212] pid_max: default: 32768 minimum: 301
[    0.000468] Mount-cache hash table entries: 512 (order: 0, 4096 bytes, linear)
[    0.000488] Mountpoint-cache hash table entries: 512 (order: 0, 4096 bytes, linear)
[    0.001539] Disabling memory control group subsystem
[    0.024173] ASID allocator initialised with 32768 entries
[    0.032180] rcu: Hierarchical SRCU implementation.
[    0.040310] EFI services will not be available.
[    0.048269] smp: Bringing up secondary CPUs ...
[    0.080555] Detected PIPT I-cache on CPU1
[    0.080657] CPU1: Booted secondary processor 0x0000000003 [0x410fd083]
[    0.080873] smp: Brought up 1 node, 2 CPUs
[    0.080897] SMP: Total of 2 processors activated.
[    0.080912] CPU features: detected: 32-bit EL0 Support
[    0.080927] CPU features: detected: CRC32 instructions
[    0.107363] CPU: All CPU(s) started at EL1
[    0.107424] alternatives: patching kernel code
[    0.108456] devtmpfs: initialized
[    0.109693] Enabled cp15_barrier support
[    0.109742] Enabled setend support
[    0.110146] clocksource: jiffies: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 7645041785100000 ns
[    0.110176] futex hash table entries: 512 (order: 3, 32768 bytes, linear)
[    0.110598] pinctrl core: initialized pinctrl subsystem
[    0.111224] DMI not present or invalid.
[    0.111535] NET: Registered protocol family 16
[    0.115030] DMA: preallocated 256 KiB pool for atomic allocations
[    0.115498] cpuidle: using governor menu
[    0.115614] hw-breakpoint: found 6 breakpoint and 4 watchpoint registers.
[    0.115797] Serial: AMBA PL011 UART driver
[    0.145576] vgaarb: loaded
[    0.145973] SCSI subsystem initialized
[    0.146198] usbcore: registered new interface driver usbfs
[    0.146255] usbcore: registered new interface driver hub
[    0.146334] usbcore: registered new device driver usb
[    0.147487] clocksource: Switched to clocksource arch_sys_counter
[    0.859401] VFS: Disk quotas dquot_6.6.0
[    0.859555] VFS: Dquot-cache hash table entries: 512 (order 0, 4096 bytes)
[    0.859748] FS-Cache: Loaded
[    0.859972] CacheFiles: Loaded
[    0.872917] thermal_sys: Registered thermal governor 'step_wise'
[    0.873125] NET: Registered protocol family 2
[    0.873861] tcp_listen_portaddr_hash hash table entries: 256 (order: 0, 4096 bytes, linear)
[    0.873896] TCP established hash table entries: 1024 (order: 1, 8192 bytes, linear)
[    0.873926] TCP bind hash table entries: 1024 (order: 2, 16384 bytes, linear)
[    0.873961] TCP: Hash tables configured (established 1024 bind 1024)
[    0.874083] UDP hash table entries: 256 (order: 1, 8192 bytes, linear)
[    0.874118] UDP-Lite hash table entries: 256 (order: 1, 8192 bytes, linear)
[    0.874334] NET: Registered protocol family 1
[    0.875204] RPC: Registered named UNIX socket transport module.
[    0.875222] RPC: Registered udp transport module.
[    0.875234] RPC: Registered tcp transport module.
[    0.875246] RPC: Registered tcp NFSv4.1 backchannel transport module.
[    0.875268] PCI: CLS 0 bytes, default 64
[    0.875541] Trying to unpack rootfs image as initramfs...
[    0.940073] Freeing initrd memory: 8432K
[    0.941682] Initialise system trusted keyrings
[    0.942009] workingset: timestamp_bits=46 max_order=15 bucket_order=0
[    0.950307] FS-Cache: Netfs 'nfs' registered for caching
[    0.951036] NFS: Registering the id_resolver key type
[    0.951079] Key type id_resolver registered
[    0.951093] Key type id_legacy registered
[    0.951117] nfs4filelayout_init: NFSv4 File Layout Driver Registering...
[    0.952359] Key type asymmetric registered
[    0.952376] Asymmetric key parser 'x509' registered
[    0.952424] Block layer SCSI generic (bsg) driver version 0.4 loaded (major 250)
[    0.956731] io scheduler mq-deadline registered
[    0.956748] io scheduler kyber registered
[    0.957317] pci-host-generic e0000000.pci: host bridge /pci@e0000000 ranges:
[    0.957370] pci-host-generic e0000000.pci:   MEM 0x10000000..0x1000ffff -> 0x10000000
[    0.957497] pci-host-generic e0000000.pci: ECAM at [mem 0xe0000000-0xe00fffff] for [bus 00]
[    0.957653] pci-host-generic e0000000.pci: PCI host bridge to bus 0000:00
[    0.957673] pci_bus 0000:00: root bus resource [bus 00]
[    0.957690] pci_bus 0000:00: root bus resource [mem 0x10000000-0x1000ffff]
[    0.957764] pci 0000:00:00.0: [110a:4106] type 00 class 0xff0000
[    0.957871] pci 0000:00:00.0: reg 0x10: [mem 0x00000000-0x00000fff]
[    0.958400] pci 0000:00:01.0: [110a:4106] type 00 class 0xff0001
[    0.958500] pci 0000:00:01.0: reg 0x10: [mem 0x00000000-0x00000fff]
[    0.960553] pci 0000:00:00.0: BAR 0: assigned [mem 0x10000000-0x10000fff]
[    0.960587] pci 0000:00:01.0: BAR 0: assigned [mem 0x10001000-0x10001fff]
[    0.961334] Serial: 8250/16550 driver, 1 ports, IRQ sharing enabled
[    0.962353] printk: console [ttyS0] disabled
[    0.962449] fe215040.serial: ttyS0 at MMIO 0x0 (irq = 5, base_baud = 62500000) is a 16550
[    1.639268] printk: console [ttyS0] enabled
[    1.644515] vc-mem: phys_addr:0x00000000 mem_base=0x00000000 mem_size:0x00000000(0 MiB)
[    1.653123] cacheinfo: Unable to detect cache hierarchy for CPU 0
[    1.670270] brd: module loaded
[    1.684054] loop: module loaded
[    1.687364] Loading iSCSI transport class v2.0-870.
[    1.694058] libphy: Fixed MDIO Bus: probed
[    1.698455] usbcore: registered new interface driver r8152
[    1.704145] usbcore: registered new interface driver lan78xx
[    1.709997] usbcore: registered new interface driver smsc95xx
[    1.716158] ivshmem-net 0000:00:01.0: enabling device (0000 -> 0002)
[    1.722848] ivshmem-net 0000:00:01.0: TX memory at 0x000000003fb80000, size 0x000000000007f000
[    1.731657] ivshmem-net 0000:00:01.0: RX memory at 0x000000003fb01000, size 0x000000000007f000
[    1.741470] uio_ivshmem 0000:00:00.0: enabling device (0000 -> 0002)
[    1.748093] uio_ivshmem 0000:00:00.0: state_table at 0x000000003faf0000, size(s) 0x0000000000001000
[    1.757349] uio_ivshmem 0000:00:00.0: rw_section at 0x000000003faf1000, size(s) 0x0000000000009000
[    1.766498] uio_ivshmem 0000:00:00.0: input_sections at 0x000000003fafa000, size(s) 0x0000000000006000
[    1.775992] uio_ivshmem 0000:00:00.0: output_section at 0x000000003fafe000, size(s) 0x0000000000002000
[    1.785473] IVSHM_PRIV_CNTL_ONESHOT_INT=1
[    1.790094] dwc_otg: version 3.00a 10-AUG-2012 (platform bus)
[    1.800331] usbcore: registered new interface driver uas
[    1.805817] usbcore: registered new interface driver usb-storage
[    1.812114] mousedev: PS/2 mouse device common for all mice
[    1.818304] sdhci: Secure Digital Host Controller Interface driver
[    1.824624] sdhci: Copyright(c) Pierre Ossman
[    1.829219] sdhci-pltfm: SDHCI platform and OF driver helper
[    1.835202] ledtrig-cpu: registered to indicate activity on CPUs
[    1.841397] hidraw: raw HID events driver (C) Jiri Kosina
[    1.847013] usbcore: registered new interface driver usbhid
[    1.852707] usbhid: USB HID core driver
[    1.856919] Initializing XFRM netlink socket
[    1.861330] NET: Registered protocol family 17
[    1.865973] Key type dns_resolver registered
[    1.870641] registered taskstats version 1
[    1.874843] Loading compiled-in X.509 certificates
[    1.880477] of_cfs_init
[    1.883058] of_cfs_init: OK
[    1.903535] IP-Config: Guessing netmask 255.255.255.0
[    1.908714] IP-Config: Complete:
[    1.912036]      device=eth0, hwaddr=a6:3f:7e:50:36:b8, ipaddr=192.168.19.2, mask=255.255.255.0, gw=255.255.255.255
[    1.922670]      host=192.168.19.2, domain=, nis-domain=(none)
[    1.928627]      bootserver=255.255.255.255, rootserver=255.255.255.255, rootpath=
[    1.940394] Freeing unused kernel memory: 1024K
[    1.951593] Run /init as init process
Starting syslogd: OK
Starting klogd: OK
Running sysctl: OK
Saving random seed: [    2.035839] random: dd: uninitialized urandom read (512 bytes read)
OK
Starting dropbear sshd: [    2.058528] random: dropbear: uninitialized urandom read (32 bytes read)
OK

Welcome to Buildroot
jailhouse login: root     
# ls
ivshmem-demo
```



---

**参考资料**

+ [树莓派官方文档](https://www.raspberrypi.com/documentation/)
+ [Jailhouse论坛](https://bbs.csdn.net/forums/jailhouse?spm=1001.2014.3001.6682)
+ [基于树莓派4B的Jailhouse源码资源](https://download.csdn.net/download/weixin_39541632/87858987?spm=1001.2014.3001.5501)

[^1]:https://www.raspberrypi.com/software/

