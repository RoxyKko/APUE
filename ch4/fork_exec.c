#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

// 在内存中的文件系统/tmp
#define TMP_FILE "/tmp/.ifconfig.log"

int main(int argc, char **argv)
{
	pid_t pid = -1;
	int fd = -1;
	char buf[1024];
	int rv = -1;
	FILE *fp = NULL;
	char *ptr = NULL;
	char *ip_start = NULL;
	char *ip_end = NULL;
	char ipaddr[16];

	// 如果忘记了open参数，其为
	// （文件描述符， 可读可写｜如果不存在则创建｜以可写/可读可写打开时将光标置于文件首， 文件权限）
	// 打开不成功则返回值<0，不存在 = 0，这里不存在则创建，故不可能，打开成功返回该文件的文件描述符
	if ((fd = open(TMP_FILE, O_RDWR | O_CREAT | O_TRUNC, 0644)) < 0)
	{
		printf("Redirect standard output to file failure: %s\n", strerror(errno));
		return -1;
	}

	if ((pid = fork()) < 0)
	{
		printf("fork() creat child proess failure: %s\n", strerror(errno));
		return -2;
	}
	// 进入子进程
	else if (pid == 0)
	{
		printf("Child proess start excute ifconfig program\n");
		// 重定向函数，把输出重定向至fd文件描述符对应的文件
		dup2(fd, STDOUT_FILENO);

		/*******************重点部分**********************/

		// 执行文件（程序）， 命令， 参数， 最后以NULL结尾
		execl("/sbin/ifconfig", "ifconfig", "eth0", NULL);

		// execl并不会返回，执行execl后，该进程将抛弃父进程的文本段、数据段，加载指定参数的程序的文本、数据段并重新建立内存空间
		// 若exec*类的函数返回，则说明发生错误
		printf("Child proess excute another program, But now return, it mean is execl() error\n");
		return -3;
	}
	else
	{
		// 父进程等待3s，等待子进程先执行
		sleep(3);
	}

	// 由于子进程已经丢弃父进程的文本段，故不会运行到这
	// 只有父进程会运行到此
	memset(buf, 0, sizeof(buf));

	// 此时读不到任何东西，因为子进程往文件写内容将文件偏移量修改到文件尾部了
	rv = read(fd, buf, sizeof(buf));
	printf("Read %d bytes date dierectly read after child proess write\n", rv);

	memset(buf, 0, sizeof(buf));
	// 将文件偏移量移至文件头
	lseek(fd, 0, SEEK_SET);
	rv = read(fd, buf, sizeof(buf));
	printf("Read %d bytes date dierectly read after child proess write\n", rv);

	// 一行一行的读取
	memset(buf, 0, sizeof(buf));
	// 转化成文件流
	fp = fdopen(fd, "r");

	fseek(fp, 0, SEEK_SET);
	while (fgets(buf, sizeof(buf), fp)) // 一次读一行，读到文件尾返回NULL
	{
		// 如果行中包含netmask
		if (strstr(buf, "netmask"))
		{
			ptr = strstr(buf, "inet");
			if (!ptr)
			{
				break;
			}
			// 跳过字符inet
			ptr += strlen("inet");

			// inet后是空白符，不清楚是空格还是TAB，用isblank()判断
			// 若是空白符则跳过
			while (isblank(*ptr))
			{
				ptr++;
			}

			// 跳过空白符后是IP地址的起始字符
			ip_start = ptr;

			// IP地址的点分十进制后是空白符，故只要判断出现空白符后视为IP地址的尾
			while (!isblank(*ptr))
			{
				ptr++;
			}

			ip_end = ptr;
			// 使用memcpy()将ip_start~ip_end两指针中间的内存
			memcpy(ipaddr, ip_start, ip_end - ip_start);
			break;
		}
	}

	printf("Parser and get IP address: %s\n", ipaddr);

	fclose(fp);
	unlink(TMP_FILE);

	return 0;
}
