#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void *thread_worker1(void *args);
void *thread_worker2(void *args);

int main(int argc, char** argv)
{
	int shared_var = 1000;
	pthread_t	tid;
	pthread_attr_t	thread_attr;

	// 初始化结构体
	if( pthread_attr_init(&thread_attr) )
	{
		printf("pthread_attr_init() failure：%s\n", strerror(errno));
		return -1;
	}

	// 设置栈区大小为120KB
	if( (pthread_attr_setstacksize(&thread_attr, 120*1024)) < 0 )
	{
		printf("pthread_attr_setstacksize() failure: %s\n", strerror(errno));
		return -2;
	}

	// 设置线程属性->分离状态为分离
	if( pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED) )
	{
		printf("pthread_attr_setdetachstate() failure: %s\n", strerror(errno));
		return -3;
	}

	// 创建线程，返回线程的线程IP给tid， 线程属性为thread_attr，
	// 线程调用的函数为thread_worker1，
	// shared_var是需要传给线程的参数，如果有多个参数需要传给子线程函数，
	// 则需要将多个参数封装到一个结构体里再传进去
	pthread_create(&tid ,&thread_attr, thread_worker1, &shared_var);
	printf("Thread worker1 tid[%ld] created ok\n", tid);

	// 使用默认结构体，故thread_worker2的分离属性是默认的是可会合
	pthread_create(&tid, NULL, thread_worker2, &shared_var);
	printf("Thread worker2 tid[%ld] created ok\n", tid);

	// 创建子线程完毕，线程属性结构体使用完毕，摧毁释放
	pthread_attr_destroy(&thread_attr);
	// 主线程阻塞，等待直到可会合线程执行完毕（既thread_worker2)
	pthread_join(tid, NULL);

	while(1)
	{
		printf("Main/Control thread shared_var: %d\n", shared_var);
		sleep(10);
	}


}


void *thread_worker1(void *args)
{
	int	*ptr = (int *)args;

	if( !args )
	{
		printf("%s() get invalid arguments\n", __FUNCTION__);
		pthread_exit(NULL);
	}

	printf("Thread workder 1 [%ld] start running ...\n", pthread_self());

	while(1)
	{
		printf("+++: %s before shared_var++: %d\n", __FUNCTION__, *ptr);
		*ptr += 1;
		sleep(2);
		printf("+++: %s after shared_var++: %d\n", __FUNCTION__, *ptr);
	}
	printf("Thread worker 1 exit ...\n");

	return NULL;
}


void *thread_worker2(void *args)
{
	int	*ptr = (int *)args;

	if( !args )
	{
		printf("%s() get invalid arguments\n", __FUNCTION__);
		pthread_exit(NULL);
	}

	printf("Thread workder 2 [%ld] start running ...\n", pthread_self());

	while(1)
	{
		printf("---: %s before shared_var++: %d\n", __FUNCTION__, *ptr);
		*ptr += 1;
		sleep(2);
		printf("---: %s after shared_var++: %d\n", __FUNCTION__, *ptr);
	}
	printf("Thread worker 2 exit ...\n");

	return NULL;
}
