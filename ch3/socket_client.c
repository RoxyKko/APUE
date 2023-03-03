#include <stdio.h>
#include <sys/types.h>	// for socket
#include <sys/socket.h> // for socket
#include <string.h>		// fpr strerror and memset
#include <errno.h>		// for errno
#include <netinet/in.h> // for sockaddr_in
#include <arpa/inet.h>	// for inet_aton
#include <fcntl.h>		// for open
#include <unistd.h>		// for read/write/close
#include <getopt.h>		// for getopt_long
#include <stdlib.h>		// for atoi

#define MSG_STR "Hello Wrold!"

void print_usage(char *progname);

int main(int argc, char **argv)
{
	int socket_fd = -1;
	int rv = -1;
	struct sockaddr_in servaddr;
	char *servip = "192.168.0.200";
	int port = 12345;
	char buf[1024];
	// 用在getopt_long中
	int opt = -1;
	const char *optstring = "i:p:h";
	struct option opts[] =
	{
		{"help", no_argument, NULL, 'h'},
		{"ipaddr", required_argument, NULL, 'i'},
		{"prot", required_argument, NULL, 'p'},
		{0, 0, 0, 0}
	};

	while ((opt = getopt_long(argc, argv, optstring, opts, NULL)) != -1)
	{
		switch (opt)
		{
			case 'i' :
				servip = optarg;
				break;
			case 'p' :
				port = atoi(optarg);
				break;
			case 'h' :
				print_usage(argv[0]);
				return 0;
		}
	}

	// 创建socket并获得socket描述符
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	// 创建socket失败
	if(socket_fd < 0)
	{
		printf("Create socket failure: %s\n", strerror(errno));
		return -1;
	}
	// 创建socket成功
	printf("Create socket[%d] successfully\n",socket_fd);

	// 初始化结构体，将空余的8位字节填充为0
	// 设置参数，connect连接服务器
	memset(&servaddr, 0, sizeof(servaddr));	
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	inet_aton(servip, &servaddr.sin_addr);
	rv = connect(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	// 连接服务器失败
	if(rv < 0)
	{
		printf("Connect to server[%s:%d] failure : %s\n",
				servip, port, strerror(errno));
		return -2;
	}
	// 连接服务器成功,打印服务器IP地址和端口号
	printf("Connect to server[%s:%d] successfully!\n", servip, port);

	// 向服务器发送一次消息，Hello World!
	rv = write(socket_fd, MSG_STR, strlen(MSG_STR));
	if(rv < 0)
	{
		printf("Write to server by sockfd[%d] failure : %s\n",
				socket_fd, strerror(errno));
		return -3;
	}
	while(1)
	{
		// 每次进入循环清空缓冲区
		// 持续读取服务器发送的数据存入缓冲区
		memset(buf, 0, sizeof(buf));
		rv = read(socket_fd, buf, sizeof(buf));
		if(rv < 0)
		{
			printf("Read data form server by sockfd[%d] failure : %s \n",
					socket_fd, strerror(errno));
			break;
		}
		else if(rv == 0)
		{
			printf("Socket[%d] get disconnect!\n", socket_fd);
			break;
		}
		else if(rv > 0)
		{
			// 读取成功，本机（客户端）打印读取到的数据
			// 读取到的字节数就是read()的返回值
			printf("Read %d bytes data form Server: %s\n",
				rv, buf);
		}

		// 读取成功后进入此，向服务器发送接收到的数据
		rv = write(socket_fd, buf, sizeof(buf));
		if(rv < 0)
		{
			printf("Write to server by sockfd[%d] failure : %s\n",
					socket_fd, strerror(errno));
			break;
		}
	}

	close(socket_fd);
	return 0;
}


void print_usage(char *progname)
{
	printf("%s usage: \n", progname);
	printf("-i(--ipaddr): sepcify server IP address\n");
	printf("-p(--port): sepcify server port \n");
	printf("-h(--help): printf help information \n");

	return ;
}
