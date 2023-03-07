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
#include <pthread.h>		// for pthread
#include <ctype.h>

typedef void *(THREAD_BODY) (void *thread_arg);

void *thread_worker(void *ctx);
int thread_start(pthread_t *thread_id, THREAD_BODY *thread_workbody, void *thread_arg);

#define MSG_STR "Hello Wrold! \n"
#define MAX_CLIENT 5

void print_usage(char *progname);

int main(int argc, char **argv)
{
	int socket_fd, client_fd = -1;
	int rv = -1;
	struct sockaddr_in servaddr;
	struct sockaddr_in clientaddr;
	socklen_t cliaddr_len;
	int port = 0;
	// 用在getopt_long中
	int opt = -1;
	const char *optstring = "p:h";

	// pthread
	pthread_t tid;
	struct option opts[] =
	{
		{"help", no_argument, NULL, 'h'},
		{"prot", required_argument, NULL, 'p'},
		{0, 0, 0, 0}
	};

	while ((opt = getopt_long(argc, argv, optstring, opts, NULL)) != -1)
	{
		switch (opt)
		{
			case 'p' :
				port = atoi(optarg);
				break;
			case 'h' :
				print_usage(argv[0]);
				return 0;
		}
	}

	if( !port )
	{
		print_usage(argv[0]);
		return 0;
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
	// 监听所有网卡的所有IP地址
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	rv = bind(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	// bind链接到port失败
	if(rv < 0)
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
	while(1)
	{
		printf("Waiting for client connect...\n");
		client_fd = accept(socket_fd, (struct sockaddr*)&clientaddr, &cliaddr_len);
		if(client_fd < 0)
		{
			printf("Accept client connect failure: %s\n", strerror(errno));
			break;
		}
		printf("Accept client connect from %s:%d\n",
				inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		/*************启动子线程*****************/
		thread_start(&tid, thread_worker, &client_fd);

	}

	close(socket_fd);
	return 0;
}


void print_usage(char *progname)
{
	printf("%s usage: \n", progname);
	printf("-p(--port): sepcify server port \n");
	printf("-h(--help): printf help information \n");

	return ;
}

void *thread_worker(void *ctx);

int thread_start(pthread_t *thread_id, THREAD_BODY *thread_workbody, void *thread_arg)
{
	int rv = -1;
	pthread_attr_t thread_attr;

	if( pthread_attr_init(&thread_attr) )
	{
		printf("pthread_attr_init() failure: %s\n", strerror(errno));
		pthread_attr_destroy(&thread_attr);
		return rv;
	}

	if( (pthread_attr_setstacksize(&thread_attr, 120*1024)) < 0)
	{
		printf("pthread_attr_setstacksize() failure: %s\n", strerror(errno));
		pthread_attr_destroy(&thread_attr);	
		return rv;
	}

	if( pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED) ) 
	{
		printf("pthread_attr_setdetachstate() failure: %s\n", strerror(errno));
		pthread_attr_destroy(&thread_attr);
		return rv;
	}

	// 创建子线程
	if( pthread_create(thread_id, &thread_attr, thread_workbody, thread_arg) )
	{
		printf("pthread_create() failure: %s\n", strerror(errno));
		pthread_attr_destroy(&thread_attr);
		return rv;

	}

	rv = 0;
}

void *thread_worker(void *ctx)
{
	int clifd;
	int rv;
	int i;
	char buf[1024];
	if( !ctx )
	{
		printf("Invalid input arguments in %s()\n", __FUNCTION__);
		pthread_exit(NULL);
	}
	clifd = *((int*)ctx);

	printf("Child thread start to commuicate with socket client...\n");

	while(1)
	{
		memset(buf, 0, sizeof(buf));
		rv = read(clifd, buf, sizeof(buf));
		if(rv < 0)
		{
			printf("Read data from client socket failure: %s\n" ,strerror(errno));
			close(clifd);
			pthread_exit(NULL);
		}
		else if(rv == 0)
		{
			printf("Socket[%d] get disconnected and thread will exit.\n", clifd);
			close(clifd);
			pthread_exit(NULL);
		}
		else if( rv > 0 )
		{
			printf("Read %d bytes data from Server: %s\n", rv, buf);
		}

		for(i=0; i<rv; i++)
		{
			buf[i] = toupper(buf[i]);
		}

		rv = write(clifd, buf, rv);
		if(rv < 0)
		{
			printf("Write to client by sockfd[%d] failure: %s\n", strerror(errno));
			close(clifd);
			pthread_exit(NULL);
		}
	}
}

