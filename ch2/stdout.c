#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define MSG_STR "Hello World\n"

int main(int main, char *argv[])
{
	printf("%s", MSG_STR);
	// fputs 写入什么 写入到哪
	fputs(MSG_STR, stdout);
	// write 写入（在哪写入， 写入什么， 写入多少）
	write(STDOUT_FILENO, MSG_STR, strlen(MSG_STR));

	return 0;
}
