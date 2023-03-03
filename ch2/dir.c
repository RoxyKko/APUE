#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>  // for chdir() close()
#include <fcntl.h>   // for creat()

#define TEST_DIR "dir"

void cleanup(int fd1, int fd2);

int main(int argc, char *argv[])
{
    // 初始化
    int rv = 0;    // 返回值
    int fd1 = -1;   // 文件描述符1
    int fd2 = -1;   // 文件描述符2
    DIR *dirp = NULL;   // 目录流指针
    struct dirent *direntp = NULL;   // 目录项指针

    /* 创建文件夹，文件权限755 */
    if (mkdir(TEST_DIR, 0755) < 0)
    {
        printf("create directory '%s' failure: %s \n", TEST_DIR, strerror(errno));
        rv = -1;
    }

    /* 更改当前工作路径到文件夹dir下 */
    if (chdir(TEST_DIR) < 0)
    {
        printf("Change directory to '%s' failure: %s\n",TEST_DIR, strerror(errno));
        rv = -2;
        cleanup(fd1, fd2);
    }

    /* 在dir文件夹下 创建普通文本文件file1.txt，并设置其权限位为644 */
    if ((fd1 = creat("file1.txt", 0644)) < 0)
    {
        printf("Create file1.txt failure: %s\n", strerror(errno));
        rv = -3;
        cleanup(fd1, fd2);
    }

    /* 在dir文件夹下 创建普通文本文件file2.txt，并设置其权限位为644 */
    if ((fd2 = creat("file2.txt", 0644)) < 0)
    {
        printf("Create file2.txt failure: %s\n", strerror(errno));
        rv = -4;
        cleanup(fd1, fd2);
    }

    /* 更改当前工作路径到父目录去 */
    if (chdir("../") < 0)
    {
        printf("Change directory to '%s' failure: %s\n", TEST_DIR, strerror(errno));
        rv = -5;
        cleanup(fd1, fd2);
    }

    /* 打开dir文件夹 */
    if ((dirp = opendir(TEST_DIR)) == NULL)
    {
        printf("opendir %s error: %s\n", TEST_DIR, strerror(errno));
        rv = -6;
    }

    /* 列出dir里面的所有文件和文件夹 */
    while((direntp = readdir(dirp)) != NULL)
    {
        printf("Find file: %s\n", direntp->d_name);
    }

    /* 关闭所有打开的文件夹 */
    closedir(dirp);
    return rv;

}
void cleanup(int fd1, int fd2)
{
        if(fd1 >= 0)
    {
        close(fd1);
    }

    if(fd2 >= 0)
    {
        close(fd2);
    }
}