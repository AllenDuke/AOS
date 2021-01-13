//
// Created by 杜科 on 2021/1/10.
//
/**
 * io相关，posix标准库相关
 */

#ifndef AOS_STDIO_H
#define AOS_STDIO_H

/* string */
#define STR_DEFAULT_LEN    1024

#define O_CREAT     1
#define O_RDWR      2
#define O_TRUNC     4

#define SEEK_SET    1
#define SEEK_CUR    2
#define SEEK_END    3

#define MAX_PATH    128

/**
 * @struct stat
 * @brief  File status, returned by syscall stat();
 */
struct stat {
    int st_dev;        /* major/minor device number */
    int st_ino;        /* i-node number */
    int st_mode;        /* file mode, protection bits, etc. */
    int st_rdev;        /* device ID (if special file) */
    int st_size;        /* file size */
};

int printf(const char *fmt, ...);

void exit(int status);
int open(const char *pathname, int flags);
int close(int fd);
int read(int fd, void *buf, int count);
int write(int fd, const void *buf, int count);
int stat(const char *path, struct stat *buf);
int unlink(const char *pathname);
int wait(int *status);
int fork();
int exec(const char * path);
int execl(const char *path, const char *arg, ...);
int execv(const char *path, char * argv[]);
#endif //AOS_STDIO_H
