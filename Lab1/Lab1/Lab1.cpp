#include <windows.h>
#include <string>
#include <iostream>

DWORD WINAPI ThreadProc(CONST LPVOID lpParam)
{
	printf("Поток №%d выполняет свою работу\n", static_cast<int>(reinterpret_cast<intptr_t>(lpParam)));
	ExitThread(0);
}

int main(int argc, CHAR* argv[])
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	if (argc != 2)
	{
		std::cerr << "LabTest1.exe <количество потоков>" << std::endl;
		return -1;
	}

	int numThreads = atoi(argv[1]);

	if (numThreads < 0)
	{
		std::cerr << "Количество потоков не может быть отрицательным!" << std::endl;
		return -2;
	}

	HANDLE* handles = new HANDLE[numThreads];

	for (int i = 0; i < numThreads; i++)
	{
		handles[i] = CreateThread(NULL, 0, &ThreadProc, (LPVOID)(intptr_t)i, CREATE_SUSPENDED, NULL);
	}

	for (int m = 0; m < numThreads; m++)
		ResumeThread(handles[m]);

	WaitForMultipleObjects(numThreads, handles, true, INFINITE);

	for (int i = 0; i < numThreads; i++)
		CloseHandle(handles[i]);

	return 0;
}