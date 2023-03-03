#include <stdio.h>
#include <unistd.h>

// 改成可执行文件看看结果
#define TEST_FILE "access"

int main(int argc, char *argv[])
{
    // 测试文件是否存在
    if(access(TEST_FILE, F_OK) != 0)
    {
        printf("File %s not exist!\n", TEST_FILE);
        return 0;
    }

    printf("File %s exist!\n", TEST_FILE);
	// 测试读许可权
    if(access(TEST_FILE, R_OK) == 0)
    {
        printf("READ OK\n");
    }
	// 测试写许可权
    if(access(TEST_FILE, W_OK) == 0)
    {
        printf("WRITE OK\n");
    }
	// 测试执行许可权
    if(access(TEST_FILE, X_OK) == 0)
    {
        printf("EXECUTE OK\n");
    }
    
    return 0;
}