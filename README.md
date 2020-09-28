# AOS(Allen's OS)
AOS是一个可运行于i386的32位操作系统。
## 大概流程
AOS.img设置到机器的软盘控制器中，将boot.bin写入AOS.img的引导扇区。在AOS.img上建立FAT12文件系统，此时AOS.img的架构如下：

![img-arch](./img/img-arch.PNG)

然后我们利用linux来将loader.bin放在文件系统的根目录下，具体操作为：
1. 在32位红帽中挂载AOS.img，如: mount /home/allen/loader.img /mnt/floppy -o loop 。
2. 将文件loader.bin复制到 /mnt/floppy下。此时linux会帮我们写好一些信息，比如FAT项，根目录项，将loader.bin的内容写到数据区。
3. 在32位红帽中取消挂载AOS.img。

加电启动后，一旦BIOS发现了一个正确的引导扇区(以0xAA55结尾)，
它就会将这个引导扇区的512B的内容加载到内存0x7c00处，这是由硬件决定的。然后就的指令。跳转到内存0000:0x7c00处开始执行其上