#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void *thread_worker1(void *args);
void *thread_worker2(void *args);

typedef struct worker_ctx_s
{
	int shared_var;
	pthread_mutex_t lock;
} worker_ctx_t;

int main(int argc, char** argv)
{
	worker_ctx_t worker_ctx;
	pthread_t	tid;
	pthread_attr_t	thread_attr;

	// 初始化共享变量
	worker_ctx.shared_var = 1000;
	// 初始化互斥锁
	pthread_mutex_init(&worker_ctx.lock, NULL);

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
	pthread_create(&tid ,&thread_attr, thread_worker1, &worker_ctx);
	printf("Thread worker1 tid[%ld] created ok\n", tid);

	// 也将线程属性设置为分离状态
	pthread_create(&tid, &thread_attr, thread_worker2, &worker_ctx);
	printf("Thread worker2 tid[%ld] created ok\n", tid);

	// 创建子线程完毕，线程属性结构体使用完毕，摧毁释放
	pthread_attr_destroy(&thread_attr);
	// 主线程阻塞，等待直到可会合线程执行完毕（既thread_worker2)
	pthread_join(tid, NULL);

	while(1)
	{
		printf("Main/Control thread shared_var: %d\n", worker_ctx.shared_var);
		sleep(10);
	}


}


void *thread_worker1(void *args)
{
	worker_ctx_t *ctx = (worker_ctx_t *)args;

	if( !args )
	{
		printf("%s() get invalid arguments\n", __FUNCTION__);
		pthread_exit(NULL);
	}

	printf("Thread workder 1 [%ld] start running ...\n", pthread_self());

	while(1)
	{
		// 申请锁，这里是阻塞锁，如果锁被别的线程持有则该函数不会返回，直到锁被释放
		pthread_mutex_lock(&ctx->lock);

		printf("+++: %s before shared_var++: %d\n", __FUNCTION__, ctx->shared_var);
		ctx->shared_var += 1;
		sleep(2);
		printf("+++: %s after shared_var++: %d\n", __FUNCTION__, ctx->shared_var);

		// 释放锁，这样其他线程才能再次访问
		pthread_mutex_unlock(&ctx->lock);
		// 两个都要加上延时，否则一个线程拿到锁之后会一直占有该锁；另外一个线程则不能获取到锁；
		sleep(1);
	}
	printf("Thread worker 1 exit ...\n");

	return NULL;
}


void *thread_worker2(void *args)
{
	worker_ctx_t *ctx = (worker_ctx_t *)args;

	if( !args )
	{
		printf("%s() get invalid arguments\n", __FUNCTION__);
		pthread_exit(NULL);
	}

	printf("Thread workder 2 [%ld] start running ...\n", pthread_self());

	while(1)
	{
		// 申请锁，这里使用的是非阻塞锁；如果锁现在被别的线程占用则返回非0值，如果没有被占用则返回0
		if( pthread_mutex_trylock(&ctx->lock) != 0)
		{
			continue;
		}

		printf("---: %s before shared_var++: %d\n", __FUNCTION__, ctx->shared_var);
		ctx->shared_var += 1;
		sleep(2);
		printf("---: %s after shared_var++: %d\n", __FUNCTION__, ctx->shared_var);

		pthread_mutex_unlock(&ctx->lock);
		// 两个都要加上延时，否则一个线程拿到锁之后会一直占有该锁；另外一个线程则不能获取到锁；
		sleep(1);
	}
	printf("Thread worker 2 exit ...\n");

	return NULL;
}
