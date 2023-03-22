#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MSG_STR "This message is from parent process: Hello child process"

int main(int argc, char** argv)
{
    int pipe_fd[2];
    int rv;
    int pid;
    char buf[512];
    int wstatus;

    if(pipe(pipe_fd) < 0)
    {
        printf("Create pipe failed: %s\n", strerror(errno));
        return -1;
    }

    if((pid = fork()) < 0)
    {
        printf("Create child process failed: %s\n", strerror(errno));
        return -2;
    }
    // Child process
    else if(pid == 0)  
    {
        // child process close write end
        close(pipe_fd[1]);

        memset(buf, 0, sizeof(buf));
        rv = read(pipe_fd[0], buf, sizeof(buf));
        if(rv < 0)
        {
            printf("Child process read pipe failed: %s\n", strerror(errno));
            return -3;
        }
        else if(rv == 0)
        {
            printf("Child process read pipe failed: pipe is closed\n");
            return -4;
        }
        else
        {
            printf("Child process read pipe %d bytes data : %s\n", rv, buf);
            return 0;
        }
    }

    // Parent process
    // parent process close read end
    close(pipe_fd[0]);
    if(write(pipe_fd[1], MSG_STR, strlen(MSG_STR)) < 0)
    {
        printf("Parent process write pipe failed: %s\n", strerror(errno));
        return -3;
    }
    
    printf("Parent process write pipe %d bytes data: %s\n", strlen(MSG_STR), MSG_STR);
    wait(&wstatus);

    return 0;
}