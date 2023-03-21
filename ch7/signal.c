#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>

int g_child_stop = 0;
int g_parent_run = 0;

void sig_child(int signum)
{
    if (signum == SIGUSR1)
    {
        g_child_stop = 1;
    }
}

void sig_parent(int signum)
{
    if (signum == SIGUSR2)
    {
        g_parent_run = 1;
    }
}

int main(int argc, char** argv)
{
    int pid;
    int wstatus;

    signal(SIGUSR1, sig_child);
    signal(SIGUSR2, sig_parent);
    if((pid = fork()) < 0)
    {
        printf("Create child process failed: %s\n", strerror(errno));
        return -1;
    }
    else if (pid == 0)
    {
        printf("Child process start running and send parent process a signal\n");
        kill(getppid(), SIGUSR2);

        while(!g_child_stop)
        {
            sleep(1);
        }

        printf("Child process receive parent process signal and stop running\n");
        return 0;
    }
    
    printf("Parent process start running and wait child process\n");
    while(!g_parent_run)
    {
        sleep(1);
    }

    // parent process can do something here, then tell child process to exit
    printf("Parent start running now and send child a signal to exit\n");
    kill(pid, SIGUSR1);

    // wait child process exit
    wait(&wstatus);
    printf("Parent process receive child process signal and exit\n");

    return 0;
}