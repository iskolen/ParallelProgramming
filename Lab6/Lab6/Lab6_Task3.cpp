#include <iostream>
#include <omp.h>

const int N = 250;
const int NUM_THREADS = 4;

void MatrixMultParallel(int A[N][N], int B[N][N], int C[N][N])
{
	double startTime, endTime;
	startTime = omp_get_wtime();
#pragma omp parallel for num_threads(NUM_THREADS)
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			int sum = 0;
			for (int k = 0; k < N; k++)
			{
				sum += A[i][k] * B[k][j];
			}
#pragma omp critical
			C[i][j] = sum;
		}
	}
	endTime = omp_get_wtime();
	printf("Параллельное умножение: Время выполнения = %.6f секунд\n", endTime - startTime);
}

void MatrixMult(int A[N][N], int B[N][N], int C[N][N])
{
	double startTime, endTime;
	startTime = omp_get_wtime();
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			int sum = 0;
			for (int k = 0; k < N; k++)
			{
				sum += A[i][k] * B[k][j];
			}
			C[i][j] = sum;
		}
	}
	endTime = omp_get_wtime();
	printf("Последовательное умножение: Время выполнения = %.6f секунд\n", endTime - startTime);
}

int main()
{
	setlocale(LC_ALL, "Russian");
	int A[N][N];
	int B[N][N];
	int C[N][N];

	//std::cout << "Матрица A:" << std::endl;
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			A[i][j] = i + j;
			//std::cout << A[i][j] << " ";
		}
		//std::cout << std::endl;
	}

	//std::cout << "Матрица B:" << std::endl;
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			B[i][j] = j + i;
			//std::cout << B[i][j] << " ";
		}
		//std::cout << std::endl;
	}

	MatrixMult(A, B, C);
	MatrixMultParallel(A, B, C);

	/*std::cout << "Результат умножения матриц:" << std::endl;
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			std::cout << C[i][j] << " ";
		}
		std::cout << std::endl;
	}*/

	return 0;
}
