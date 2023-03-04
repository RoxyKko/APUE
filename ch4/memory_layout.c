#include <stdio.h>
#include <stdlib.h>

int g_var1;		// save data.bss
int g_var2 = 20;	// save data.data

int main(int argc, char** argv)
{
	static int s_var1;	// save data.bss
	static int s_var2 = 20;	// save datd.data

	char *str = "Hello";	//save STACK and value = &"Hello"
	char *ptr;		//save STACK but value = ?
	
	printf("*str = %p \n", str);
	printf("*ptr = %p \n", ptr);
	ptr = malloc(100);
	printf("new *ptr = %p \n", ptr);

	printf("[cmd args]: argv address: %p\n", argv);
	printf("[  Stack ]: str address: %p\n", &str);
	printf("\n");

	printf("[  Heap  ]: malloc adress: %p\n", ptr);
	printf("\n");

	printf("[  bss   ]: s_var1 address: %p value: %d\n", &s_var1, s_var1);
	printf("[  bss   ]: g_var1 address: %p value: %d\n", &g_var1, g_var1);
	
	printf("[  data  ]: s_var2 address: %p value: %d\n", &s_var2, s_var2);
	printf("[  data  ]: g_var2 address: %p value: %d\n", &g_var2, g_var2);

	printf("[ rodata ]: \"%s\" address: %p \n\n" ,str ,str);

	printf("[  text  ]: main() address: %p \n\n", main);
	return 0;	
}
