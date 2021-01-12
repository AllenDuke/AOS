cmake_minimum_required(VERSION 3.15)
project(AOS)
# 设置C语言所使用的标准
set(CMAKE_C_STANDARD 99)

# 导入头文件目录，特别是针对于操作系统工程
include_directories(HEAD "src/kernel/include")

# 将所有的可执行文件添加进跟踪列表
add_executable(
        AOS

        #        src/kernel/include/core/config.h
        #        src/kernel/include/core/constant.h
        #        src/kernel/include/core/struct_type.h
        #        src/kernel/include/types.h
        #
        #        src/kernel/include/ansi.h
        #        src/kernel/include/core/errno.h
        #        src/kernel/include/core/stddef.h
        #        src/kernel/include/unistd.h
        #
        #        src/kernel/include/assert.h
        #        src/kernel/include/core/global.h
        #        src/kernel/include/core/protect.h
        #        src/kernel/include/core/prototype.h
        #        src/kernel/include/core/process.h
        src/kernel/main.c
        src/kernel/protect.c
        src/kernel/init_c.c

        src/kernel/lib/ansi/memcmp.c
        src/kernel/lib/ansi/cstring.c


        src/kernel/lib/stdio/printf.c
        src/kernel/ipc.c
        src/kernel/clock/clock.c
        src/kernel/process.c
        src/kernel/dump.c
        src/kernel/tty/keyboard.c src/kernel/tty/tty.c src/kernel/idle/idle.c src/kernel/mm/mm.c src/kernel/mm/alloc.c src/kernel/mm/fork.c src/kernel/mm/mem_map.c src/kernel/mm/exit.c src/kernel/mm/wait.c src/kernel/hd/at_wini.c src/kernel/fs/fs.c src/kernel/fs/open.c src/kernel/lib/stdio/close.c src/kernel/lib/stdio/open.c src/kernel/lib/stdio/read.c src/kernel/lib/stdio/write.c src/kernel/test/fs_test.c src/kernel/test/tty_test.c src/kernel/mm/exec.c src/kernel/lib/stdio/stat.c src/kernel/lib/stdio/vsprintf.c src/kernel/lib/stdio/unlink.c src/kernel/lib/stdio/wait.c src/kernel/origin/origin.c)