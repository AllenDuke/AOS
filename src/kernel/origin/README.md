# origin 起源进程 pid=0
这是第一个用户进程，不与内核编译链接到一起，作为一个单独的elf文件，存在于AOS.img中，内核启动后，由内核读取文件，加载到内存中。
它将放在13MB~15MB处。