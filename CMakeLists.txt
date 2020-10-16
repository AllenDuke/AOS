cmake_minimum_required(VERSION 3.15)
project(AOS)
# 设置C语言所使用的标准
set(CMAKE_C_STANDARD 99)

# 导入头文件目录，特别是针对于操作系统工程
include_directories(HEAD "include")

# 将所有的可执行文件添加进跟踪列表
add_executable(
        AOS
        include/ibm/cmos.h
        include/ibm/diskparm.h
        include/ibm/int86.h
        include/ibm/partition.h
        include/ibm/portio.h

        include/os/common.h
        include/os/config.h
        include/os/constant.h
        include/os/some_lib.h
        include/os/struct_type.h
        include/os/sys_call_num.h
        include/os/sys_lib.h
        include/sys/types.h

        include/sys/time.h

        include/ansi.h
        include/cstring.h
        include/errno.h
        include/lib.h
        include/limit.h
        include/stdarg.h
        include/stddef.h
        include/stdio.h
        include/stdlib.h
        include/terminal_io.h
        include/unistd.h

        src/kernel/assert.h
        src/kernel/clock.c
        src/kernel/const.h
        src/kernel/dump.c
        src/kernel/exception.c
        src/kernel/global.h
        src/kernel/i8259.c
        src/kernel/ipc_msg.c
        src/kernel/main.c
        src/kernel/misc.c
        src/kernel/process.c
        src/kernel/process.h
        src/kernel/protect.h
        src/kernel/protect.c
        src/kernel/prototype.h
        src/kernel/start.c
        src/kernel/table.c
        src/kernel/type.h

        src/lib/ansi/memcmp.c
        src/lib/ansi/cstring.c


        src/lib/stdio/sprintf.c
        src/lib/stdio/vsprintf.c
)