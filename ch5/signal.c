#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>

int g_stop = 0;

void sig_handle(int signum);
void sig_stop(int signum);
void sig_user(int signum);
void sig_code(int signum);

int main(int argc, char** argv)
{

	char	*ptr = NULL;
	// 			sigact执行的动作，sigign忽略的信号
	struct sigaction	sigact,sigign;

	/****************使用signal初始化信号*********************/

	// 注册信号函数
	// 第一个填信号类型
	// 第二个是个函数指针，原型为
	// typedef void (*sighandler_t)(int);
	// 默认给返回一个int型
	// 第二个填的执行的函数名称
	// 只要发生（信号类型）就执行（信号函数）
	
	//signal(SIGINT, sig_handle);
	//signal(SIGTERM, sig_handle);
	//signal(SIGALRM, sig_handle);
	
	signal(SIGTERM, sig_stop);
	signal(SIGALRM, sig_stop);

	signal(SIGBUS, sig_code);
	signal(SIGILL, sig_code);
	signal(SIGSEGV, sig_code);

	/***********************用sigaction注册信号******************************/
	// 目前常用都是sigaction，因为这个比signal安全
	// 因为signal会丢失信号
	
	// 初始化结构体
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigact.sa_handler = sig_user;

	// 设置忽的信号
	sigemptyset(&sigign.sa_mask);
	sigign.sa_flags = 0;
	sigign.sa_handler = SIG_IGN;

	sigaction(SIGINT, &sigign, 0);	// 忽略SIGINT信号：CTRL+C

	sigaction(SIGUSR1, &sigact, 0);
	sigaction(SIGUSR2, &sigact, 0);

	printf("Program start running for 20 seconds...\n");
	alarm(20);
	while( !g_stop )
	{
		;
	}
	printf("Power off gprs\n");

	*ptr = 'h';

	return 0;
}

void sig_stop(int signum)
{
	if( SIGTERM == signum )
	{
		printf("SIGTERM signal detected\n");
	}
	else if( SIGALRM == signum )
	{
		printf("SIGALRM signal detected\n");
		g_stop = 1;
	}
}

void sig_code(int signum)
{
	if(signum == SIGBUS)
	{
		printf("SIGBUS signal detected\n");
	}
	else if(signum == SIGILL)
	{
		printf("SIGILL signal detected\n");
	}
	else if(signum == SIGSEGV)
	{
		printf("SIGSEGV signal detected\n");
	}
	exit (-1);
}

void sig_user(int signum)
{
	if( signum == SIGUSR1)
	{
		printf("SIGUSR1 signal detected\n");	
	}
	else if(signum == SIGUSR2)
	{
		printf("SIGUSR2 signal detected\n");
	}
	g_stop = 1;

}

void sig_handle(int signum)
{
	printf("\tCatch signal [%d]\n", signum);
	g_stop = 1;
}
