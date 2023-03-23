#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define FTOK_PATH   "/dev/zero"
#define FTOK_PROJID 0x02

typedef struct st_student{
    char name[4];
    int age;
} t_student;

int main(int argc, char **argv)
{
    key_t key;
    int shmid;
    int i;
    t_student *p_student;

    if( (key = ftok(FTOK_PATH, FTOK_PROJID)) == -1)
    {
        printf("ftok() failed: %s\n", strerror(errno));
        return -1;
    }

    shmid = shmget(key, sizeof(t_student), IPC_CREAT|0666);
    if( shmid < 0 )
    {
        printf("shmget() failed: %s\n", strerror(errno));
        return -2;
    }

    p_student = shmat(shmid, NULL, 0);
    if( p_student == (void *)-1 )
    {
        printf("shmat() failed: %s\n", strerror(errno));
        return -3;
    }

    for(i=0; i<4; i++)
    {
        printf("Parent process: name=%s, age=%d\n", p_student->name, p_student->age);
        sleep(1);
    }

    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
