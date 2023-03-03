#include <stdio.h>
#include <errno.h>
// write 和 read close都在 unistd.h下
#include <unistd.h>
// memset的头文件
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFSIZE 1024
#define MSG_STR "Hello World\n"

int main(int argc, char *argv[])
{
    // 文件句柄，正常应该为非负的整数
    int fd = -1;
    // 返回值return value
    int rv = -1;
    char buf[BUFSIZE];

    // open返回一个文件描述符保存在fd中
    fd = open("test.txt", O_RDWR|O_CREAT|O_TRUNC, 0666);
    if(fd < 0)
    {
        /* perror()函数可以打印系统调用出错的具体原因,其参数只能跟一个字符串提示符 */        
        perror("Open/Create file test.txt failure");
        return 0;
    }
    printf("Open file returned file descriptor [%d]\n", fd);
    // write参数 写到哪里 写什么 写的大小
    // 用strlen是因为MSG_STR是宏定义，宏定义是地址
    if( (rv=write(fd, MSG_STR, strlen(MSG_STR))) < 0 )
    {
        printf("Write %d bytes into failure: %s\n", rv, strerror(errno));
        // 疑问？：与其使用goto为什么不封装直接一个函数close和return呢
        // 解答： 一些简短的使用goto方便些
        goto cleanup;
    }

    /*
    程序中定义的buf是局部变量存放在栈中，而栈中的数据是随机数据，所以这里会显示两个乱码字符。
    所以在使用局部变量之前，我们都应该对其进行初始化。
    */ 
    // 初始化数组，在读数组前要先memset清空数组
    /******************!!!只要是栈中的数据都应该初始化!!!****************************/
    lseek(fd, 0, SEEK_SET);
    // close(fd);
    memset(buf, 0, sizeof(buf));
    // read的返回值是你实际读取了多少字节
    if( (rv=read(fd, buf, sizeof(buf))) < 0)
    {
        printf("Read data from file failure : %s\n", strerror(errno));
        goto cleanup;
    }

    // 疑问？：rv的值一直是0，但test.txt里是有成功写入的，为什么？
    // 解决：在Hello World写入后，文件偏移量（类似光标）位于World的后方，读或写入时从光标所在位置开始读写
    //      所以是读不到值的，解决方法是写入后先close文件再读或者使用leeek(fd, 0, SEEK_SET)，两种方法都将文件偏移量设置到文件开始的第一个字节上
    printf("Read %d bytes data from file: %s\n", rv, buf);

    cleanup:
        close(fd);
        // 非main函数return只会导致函数退出
        // 而main函数的return会调用exit()，导致整个进程中止
        // 同理，其他非main函数调用exit()一样会导致整个进程中止
        return 0;
}
