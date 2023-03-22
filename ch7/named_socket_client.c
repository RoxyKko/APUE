#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/socket.domain"
#define MSG_STR "Hello, Unix Domain Socket Server"

int main(int argc, char** argv)
{
    int conn_fd = -1;
    int rv = -1;
    char buf[1024];
    struct sockaddr_un serv_addr;

    conn_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(conn_fd < 0)
    {
        printf("Create socket failed: %s\n", strerror(errno));
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, SOCKET_PATH, sizeof(serv_addr.sun_path)-1);

    if( connect(conn_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0 )
    {
        printf("Connect to unix domain socket server on \"%s\" failure: %s\n", SOCKET_PATH, strerror(errno));
        return -2;
    }
    printf("connect to unix domain socket server on \"%s\" success\n", SOCKET_PATH);

    if(write(conn_fd, MSG_STR, strlen(MSG_STR)) < 0)
    {
        printf("write data to unix domain socket server on \"%s\" failure: %s\n", SOCKET_PATH, strerror(errno));
        close(conn_fd);
    }

    memset(buf, 0, sizeof(buf));
    rv = read(conn_fd, buf, sizeof(buf));
    if(rv < 0)
    {
        printf("read data from unix domain socket server on \"%s\" failure: %s\n", SOCKET_PATH, strerror(errno));
        close(conn_fd);
    }
    else if(rv == 0)
    {
        printf("unix domain socket server on \"%s\" closed\n", SOCKET_PATH);
        close(conn_fd);
    }
    printf("read data from unix domain socket server on \"%s\": %s\n", SOCKET_PATH, buf);

    close(conn_fd);
    return 0;


}