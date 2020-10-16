# AOS(Allen's OS)
AOS是一个可运行于i386的32位操作系统。
## 一些重要的前置知识
80386为了兼容8086的程序，有实模式和保护模式的区别。在实模式下，我们可以当作是在8086下编写代码。而在8086中，数据总线有16位，
地址总线却有20位，此时为了寻址到20位，有了这种形式，16位的段地址，16位的段内偏移量，在计算真实的物理地址时，需要段地址左移4位，
然后再加上偏移量。这一点是需要牢记的。另外8086中，段地址一般选取16字节对齐的地址，如：82260h，这样的话，段内偏移量就可以从0开始。

使用NASM语法，在此特别提醒，在对某个内存单元进行读写时，要加上[]，例如：mov ax, [value]（不同于masm等于mov ax, value），
mov ax, [es:di]，没有mov OFFSET指令，nasm的mov ax, value == masm的mov ax, OFFSET value这有别于MASM和x86的汇编语法，
另外的一些区别参见NASM中文手册。
## 大概流程
AOS.img设置到机器的软盘控制器中，将boot.bin写入AOS.img的引导扇区。在AOS.img上建立FAT12文件系统，此时AOS.img的架构如下：

![img-arch](./img/img-arch.PNG)

然后我们利用linux来将loader.bin放在文件系统的根目录下，具体操作为：
1. 在32位红帽中挂载AOS.img，如: mount /home/allen/AOS.img /mnt/floppy -o loop 。
2. 将文件loader.bin复制到 /mnt/floppy下。此时linux会帮我们写好一些信息，比如FAT项，根目录项，将loader.bin的内容写到数据区。
3. 在32位红帽中取消挂载AOS.img。

加电启动后，一旦BIOS发现了一个正确的引导扇区(以0xAA55结尾)，它就会将这个引导扇区的512B的内容加载到内存0x7c00处，这是由硬件决定的。
然后就的指令。跳转到内存0000:0x7c00处开始执行其上的指令。我们可以在boot.bin做一些必要操作(512字节的boot.bin能做的操作还是有限的)，
然后将loader.bin加载到内存当中，接着跳转到loader.bin所处的内存地址，执行其上的指令，就可以跨越512字节的限制了(所以我们可以把很多
指令放在loader.bin中)。

