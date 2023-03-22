#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/socket.domain"

int main(int argc, char** argv)
{
    int rv = -1;
    int listen_fd, client_fd = -1;
    struct sockaddr_un serv_addr;
    struct sockaddr_un client_addr;
    socklen_t cliaddr_len;
    char buf[1024];
    // 中文注释

    // 创建socket,目标AF_LOCAL,即UNIX域socket
    listen_fd = socket(AF_LOCAL, SOCK_STREAM, 0);   
    if(listen_fd < 0)
    {
        printf("Create socket failed: %s\n", strerror(errno));
        return -1;
    }
    printf("socket create fd[%d]\n", listen_fd);

    if( !access(SOCKET_PATH, F_OK) )
    {
        unlink(SOCKET_PATH);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, SOCKET_PATH, sizeof(serv_addr.sun_path)-1);

    if( bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0 )
    {
        printf("Bind socket failed: %s\n", strerror(errno));
        unlink(SOCKET_PATH);
        return -2;
    }
    printf("create socket failure: %s\n", strerror(errno));

    listen(listen_fd, 5); // 最大链接数5

    while(1)
    {
        printf("\nStatr waiting and accept new client cconnect...\n", listen_fd);
        client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &cliaddr_len);
        if(client_fd < 0)
        {
            printf("Accept new client connect failed: %s\n", strerror(errno));
            return -2;
        }

        memset(buf, 0, sizeof(buf));
        if( rv = read(  ))
    }
}