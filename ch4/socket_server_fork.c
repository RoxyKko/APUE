#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <ctype.h>

void print_usage(char *progname);
#define MAX_CLIENT 5


int main(int argc, char **argv)
{
    int socket_fd, client_fd = -1;
    int rv = -1;
    struct sockaddr_in servaddr;
    struct sockaddr_in clientaddr;
    socklen_t cliaddr_len;
    int port = 0;

    int childpid = -1;
    int on = 1;
    pid_t pid = -1;

    // 用在getopt_long中
    int opt = -1;
    const char *optstring = "p:h";
    struct option opts[] =
        {
            {"help", no_argument, NULL, 'h'},
            {"prot", required_argument, NULL, 'p'},
            {0, 0, 0, 0}};

    while ((opt = getopt_long(argc, argv, optstring, opts, NULL)) != -1)
    {
        switch (opt)
        {
        case 'p':
            port = atoi(optarg);
            break;
        case 'h':
            print_usage(argv[0]);
            return 0;
        }
    }

    if (!port)
    {
        print_usage(argv[0]);
        return 0;
    }

    // 创建socket并获得socket描述符
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    // 创建socket失败
    if (socket_fd < 0)
    {
        printf("Create socket failure: %s\n", strerror(errno));
        return -1;
    }
    // 创建socket成功
    printf("Create socket[%d] successfully\n", socket_fd);

    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    // 初始化结构体，将空余的8位字节填充为0
    // 设置参数，connect连接服务器
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    // 监听所有网卡的所有IP地址
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    rv = bind(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    // bind链接到port失败
    if (rv < 0)
    {
        printf("Socket[%d] bind on port [%d] failure : %s\n",
               socket_fd, port, strerror(errno));
        return -2;
    }
    // bind链接到port成功
    printf("Socket[%d] bind on port [%d] successfully!\n", socket_fd, port);

    // 开启监听
    listen(socket_fd, MAX_CLIENT);

    memset(&clientaddr, 0, sizeof(clientaddr));
    while (1)
    {
        printf("Waiting for client connect...\n");

        childpid = accept(socket_fd, (struct sockaddr *)&clientaddr, &cliaddr_len);
        if (childpid < 0)
        {
            printf("Accept client connect failure: %s\n", strerror(errno));
            continue;
        }

        printf("Accept new client [%s:%d] successfully\n",
               inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        pid = fork();

        if (pid < 0)
        {
            printf("fork() creat child proess failure: %s\n", strerror(errno));
            close(childpid);
            continue;
        }
        else if (pid > 0)
        {
            // printf("fork() creat child proess successfully\n");
            // 父进程关闭监听端口的进程，使能接受下一个客户端建立socket
            close(childpid);
            continue;
        }
        else
        {
            char buf[1024];
            int i = 0;
            client_fd = getpid();

            printf("Child proess[%d] start to commuicate with socket client...\n", client_fd);

            close(socket_fd); // 子进程关闭与监听端口的链接


            while (1)
            {
                memset(buf, 0, sizeof(buf));
                rv = read(childpid, buf, sizeof(buf));
                if (rv < 0)
                {
                    printf("Read data form client socket_fd[%d] failure: %s\n", strerror(errno));
                    close(childpid);
                    exit(0);
                }
                else if (rv == 0)
                {
                    printf("Socket[%d] get disconnect!\n", client_fd);
                    close(childpid);
                    exit(0);
                }
                else if (rv > 0)
                {
                    printf("Read %d bytes data from Client[%d]: %s\n", rv, client_fd, buf);
                }

                // 将字母从小写转换为大写
                for (i=0; i<rv; i++)
                {
                    buf[i] = toupper(buf[i]);
                }

                rv = write(childpid, buf, sizeof(buf));
                if(rv < 0)
                {
                    printf("Write to client by socket_fd[%d] failure : %s\n", client_fd, strerror(errno));
                    close(childpid);
                    exit(0);
                }
            } /* 子进程循环 */
        } /* 子进程启动 */
    }
}

void print_usage(char *progname)
{
    printf("%s usage: \n", progname);
    printf("-p(--port): sepcify server port \n");
    printf("-h(--help): printf help information \n");

    return;
}