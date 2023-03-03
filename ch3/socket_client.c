#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#define MSG_STR "Hello Wrold!"

int main(int argv, char**argc)
{
	int socket_fd = -1;
	int rv = -1;
	struct sockaddr_in servaddr;
	char *servip = "192.168.0.200";
	int port = 12345;
	char buf[1024];

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd < 0)
	{
		printf("Create socket failure: %s\n", strerror(errno));
		return -1;
	}
	printf("Create socket[%d] successfully\n",socket_fd);

	memset(&servaddr, 0, sizeof(servaddr));	
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	inet_aton(servip, &servaddr.sin_addr);
	rv = connect(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	if(rv < 0)
	{
		printf("Connect to server[%s:%d] failure : %s\n",
				servip, port, strerror(errno));
		return -2;
	}

	printf("Connect to server[%s:%d] successfully!\n", servip, port);

	rv = write(socket_fd, MSG_STR, strlen(MSG_STR));
	if(rv < 0)
	{
		printf("Write to server by sockfd[%d] failure : %s\n",
				socket_fd, strerror(errno));
		return -3;
	}
	while(1)
	{
	
		memset(buf, 0, sizeof(buf));
		rv = read(socket_fd, buf, sizeof(buf));
		if(rv < 0)
		{
			printf("Read data form server by sockfd[%d] failure : %s \n",
					socket_fd, strerror(errno));
			return -4;
		}
		else if(rv == 0)
		{
			printf("Socket[%d] get disconnect!\n", socket_fd);
			return -5;
		}
		else if(rv > 0)
		{
			printf("Read %d bytes data form Server: %s\n",
				rv, buf);
		}

		rv = write(socket_fd, buf, sizeof(buf));
		if(rv < 0)
		{
			printf("Write to server by sockfd[%d] failure : %s\n",
					socket_fd, strerror(errno));
			return -3;
		}

			
	
	}

	close(socket_fd);
	return 0;
}

