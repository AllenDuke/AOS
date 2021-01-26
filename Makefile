# ======================================================================================================================
#   AOS内核的内存挂载点
# ----------------------------------------------------------------------------------------------------------------------
# 这个值必须存在且相等在文件"loader.inc"中的 'KERNEL_ENTRY_POINT_PHY_ADDR'！
ENTRY_POINT     = 0x100000
# ======================================================================================================================


# ======================================================================================================================
# ----------------------------------------------------------------------------------------------------------------------
# 一些变量
# ----------------------------------------------------------------------------------------------------------------------386

# 内核头文件目录
i = src/kernel/include
ic = $i/core
ib = $i/ibm

# 内核C文件所在目录
sk = src/kernel

# 库文件所在目录
lansi = src/kernel/lib/ansi
lstdio = src/kernel/lib/stdio
lstdlib = src/kernel/lib/stdlib
li386 = src/kernel/lib/i386

# 编译链接中间目录
t = target
tb = $t/boot
tk = $t/kernel
tl = $t/lib

# 所需软盘镜像，可以指定已存在的软盘镜像，系统内核将被写入到这里
Img = AOS.img
# 硬盘镜像 -- 还没有


# 镜像挂载点，自指定，必须存在于自己的计算机上，没有请自己创建一下
ImgMountPoint = /mnt/floppy

# 所需要的编译器以及编译参数
ASM             = nasm
CC              = gcc
LD              = ld
ASMFlagsOfBoot  = -I src/boot/include/
ASMFlagsOfKernel= -f elf -I src/kernel/include/core/
CFlags			= -std=c99 -I $i -c -fno-builtin -Wall
LDFlags			= -Ttext $(ENTRY_POINT) -Map kernel.map  # -Ttext 选项参数用来调整elf文件的可执行代码中的p_vaddr的值

AR              = ar
ARFLAGS		    = rcs
# ======================================================================================================================


# ======================================================================================================================
# ----------------------------------------------------------------------------------------------------------------------
#   目标程序以及编译的中间文件
# ----------------------------------------------------------------------------------------------------------------------
AOSBoot      = $(tb)/boot.bin $(tb)/loader.bin
AOSKernel    = $(tk)/kernel.bin

# 内核，只实现基本功能
KernelObjs      = $(tk)/kernel.o $(tk)/main.o $(tk)/kernel_i386lib.o $(tk)/protect.o \
                  $(tk)/init_c.o $(tk)/exception.o $(tk)/panic.o $(tk)/i8259.o $(tk)/clock.o \
                  $(tk)/process.o $(tk)/ipc.o $(tk)/dump.o $(tk)/keyboard.o $(tk)/tty.o \
                  $(tk)/console.o $(tk)/idle.o $(tk)/alloc.o $(tk)/mm.o $(tk)/fork.o $(tk)/mem_map.o \
                  $(tk)/exit.o $(tk)/wait.o $(tk)/at_wini.o $(tk)/fs.o $(tk)/open.o $(tk)/fs_misc.o \
                  $(tk)/read_write.o $(tk)/link.o $(tk)/fs_test.o $(tk)/tty_test.o $(tk)/exec.o \
                  $(tk)/misc.o $(tk)/origin.o $(tk)/hash.o $(tk)/pwd.o $(tk)/date.o $(tk)/echo.o \
                  $(tk)/cat.o $(tk)/touch.o $(tk)/vi.o $(tk)/clear.o $(tk)/rm.o

# 内核之外所需要的库，有系统库，也有提供给用户使用的库
LIB		        = $(l)/aos_lib.a
LibObjs         = $(AnsiObjs) $(StdioObjs) $(I386Objs) $(StdlibObjs)
AnsiObjs        = $(tl)/ansi/string.o $(tl)/ansi/memcmp.o $(tl)/ansi/cstring.o
StdioObjs       = $(tl)/stdio/printf.o $(tl)/stdio/open.o $(tl)/stdio/close.o $(tl)/stdio/write.o \
                  $(tl)/stdio/read.o $(tl)/stdio/stat.o $(tl)/stdio/exit.o $(tl)/stdio/vsprintf.o \
                  $(tl)/stdio/fork.o $(tl)/stdio/exec.o $(tl)/stdio/wait.o $(tl)/stdio/unlink.o
I386Objs        = $(tl)/i386/ipc/ipc.o
StdlibObjs      = $(tl)/stdlib/get_time.o $(tl)/stdlib/change_console.o $(tl)/stdlib/clean_console.o

Objs            = $(KernelObjs) $(LibObjs)
# ======================================================================================================================


# ======================================================================================================================
# ----------------------------------------------------------------------------------------------------------------------
# 所有的功能伪命令，需要严格的制表符
# ----------------------------------------------------------------------------------------------------------------------
.PHONY: nop all image debug run clean realclean
# 默认选项（输入make但没有跟参数，自动执行），提示一下用户我们的makefile有哪些功能
nop:
	@echo "everything              编译所有文件，生成目标文件(二进制文件，boot.bin)"
	@echo "image            生成系统镜像文件"
	@echo "debug            打开bochs进行系统的运行和调试"
	@echo "run              提示用于如何将系统安装到虚拟机上运行"
	@echo "clean            清理所有的中间编译文件"
	@echo "realclean        完全清理：清理所有的中间编译文件以及生成的目标文件（二进制文件）	"

# 编译所有文件
everything: $(AOSBoot) $(AOSKernel) $(LIB) # 这表示依赖的东西
	@echo "已经生成 AOS 内核！"

# 生成系统镜像文件
image: $(Img) $(AOSBoot)
	dd if=$(tb)/boot.bin of=$(Img) bs=512 count=1 conv=notrunc
	sudo mount $(Img) $(ImgMountPoint) -o loop
	sudo cp -fv $(tb)/loader.bin $(ImgMountPoint)
	sudo cp -fv $(AOSKernel) $(ImgMountPoint)
	sudo umount $(ImgMountPoint)

# 打开 bochs 进行系统的运行和调试
debug: $(Img)
	bochs -q

# 运行AOS：打印提示信息
run: $(Img)
	@qemu-system-i386 -m 256 -drive file=$(Img),if=floppy
	@echo "你还可以使用Vbox等虚拟机挂载.img软盘，即可开始运行！"

# 更新映像并运行
uprun: $(Img) image run

# 更新映像并运行
updebug: $(Img) image debug

# 清理所有的中间编译文件
clean:
	-rm -rf $(Objs)
	@echo "中间文件清理完毕"

# 完全清理：清理所有的中间编译文件以及生成的目标文件（二进制文件）
realclean: clean
	-rm -f $(AOSBoot) $(AOSKernel) $(Objs)

all: realclean everything
# ======================================================================================================================


# ======================================================================================================================
# ----------------------------------------------------------------------------------------------------------------------
#   目标文件生成规则
# ----------------------------------------------------------------------------------------------------------------------
# 软盘镜像文件不存在，应该怎么生成？  生成规则
$(Img):
	dd if=/dev/zero of=$(Img) bs=512 count=2880

# 引导程序的生成规则
$(tb)/boot.bin: src/boot/include/fat12hdr.inc
$(tb)/boot.bin: src/boot/boot.asm
	$(ASM) $(ASMFlagsOfBoot) -o $@ $<

# 加载程序Loader
$(tb)/loader.bin: src/boot/include/fat12hdr.inc
$(tb)/loader.bin: src/boot/include/load.inc
$(tb)/loader.bin: src/boot/include/pm.inc
$(tb)/loader.bin: src/boot/loader.asm
	$(ASM) $(ASMFlagsOfBoot) -o $@ $<

# 内核
$(AOSKernel): $(Objs)
	$(LD) $(LDFlags) -o $(AOSKernel) $^

$(LIB) : $(LibObjs)
	$(AR) $(ARFLAGS) $@ $^
# ======================================================================================================================


# ======================================================================================================================
# ----------------------------------------------------------------------------------------------------------------------
#   中间Obj文件生成规则
# ----------------------------------------------------------------------------------------------------------------------
#   内核
$(tk)/kernel.o: $(sk)/kernel.asm
	$(ASM) $(ASMFlagsOfKernel) -o $@ $<

$(tk)/kernel_i386lib.o: $(sk)/include/core/kernel_i386lib.asm
	$(ASM) $(ASMFlagsOfKernel) -o $@ $<

$(tk)/main.o: $(sk)/main.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/init_c.o: $(sk)/init_c.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/protect.o: $(sk)/protect.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/exception.o: $(sk)/exception.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/panic.o: $(sk)/panic.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/i8259.o: $(sk)/i8259.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/clock.o: $(sk)/clock/clock.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/process.o: $(sk)/process.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/ipc.o: $(sk)/ipc.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/dump.o: $(sk)/dump.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/misc.o: $(sk)/misc.c
	$(CC) $(CFlags) -o $@ $<
# ----------------------------------------------------------------------------------------------------------------------
#   tty
# ----------------------------------------------------------------------------------------------------------------------
$(tk)/keyboard.o: $(sk)/tty/keyboard.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/tty.o: $(sk)/tty/tty.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/console.o: $(sk)/tty/console.c
	$(CC) $(CFlags) -o $@ $<
# ----------------------------------------------------------------------------------------------------------------------
#   idle
# ----------------------------------------------------------------------------------------------------------------------
$(tk)/idle.o: $(sk)/idle/idle.c
	$(CC) $(CFlags) -o $@ $<
# ----------------------------------------------------------------------------------------------------------------------
#   mm
# ----------------------------------------------------------------------------------------------------------------------
$(tk)/mm.o: $(sk)/mm/mm.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/alloc.o: $(sk)/mm/alloc.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/fork.o: $(sk)/mm/fork.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/mem_map.o: $(sk)/mm/mem_map.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/exit.o: $(sk)/mm/exit.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/exec.o: $(sk)/mm/exec.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/wait.o: $(sk)/mm/wait.c
	$(CC) $(CFlags) -o $@ $<
# ----------------------------------------------------------------------------------------------------------------------
#   hd
# ----------------------------------------------------------------------------------------------------------------------
$(tk)/at_wini.o: $(sk)/hd/at_wini.c
	$(CC) $(CFlags) -o $@ $<
# ----------------------------------------------------------------------------------------------------------------------
#   fs
# ----------------------------------------------------------------------------------------------------------------------
$(tk)/fs.o: $(sk)/fs/fs.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/open.o: $(sk)/fs/open.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/fs_misc.o: $(sk)/fs/fs_misc.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/read_write.o: $(sk)/fs/read_write.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/link.o: $(sk)/fs/link.c
	$(CC) $(CFlags) -o $@ $<
# ----------------------------------------------------------------------------------------------------------------------
#   origin
# ----------------------------------------------------------------------------------------------------------------------
$(tk)/origin.o: $(sk)/origin/origin.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/hash.o: $(sk)/origin/hash.c
	$(CC) $(CFlags) -o $@ $<
# ----------------------------------------------------------------------------------------------------------------------
#   origin/cmd
# ----------------------------------------------------------------------------------------------------------------------
$(tk)/pwd.o: $(sk)/origin/cmd/pwd.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/date.o: $(sk)/origin/cmd/date.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/echo.o: $(sk)/origin/cmd/echo.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/cat.o: $(sk)/origin/cmd/cat.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/touch.o: $(sk)/origin/cmd/touch.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/vi.o: $(sk)/origin/cmd/vi.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/clear.o: $(sk)/origin/cmd/clear.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/rm.o: $(sk)/origin/cmd/rm.c
	$(CC) $(CFlags) -o $@ $<
# ----------------------------------------------------------------------------------------------------------------------
#   test
# ----------------------------------------------------------------------------------------------------------------------
$(tk)/fs_test.o: $(sk)/test/fs_test.c
	$(CC) $(CFlags) -o $@ $<

$(tk)/tty_test.o: $(sk)/test/tty_test.c
	$(CC) $(CFlags) -o $@ $<
# ----------------------------------------------------------------------------------------------------------------------
#   库文件
# ----------------------------------------------------------------------------------------------------------------------
#   ansi
$(tl)/ansi/string.o: $(lansi)/string.asm
	$(ASM) $(ASMFlagsOfKernel) -o $@ $<

$(tl)/ansi/memcmp.o: $(lansi)/memcmp.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/ansi/cstring.o: $(lansi)/cstring.c
	$(CC) $(CFlags) -o $@ $<
# ----------------------------------------------------------------------------------------------------------------------
#   stdio
# ----------------------------------------------------------------------------------------------------------------------
$(tl)/stdio/printf.o: $(lstdio)/printf.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/stdio/vsprintf.o: $(lstdio)/vsprintf.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/stdio/close.o: $(lstdio)/close.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/stdio/open.o: $(lstdio)/open.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/stdio/read.o: $(lstdio)/read.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/stdio/write.o: $(lstdio)/write.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/stdio/stat.o: $(lstdio)/stat.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/stdio/exit.o: $(lstdio)/exit.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/stdio/exec.o: $(lstdio)/exec.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/stdio/fork.o: $(lstdio)/fork.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/stdio/wait.o: $(lstdio)/wait.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/stdio/unlink.o: $(lstdio)/unlink.c
	$(CC) $(CFlags) -o $@ $<
# ----------------------------------------------------------------------------------------------------------------------
#   i386/ipc
# ----------------------------------------------------------------------------------------------------------------------
$(tl)/i386/ipc/ipc.o: $(li386)/ipc/ipc.asm
	$(ASM) $(ASMFlagsOfKernel) -o $@ $<
# ----------------------------------------------------------------------------------------------------------------------
#   stdlib
# ----------------------------------------------------------------------------------------------------------------------
$(tl)/stdlib/get_time.o: $(lstdlib)/get_time.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/stdlib/get_pid.o: $(lstdlib)/get_pid.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/stdlib/get_ppid.o: $(lstdlib)/get_ppid.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/stdlib/change_console.o: $(lstdlib)/change_console.c
	$(CC) $(CFlags) -o $@ $<

$(tl)/stdlib/clean_console.o: $(lstdlib)/clean_console.c
	$(CC) $(CFlags) -o $@ $<
# ======================================================================================================================