#include <iostream>
#include <omp.h>

int main()
{
	int x = 44;
#pragma omp parallel for private(x)
	for (int i = 0; i <= 10; i++) 
	{
		x = i;
		printf("Thread number: %d x: %d\n", omp_get_thread_num(), x);
	}
	printf("x is %d\n", x);
}