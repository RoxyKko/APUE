#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define FTOK_PATH   "/tmp"
#define FTOK_PROJID 0x02

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int semaphore_init(void);
int semaphore_p(int sem_id);
int semaphore_v(int sem_id);
void semaphore_term(int sem_id);

int main(int argc, char **argv)
{
    int sem_id;
    pid_t pid;
    int i;

    if( (sem_id = semaphore_init() ) == -1)
    {
        printf("semaphore_init() failed: %s\n", strerror(errno));
        return -1;
    }

    if( (pid = fork()) < 0)
    {
        printf("fork() failed: %s\n", strerror(errno));
        return -2;
    }
    else if(pid == 0)
    {
        printf("Child process start running and do something now..\n");
        sleep(3);
        printf("Child process do something over...\n");
        semaphore_v(sem_id);

        sleep(1);

        printf("Child process exit now...\n");
        exit(0);
    }

    /* 前面的semaphore_init()函数里将信号量的值设为0，如果这时候父进程先执行的话，P操作会阻塞。直到子进程执行V操作
        后，父进程的P操作才能返回继续执行 */
    printf("Parent process P operator wait child process over..\n");
    semaphore_p(sem_id);

    printf("Parent process destroy semaphore and exit\n");
    sleep(2);
    printf("Child process exit and \n");

    semaphore_term(sem_id);
    return 0;
}

int semaphore_init(void)
{
    key_t key;
    int sem_id;
    union semun sem_union;

    // ftok()函数获取IPC关键字
    if( (key = ftok(FTOK_PATH, FTOK_PROJID)) < 0)
    {
        printf("ftok() get IPC token failed: %s\n", strerror(errno));
        return -1;
    }

    // semget()函数创建信号量
    sem_id = semget(key, 1, 0666 | IPC_CREAT);
    if(sem_id < 0)
    {
        printf("semget() create semaphore failed: %s\n", strerror(errno));
        return -2;
    }

    // 初始化信号集
    sem_union.val = 0;
    if( semctl(sem_id, 0, SETVAL, sem_union) < 0 )
    {
        printf("semctl() set semaphore failed: %s\n", strerror(errno));
        return -3;
    }

    printf("Semaphore get key_t[0x%x] and sem_id[%d]\n", key, sem_id);

    return sem_id;
}

void semaphore_term(int sem_id)
{
    union semun sem_union;
    if( semctl(sem_id, 0, IPC_RMID, sem_union) < 0)
    {
        printf("semop() delete semaphore ID failed: %s \n", strerror(errno));
    }

    return ;
}

int semaphore_p(int sem_id)
{
    struct sembuf _sembuf;

    _sembuf.sem_num = 0;
    _sembuf.sem_op = -1;
    _sembuf.sem_flg = SEM_UNDO; // IPC_NOWAIT SEM_UNDO

    if( semop(sem_id, &_sembuf, 1) < 0)
    {
        printf("semop() P operation failed: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

int semaphore_v(int sem_id)
{
    struct sembuf _sembuf;

    _sembuf.sem_num = 0;
    _sembuf.sem_op = 1;
    _sembuf.sem_flg = SEM_UNDO; // IPC_NOWAIT SEM_UNDO

    if( semop(sem_id, &_sembuf, 1) < 0)
    {
        printf("semop() V operation failed: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}