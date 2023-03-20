#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <libgen.h>
#include <poll.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

static inline void print_usage(char *progname);
int socket_server_init(char *listen_ip, int listen_port);

int main(int argc, char **argv)
{
    int listenfd, connfd;
    int serv_port = 0;
    int daemon_run = 0;
    char *progname = NULL;
    int opt;
    int rv = -1;
    int i, j;
    int found;
    int maxfd = 0;
    char buf[1024];
    struct pollfd fds_array[1024];

    struct option long_options[] =
        {
            {"daemon", no_argument, NULL, 'd'},
            {"prot", required_argument, NULL, 'p'},
            {"help", no_argument, NULL, 'h'},
            {NULL, 0, NULL, 0}};

    progname = basename(argv[0]);

    // 用于选项
    while ((opt = getopt_long(argc, argv, "bp:h", long_options, NULL)) != -1)
    {
        switch (opt)
        {
        case 'd':
            daemon_run = 1;
            break;
        case 'p':
            serv_port = atoi(optarg);
            break;
        case 'h':
            print_usage(progname);
            return EXIT_SUCCESS;
        default:
            break;
        }
    }

    if (!serv_port)
    {
        print_usage(progname);
        return -1;
    }

    if ((listenfd = socket_server_init(NULL, serv_port)) < 0)
    {
        printf("ERROR: %s server listen on prot %d failure\n", argv[0], serv_port);
        return -2;
    }
    printf("%s server start to lister on port %d\n", argv[0], serv_port);

    if (daemon_run)
    {
        // 创建daemon进程启动就能在后台一直运行，不会随着terminal的退出而结束
        // 将进程的工作目录修改为"/"根目录，并将标准输入、标准输出和标准出错关闭
        daemon(0, 0);
    }

    for (i = 0; i < ARRAY_SIZE(fds_array); i++)
    {
        fds_array[i].fd = -1;
    }
    fds_array[0].fd = listenfd;
    fds_array[0].events = POLLIN;

    maxfd = 0;

    while (1)
    {
        // 程序将在此堵塞
        // 超时设置为-1，则将永不超时
        rv = poll(fds_array, maxfd+1, -1);
        if (rv < 0)
        {
            printf("select failure: %s\n", strerror(errno));
            break;
        }
        else if (rv == 0)
        {
            printf("select get timeout\n");
            continue;
        }

        // 监听套接字获取事件意味着新客户端现在开始连接
        if (fds_array[0].revents & POLLIN)
        {
            if ((connfd = accept(listenfd, (struct sockaddr *)NULL, NULL)) < 0)
            {
                printf("accept new client failure:%s\n", strerror(errno));
                continue;
            }

            found = 0;
            for (i = 1; i < ARRAY_SIZE(fds_array); i++)
            {
                if (fds_array[i].fd < 0)
                {
                    printf("accept new client[%d] adn add it into array\n", connfd);
                    fds_array[i].fd = connfd;
                    fds_array[i].events = POLLIN;
                    found = 1;
                    break;
                }
            }

            if (!found)
            {
                printf("accept new client[%d] but full, so refuse it\n", connfd);
                close(connfd);
            }

            maxfd = i>maxfd ? i : maxfd;
            if(--rv <= 0)
                continue;
        }
        else // 数据来自已连接的客户端
        {
            for (i = 1; i < ARRAY_SIZE(fds_array); i++)
            {
                if(fds_array[i].fd < 0)
                    continue;

                if( (rv = read(fds_array[i].fd, buf, sizeof(buf))) <= 0)
                {
                    printf("Socket[%d] read failure or get disconnect.\n", fds_array[i].fd);
                    close(fds_array[i].fd);
                    fds_array[i].fd = -1;
                }
                else
                {
                    printf("Socket[%d] read %d bytes data: %s\n", fds_array[i].fd, rv, buf);

                    // 将数据原样返回给客户端
                    if(write(fds_array[i].fd, buf, rv) < 0)
                    {
                        printf("Socket[%d] write failure:%s\n",fds_array[i].fd, strerror(errno));
                        close(fds_array[i].fd);
                        fds_array[i].fd = -1;
                    }
                    memset(buf, 0, sizeof(buf));
                }
            }
        }
    }
}

int socket_server_init(char *listen_ip, int listen_port)
{
    struct sockaddr_in servaddr;
    int rv = 0;
    int on = 1;
    int listenfd;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Use socket to create TCP socket base in IPV4 failure:%s\n", strerror(errno));
        return -2;
    }

    // 设置套接字端口可重用，修复套接字服务器重启时的“地址已在使用”错误
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(listen_port);

    if (!listen_ip)
    {
        // 监听IPV4所有网卡ip
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else // 监听特别IP
    {
        if (inet_pton(AF_INET, listen_ip, &servaddr.sin_addr) <= 0)
        {
            printf("inet_pton() set listen IP address failure.\n");
            rv = -2;
            close(listenfd);
            return rv;
        }
    }

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("Use bind() to bind TCP socket failure: %s\n", strerror(errno));
        rv = -3;
        close(listenfd);
        return rv;
    }

    if (listen(listenfd, 13) < 0)
    {
        printf("Use bind() to bind the TCP socket failure:%s\n", strerror(errno));
        rv = -4;
        close(listenfd);
        return rv;
    }

    rv = listenfd;
    return rv;
}

static inline void print_usage(char *progname)
{
    printf("Usage:%s [OPTION]...\n", progname);
    printf("%s is a socket server program, which used to verify client and echo back string from it\n", progname);
    printf("-d(--daemon): set program running on background \n");
    printf("-p(--port): sepcify server port \n");
	printf("-h(--help): printf help information \n");

    return;
}