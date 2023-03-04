#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int main(int argc, char **argv)
{
	pid_t	pid;
	printf("Parent process PID[%d] start running ..\n", getpid());

	pid = fork();

	if(pid < 0)
	{
		printf("fork() create child proess failure: %s\n",strerror(errno));
		return -1;
	}
	else if(pid == 0)
	{
		printf("Child proess PID[%d] start runing, my parent PID is [%d]\n", getpid(), getppid());
		return 0;
	}
	else
	{
		printf("Child proess create sucessfully, now this is parent proess , my PID is [%d], my child proess PID is [%d]\n", getpid(), pid);
		return 0;
	}
}
