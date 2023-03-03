#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>      //open()函数
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>     // malloc()

int main(int argc, char *argv[])
{
    // struct stat stbuff;
    struct stat *stbuff;
    // 指针一定要指向一个合法的内存地址，否则会出现段错误
    stbuff = malloc(sizeof(struct stat));

    // stat("stat.c", &stbuff);
    stat("stat.c", stbuff);
    // fstat 用法
    // int fd = -1;
    // fd = open("stat.c", O_RDONLY);
    // fstat(fd, stbuff);

    
    printf("File Mode: %hu Real Size: %luB\n", stbuff->st_mode, stbuff->st_size);
    return 0;

}