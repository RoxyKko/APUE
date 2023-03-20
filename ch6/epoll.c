#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/resource.h>

#define MAX_EVENTS	512
#define ARRAY_SIZE(x)	(sizeof(x)/sizeof(x[0]))

static inline void print_usage(char *porgname);
int socket_server_init(char *listen_ip, int listen_port);
void set_socket_rlimit(void);

int main(int argc, char **argv)
{
	int	listenfd, connfd;
	int	serv_port = 0;
	int	daemon_run = 0;
	char	*progname = NULL;
	int	opt;
	int	rv;
	int	i, j;
	int	found;
	char	buf[1024];

	int	epollfd;
	struct epoll_event	event;
	struct epoll_event	event_arrat[MAX_EVENTS];
	int	events;

	struct option	long_option[] = 
	{
		{"daemon", no_argument, NULL, 'b'},
		{"port", required_argument, NULL, 'p'},
		{"help", no_argument, NULL, 'h'},
		{NULL, NULL, NULL, NULL}
	};

	progname = basename(argv[0]);

	// 命令行选项
	while( (opt = getopt_long(argc, argv, "bp:h", long_option, NULL)) != -1)
	{
		switch(opt)
		{
			case 'b':
				daemon_run = 1;
				break;
			case 'p':
				serv_port = atoi(optarg);
				break;
			case 'h':
				print_usage();
				return EXIT_SUCCESS;
			default:
				break;
		}
	}

	if (! serv_port)
	{
		print_usage();
		return -1;
	}

	set_socket_rlimit();
}

static inline void print_usage(char *progname)
{
	printf("Usage: %s [OPTION] ...\n", progname);

	printf(" %s is a socket server program, which used toverify client and echo back string from it \n", proname);
	prinft("\nMandatory arguments to long options are mandatory for short option too:\n");

	printf(" -b[daemon ] set program running on background\n");
	printf(" -p[port   ] Socket server port address\n");
	printf(" -h[help   ] Display this help information\n");

	printf("\nExample: %s -b -p 8900\n", progname);
	return 0;
}

int socker_server_init(char *listen_ip, int listen_port)
{
	struct sockaddr_in	servaddr;
	int			rv = 0;
	int			on = 1;
	int			listenfd;

	if ( (listenfd = socket(AF_INET, SOCK_STREAM, 0) ) < 0)
	{
		printf("Use socket() create a TCP socket failure: %s \n", strerror(errno));
		return -1;
	}

	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(listen_port);

	if( !listen_ip )
	{
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		if( inet_pton(AF_INET, listen_ip, &servaddr.sin_addr) <= 0)
		{
			printf("inet_pton() set listen IP address failure.\n");
			rv = -2;
			close(listenfd);
			return rv;
		}
	}

	if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		printf("Use bind() to bind the TCP socket failure: %s\n",strerror(errno));
		rv = -3;
		close(listenfd);
		return rv;
	}

	if (listen(listenfd, 64) < 0)
	{
		printf("Use bind() to bind the TCP socket failure: %s\n",strerror(errno));
		rv = -3;
		close(listenfd);
		return rv;
	}

	rv = listenfd;

	return rv;
}

void set_socket_rlimit()
{
	struct rlimit limit = {0};
	getrlimit(RLIMIT_NOFILE, &limit);
	limit.rlim_cur = limit.rlim_max;
	setrlimit(RLIMIT_NOFILE, &limit);

	printf("set socket open fd max count to %d\n", limit.rlim_max);
}

