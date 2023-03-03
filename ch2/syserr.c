#include <stdio.h>          // for perror() and printf()
#include <errno.h>          //errno系统变量
#include <string.h>         //streerror()函数
#include <sys/types.h>      //open()函数
#include <unistd.h>         //close()函数
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
    char *file_name = "/test.txt";
    int fd = -1;

    fd = open(file_name, O_RDONLY, 066);
    if (fd < 0)
    {
        // perror只能打印固定的字符串，并且只能打印到标准输出
        perror("Open file failure");
        // strerror中间能插入文件名等，能打印到log日志中去
        printf("Open file %s failure: %s\n", file_name, strerror(errno));
        return 0;
    }

    close(fd);
    return 0;
}