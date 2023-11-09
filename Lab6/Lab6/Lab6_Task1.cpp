#include <iostream>
#include <omp.h>

// 1. С использованием for (синхронно) - вычисление 1-им потоком
void CalculatePiSync(int numIterations)
{
	double pi = 0;
	double startTime, endTime;
	startTime = omp_get_wtime();

	for (int i = 0; i < numIterations; i++)
	{
		pi = pi + 4.0 * pow(-1, i) / (2.0 * i + 1.0);
	}

	endTime = omp_get_wtime();

	printf("Значение PI (for): %.10f\n", pi);
	printf("Время выполнения: %.6f секунд\n", endTime - startTime);
}

// 2. С использование параллельного for (работает неверно) - гонка данных
void CalculatePiParallel(int numIterations)
{
	double pi = 0;
	double startTime, endTime;
	startTime = omp_get_wtime();

#pragma omp parallel
	{
#pragma omp for
		for (int i = 0; i < numIterations; i++)
		{
			pi = pi + 4.0 * pow(-1, i) / (2.0 * i + 1.0);
		}
	}

	endTime = omp_get_wtime();

	printf("Значение PI (параллельный for): %.10f\n", pi);
	printf("Время выполнения: %.6f секунд\n", endTime - startTime);
}

// 3. Рабочее решение с использованием parallel for и atomic
void CalculatePiAtomic(int numIterations)
{
	double pi = 0;
	double startTime, endTime;
	startTime = omp_get_wtime();

#pragma omp parallel
	{
		double localSum = 0;
#pragma omp for
		for (int i = 0; i < numIterations; i++)
		{
			localSum = localSum + 4.0 * pow(-1, i) / (2.0 * i + 1.0);
		}
#pragma omp atomic
		pi += localSum;
	}

	endTime = omp_get_wtime();

	printf("Значение PI (for и atomic): %.10f\n", pi);
	printf("Время выполнения: %.6f секунд\n", endTime - startTime);
}

// 4. С использованием reduction
void СalculatePiReduction(int numIterations)
{
	double pi = 0;
	double startTime, endTime;
	startTime = omp_get_wtime();

#pragma omp parallel reduction(+:pi)
	{
#pragma omp for
		for (int i = 0; i < numIterations; i++)
		{
			pi = pi + 4.0 * pow(-1, i) / (2.0 * i + 1.0);
		}
	}

	endTime = omp_get_wtime();

	printf("Значение PI: %.10f\n", pi);
	printf("Время выполнения: %.6f секунд\n", endTime - startTime);
}

int main()
{
	setlocale(LC_ALL, "Russian");
	int numIterations = 10000000;
	CalculatePiSync(numIterations);
	CalculatePiParallel(numIterations);
	CalculatePiAtomic(numIterations);
	СalculatePiReduction(numIterations);
	return 0;
}