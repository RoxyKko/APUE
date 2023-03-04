#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

int g_var = 6;
char g_buf[] = "A string write to stdout.\n";
int main (int argc, char** argv)
{
	int var = 88;
	pid_t pid;
	int rv = -1;
	rv = (write(STDOUT_FILENO, g_buf, sizeof(g_buf)-1));	
	if(rv < 0)
	{
		printf("Write string to stdout error : %s\n", 
				strerror(errno));
		return -1;
	}

	printf("Befor fork\n");

	if( (pid = fork()) < 0)
	{
		printf("fork() creat failure : %s\n",
				strerror(errno));
		return -2;
	}
	else if ( pid == 0)
	{
		printf("This is child proess PID[%d] running...\n",
				getpid());
		g_var++;
		var++;
	}
	else
	{
		printf("This is parent proess PID[%d] waiting...\n",
				getpid());
		sleep(1);	// 这里只是暂时为了等待子进程先完成
				// 实际子进程和父进程的先后顺序后面会有
	}

	printf("PID = %d, g_var = %d, var = %d\n", getpid(), g_var, var);
	return 0;
}
