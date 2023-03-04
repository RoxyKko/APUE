#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

int get_ipaddr(char *interface, char* ipaddr, int ipaddr_size);

int main(int argc, char** argv)
{
    char ipaddr[16];
    char *interface = "eth0";
    memset(ipaddr, 0, sizeof(ipaddr));

    if( get_ipaddr(interface, ipaddr, sizeof(ipaddr)) < 0 )
    {
        printf("ERROR: get IP address failure\n");
        return -1;
    } 
    printf("get network interface %s IP address: [%s] \n", interface, ipaddr);
    return 0;
}

int get_ipaddr(char *interface, char * ipaddr, int ipaddr_size)
{
    char buf[1024];
    char *ptr = NULL;
    char *ip_start = NULL;
    char *ip_end = NULL;
    FILE *fp = NULL;
    int len = -1;
    int rv = -1;

    if( !interface || !ipaddr || ipaddr_size <16 )
    {
        printf("ERROR: invalid input parameter\n");
        return -1;
    }

    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "ifconfig %s", interface);
    if( (fp = popen(buf, "r")) == NULL)
    {
        printf("popen() to excute command \"%s\" failure: %s\n", strerror(errno));
        return -2;
    }

    // 设置返回值为-3，表示默认没有成功
    //rv = -3;
    while( fgets(buf, sizeof(buf), fp))
    {
        if (strstr(buf, "inet"))
        {
            ptr = strstr(buf, "inet");
            if( !ptr )
            {
                break;
            }
            ptr += strlen("inet");

            while( isblank(*ptr) )
            {
                ptr++;
            }

            ip_start = ptr;

            while( !isblank(*ptr) )
            {
                ptr++;
            }

            ip_end = ptr;
            memset(ipaddr, 0, sizeof(ipaddr));

            len = ip_end - ip_start;
            // 防止越界
            // 判断问号前面的表达式是否为真，如果为真，则返回冒号前面的表达式，否则返回冒号后面的表达式
            len = len > ipaddr_size ? ipaddr_size : len;
            memcpy(ipaddr, ip_start, len);

            rv = 0;
            break;
        }

    }
    return rv;
}