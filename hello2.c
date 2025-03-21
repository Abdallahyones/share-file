#include <stdlib.h>
#include <stdio.h>

int main (void)
{
        char name[50];
	printf("Enter your name: ");
	scanf("%49s", name);
	printf("hello %s.\n", name);
	return EXIT_SUCCESS;
}
