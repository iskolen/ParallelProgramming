#include <windows.h>
#include <string>
#include <iostream>
#include "tchar.h"
#include <fstream>

CRITICAL_SECTION FileLockingCriticalSection;

int ReadFromFile(){
	EnterCriticalSection(&FileLockingCriticalSection);
	std::fstream myfile("balance.txt", std::ios_base::in);
	int result;
	myfile >> result;
	myfile.close();
	LeaveCriticalSection(&FileLockingCriticalSection);

	return result;
}

void WriteToFile( int data ){
	EnterCriticalSection(&FileLockingCriticalSection);
	std::fstream myfile("balance.txt",  std::ios_base::out);
	myfile << data << std::endl;
	myfile.close();
	LeaveCriticalSection(&FileLockingCriticalSection);
}

int GetBalance(){
	int balance = ReadFromFile();
	return balance;
}

void Deposit(int money){
	int balance = GetBalance();
	balance += money;

	WriteToFile(balance);
	printf("Balance after deposit: %d\n", balance);
}

void Withdraw(int money){
	if (GetBalance() < money){
		printf("Cannot withdraw money, balance lower than %d\n", money);
		return;
	}

	Sleep(20);
	int balance = GetBalance();
	balance -= money;
	WriteToFile(balance);
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
	HANDLE* handles = new HANDLE[49];

	InitializeCriticalSection(&FileLockingCriticalSection);

	WriteToFile(0);

	SetProcessAffinityMask(GetCurrentProcess(), 1);
	for ( int i = 0; i < 50; i++ ){
		handles[i] = ( i % 2 == 0 )
			? CreateThread(NULL, 0, &DoDeposit, (LPVOID)230, CREATE_SUSPENDED, NULL)
			: CreateThread(NULL, 0, &DoWithdraw, (LPVOID)1000, CREATE_SUSPENDED, NULL);
		ResumeThread(handles[i]);
	}


	// ожидание окончания работы двух потоков
	WaitForMultipleObjects(50, handles, true, INFINITE);
	printf("Final Balance: %d\n", GetBalance());

	getchar();

	return 0;
}
