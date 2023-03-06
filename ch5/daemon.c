#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <libgen.h>

int main(int argc, char** argv)
{
	char *progname = basename(argv[0]);

	if( daemon(0,0) < 0)
	{
		printf("program daemon() failure: %s\n", strerror(errno));
		return -1;
	}
	openlog("damon", LOG_CONS | LOG_PID, 0);
	syslog(LOG_NOTICE, "Program '%s' start running\n", progname);
	syslog(LOG_WARNING, "Progarm '%s' running with a warnning message\n", progname);
	syslog(LOG_EMERG, "Program '%s' running with a emergency message\n", progname);
	int i = 0;
	while(1)
	{
		;
	//	i++;
	//	sleep(1);
	//	if(i >= 5)
	//	{
	//		break;
	//	}

	}

	syslog(LOG_NOTICE, "Program '%s' stop running\n", progname);
	closelog();

	return 0;
}
