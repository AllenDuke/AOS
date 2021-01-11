# ===============================================
ENTRYPOINT 		= 0x100000
# ===============================================
# 所有的变量

# 头文件目录
i = ../include
h = $i/core


# 内核的编译链接中间目录
t = ../../../target
tb = $t/boot
tk = $t/kernel
l = ../lib

# 所有实用程序目录
alldir:         = cmd

# 硬盘镜像
HD              = ../../../500MHD.img

# 所使用的编译程序，参数
ASM 			= nasm
DASM 			= objdump
CC 				= gcc
LD				= ld
ASMFLAGS	    = -f elf
CFLAGS			= -I$i -O2 -c -fno-builtin -fno-stack-protector -D_AOS -D_POSIX_SOURCE -Wall
LDFLAGS			= -Ttext $(ENTRYPOINT)
DASMFLAGS		= -D

# aos标准库
LIB             = $(l)/aos_lib.a

# 依赖关系

# ===============================================
# 生成的所有命令行程序
CMD          = pwd echo

# ===============================================
# 默认选项，提示如何编译
nop:
	@echo "请先make all，然后make install..."
# 编译全部
all: clean everything
	@echo "已经编译所有的命令行实用程序..."

# 声明所有的伪命令，将不会被识别为文件
.PHONY : everything final clean realclean disasm all install

# ===============================================
# 所有的伪命令
everything: $(CMD)

install: all
	cp $(tb)/hd_loader.bin ./ -v
	mv hd_loader.bin hdldr.bin
	cp $(tk)/kernel.bin ./ -v
	tar vcf install.tar kernel.bin $(CMD) hdldr.bin
	dd if=install.tar of=$(HD) seek=`echo "obase=10;ibase=16;(\`egrep -e '^ROOT_BASE' ../../boot/include/load.inc | sed -e 's/.*0x//g'\`+\`egrep -e '#define[[:space:]]*INSTALL_START_SECT' $h/config.h | sed -e 's/.*0x//g'\`)*200" | bc` bs=1 count=`ls -l install.tar | awk -F " " '{print $$5}'` conv=notrunc

final: all clean

clean:
	-rm -rf cmd/*.o

start.o : start.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

echo.o: cmd/echo.c $(i)/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

echo : echo.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

pwd.o: cmd/pwd.c $(i)/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

pwd : pwd.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?