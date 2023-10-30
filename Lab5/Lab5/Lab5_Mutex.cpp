#include <windows.h>
#include <string>
#include <iostream>
#include "tchar.h"
#include <fstream>

HANDLE FileLockingMutex;
HANDLE BalanceLockingMutex;

int ReadFromFile()
{
	WaitForSingleObject(FileLockingMutex, INFINITE);
	std::fstream myfile("balance.txt", std::ios_base::in);
	int result = 0;
	myfile >> result;
	myfile.close();
	ReleaseMutex(FileLockingMutex);

	return result;
}

void WriteToFile(int data)
{
	WaitForSingleObject(FileLockingMutex, INFINITE);
	std::fstream myfile("balance.txt", std::ios_base::out);
	myfile << data << std::endl;
	myfile.close();
	ReleaseMutex(FileLockingMutex);
}

int GetBalance()
{
	WaitForSingleObject(BalanceLockingMutex, INFINITE);
	int balance = ReadFromFile();
	ReleaseMutex(BalanceLockingMutex);
	return balance;
}

void Deposit(int money)
{
	WaitForSingleObject(BalanceLockingMutex, INFINITE);
	int balance = GetBalance();
	balance += money;
	WriteToFile(balance);
	ReleaseMutex(BalanceLockingMutex);
	printf("Balance after deposit: %d\n", balance);
}

void Withdraw(int money)
{
	WaitForSingleObject(BalanceLockingMutex, INFINITE);
	int currentBalance = GetBalance();
	if (currentBalance < money)
	{
		printf("Cannot withdraw money, balance lower than %d\n", money);
		ReleaseMutex(BalanceLockingMutex);
		return;
	}
	Sleep(20);
	int balance = currentBalance - money;
	WriteToFile(balance);
	ReleaseMutex(BalanceLockingMutex);
	printf("Balance after withdraw: %d\n", balance);
}

DWORD WINAPI DoDeposit(CONST LPVOID lpParameter)
{
	Deposit((int)lpParameter);
	ExitThread(0);
}

DWORD WINAPI DoWithdraw(CONST LPVOID lpParameter)
{
	Withdraw((int)lpParameter);
	ExitThread(0);
}

int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE* handles = new HANDLE[50];

	FileLockingMutex = CreateMutex(NULL, FALSE, NULL);
	BalanceLockingMutex = CreateMutex(NULL, FALSE, NULL);

	WriteToFile(0);

	SetProcessAffinityMask(GetCurrentProcess(), 1);
	for (int i = 0; i < 50; i++)
	{
		handles[i] = (i % 2 == 0)
			? CreateThread(NULL, 0, &DoDeposit, (LPVOID)230, CREATE_SUSPENDED, NULL)
			: CreateThread(NULL, 0, &DoWithdraw, (LPVOID)1000, CREATE_SUSPENDED, NULL);
		ResumeThread(handles[i]);
	}

	WaitForMultipleObjects(50, handles, true, INFINITE);
	printf("Final Balance: %d\n", GetBalance());

	getchar();

	CloseHandle(FileLockingMutex);
	CloseHandle(BalanceLockingMutex);
	delete[] handles;

	return 0;
}