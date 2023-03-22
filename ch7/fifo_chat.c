#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <libgen.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/select.h>

#define FIFO_FILE1 "/tmp/fifo_chat1"
#define FIFO_FILE2 "/tmp/fifo_chat2"

int g_stop = 0;

void sig_pipe(int signum)
{
    if (signum == SIGPIPE)
    {
        printf("SIGPIPE signal received, exit\n");
        g_stop = 1;
    }
}

int main(int argc, char**argv)
{
    int fdr_fifo;
    int fdw_fifo;
    int rv;
    fd_set read_set;
    char buf[1024];
    int mode = 0;

    if( argc != 2)
    {
        printf("Usage: %s [0|1]\n", basename(argv[0]));
        printf("This chat program need run twice, 1st time run with [0] and 2nd time with [1] \n" );
        return -1;
    }

    mode = atoi(argv[1]);

    if (access(FIFO_FILE1, F_OK))
    {
        printf("FIFO file \"%s\" not exist and create it now\n", FIFO_FILE1);
        if (mkfifo(FIFO_FILE1, 0666) < 0)
        {
            printf("Create FIFO file \"%s\" failed: %s\n", FIFO_FILE1, strerror(errno));
            return -2;
        }
    }

    if(access(FIFO_FILE2, F_OK))
    {
        printf("FIFO file \"%s\" not exist and create it now\n", FIFO_FILE2);
        if (mkfifo(FIFO_FILE2, 0666) < 0)
        {
            printf("Create FIFO file \"%s\" failed: %s\n", FIFO_FILE2, strerror(errno));
            return -3;
        }
    }

    signal(SIGPIPE, sig_pipe);

    if( mode == 0)  // 0 progname
    {
        /* 这里以只读模式打开命名管道FIFO_FILE1的读端，默认是阻塞模式；如果命名管道的写端被不打开则open()将会一直阻塞，
        所以另外一个进程必须首先以写模式打开该文件FIFO_FILE1，否则会出现死锁 */
        printf("start open '%s' for read and it will block until other process open it for write...\n ", FIFO_FILE1);

        if( (fdr_fifo = open(FIFO_FILE1, O_RDONLY)) < 0)
        {
            printf("Open FIFO file \"%s\" for read failed: %s\n", FIFO_FILE1, strerror(errno));
            return -4;
        }

        printf("start open %s for write...\n", FIFO_FILE2);
        if( (fdw_fifo=open(FIFO_FILE2, O_WRONLY)) < 0)
        {
            printf("Open FIFO file \"%s\" for write failed: %s\n", FIFO_FILE2, strerror(errno));
            return -5;
        }
    }
    else    // 1 progname
    {
        printf("start open '%s' for write and it will block until other process open it for read...\n ", FIFO_FILE1);

        if( (fdw_fifo = open(FIFO_FILE1, O_WRONLY)) < 0)
        {
            printf("Open FIFO file \"%s\" for write failed: %s\n", FIFO_FILE1, strerror(errno));
            return -4;
        }

        printf("start open %s for read...\n", FIFO_FILE2);
        if( (fdr_fifo=open(FIFO_FILE2, O_RDONLY)) < 0)
        {
            printf("Open FIFO file \"%s\" for read failed: %s\n", FIFO_FILE2, strerror(errno));
            return -5;
        }
    }

    printf("start chating with another program now, please input message now\n");
    while (!g_stop)
    {
        FD_ZERO(&read_set);
        FD_SET(STDIN_FILENO, &read_set);
        FD_SET(fdr_fifo, &read_set);

        /* select多路复用监听标准输入和作为输入的命名管道读端 */
        rv = select(fdr_fifo+1, &read_set, NULL, NULL, NULL);
        if (rv < 0)
        {
            printf("select() failed: %s\n", strerror(errno));
            break;
        }

        /* 如果是作为输入的命名管道上有数据到来则从管道上读入数据并打印到标注输出上 */
        if( FD_ISSET(fdr_fifo, &read_set))
        {
            memset(buf, 0, sizeof(buf));
            rv = read(fdr_fifo, buf, sizeof(buf));
            if (rv < 0)
            {
                printf("read() failed: %s\n", strerror(errno));
                break;
            }
            else if (rv == 0)
            {
                printf("read() return 0, peer close the FIFO\n");
                break;
            }
            else
            {
                printf("<-->: %s\n", buf);
            }
        }

        /* 如果标准输入上有数据到来，则从标准输入上读入数据后，将数据写入到作为输出的命名管道上给另外一个进程 */
        if( FD_ISSET(STDIN_FILENO, &read_set))
        {
            memset(buf, 0, sizeof(buf));
            fgets(buf, sizeof(buf), stdin);
            rv = write(fdw_fifo, buf, strlen(buf));
            if (rv < 0)
            {
                printf("write() failed: %s\n", strerror(errno));
                break;
            }
        }
    }
    

}