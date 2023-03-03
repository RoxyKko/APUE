#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    int fd = -1;
    fd = open("std.txt", O_RDWR|O_CREAT|O_TRUNC, 0666);
    if(fd < 0)
    {
        printf("Open file failure: %s\n", strerror(errno));
        return 0;
    }

    // close(0);
    // close(1);
    // close(2);

    // dup(fd);    //0 标准输入
    // dup(fd);    //1 标准输出
    // dup(fd);    //2 标准错误输出

    dup2(fd, STDIN_FILENO);     // 标准输入重定向到fd
    dup2(fd, STDOUT_FILENO);    // 标准输出重定向到fd
    dup2(fd, STDERR_FILENO);    // 标准错误输出重定向到fd

    printf("fd = %d\n", fd);

    close(fd);
    return 0;
}