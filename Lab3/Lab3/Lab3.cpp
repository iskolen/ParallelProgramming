#include <windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <iomanip>
#include <random>

const int NUM_THREADS = 2;
const int NUM_OPERATIONS = 10;
const int TIME_SLEEP = 100;

struct ThreadData 
{
	int numThread;
	DWORD startTime;
	FILE* outputFile;
};

void PayLoad()
{
	double result;
	for (int i = 0; i < 1000000; i++)
	{
		result = sin(0.5);
	}
}

DWORD WINAPI ThreadProc(CONST LPVOID lpParam)
{
	ThreadData* threadData = static_cast<ThreadData*>(lpParam);
	int threadId = threadData->numThread;

	for (int i = 0; i < NUM_OPERATIONS; i++)
	{
		PayLoad();
		DWORD endTime = timeGetTime();
		DWORD elapsedTime = endTime - threadData->startTime;

		fprintf(threadData->outputFile, "%d|%u\n", threadId, elapsedTime);
	}

	ExitThread(0);
}

int main()
{
	setlocale(LC_ALL, "Russian");

	std::cout << "Нажмите Enter, чтобы начать выполнение программы.";
	std::cin.get();

	FILE* logFile;
	if (fopen_s(&logFile, "log.txt", "w") != 0)
	{
		std::cerr << "Ошибка открытия файла." << std::endl;
		return 1;
	}

	HANDLE handles[NUM_THREADS];
	ThreadData threadData[NUM_THREADS];
	DWORD startTime = timeGetTime();

	for (int i = 0; i < NUM_THREADS; i++)
	{
		threadData[i].numThread = i;
		threadData[i].startTime = startTime;
		threadData[i].outputFile = logFile;

		handles[i] = CreateThread(NULL, 0, &ThreadProc, &threadData[i], CREATE_SUSPENDED, NULL);
	}

	for (int m = 0; m < NUM_THREADS; m++)
		ResumeThread(handles[m]);

	std::cout << "Ожидание завершения потоков..." << std::endl;
	WaitForMultipleObjects(NUM_THREADS, handles, true, INFINITE);

	std::cout << "Задачи завершены. Данные записаны в файл log.txt." << std::endl;

	return 0;
}