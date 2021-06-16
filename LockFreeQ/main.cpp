#include <iostream>
#include "MemoryDump.h"
#include "LockFreeQ.h"
#include <process.h>
#include <map>

CrashDump memoryDump;
#define THREAD_NUM 4
#define INIT_DATA 0x0000000055555555
#define INIT_COUNT  0
#define DATA_COUNT 3

HANDLE g_Thread[THREAD_NUM];

MemoryLogging_ST<10000> g_MemoryLogQ;
int64_t g_MemoryCount = 0;
struct TestData
{
	TestData()
	{
		_Data = INIT_DATA;
		_RefCount = INIT_COUNT;
	}
	int64_t _Data;
	LONG _RefCount;
};

LockFreeQ<TestData*> g_Q;


void Crash()
{
	int* p = nullptr;
	*p = 20;
}

unsigned int __stdcall TestThread(LPVOID param)
{
	TestData** dataArray = (TestData**)param;

	while (1)
	{
		for (int i = 0; i < DATA_COUNT; ++i)
		{
			InterlockedIncrement64(&dataArray[i]->_Data);
			InterlockedIncrement(&dataArray[i]->_RefCount);
		}
		//Sleep(5);

		for (int i = 0; i < DATA_COUNT; ++i)
		{
			if (dataArray[i]->_Data != INIT_DATA + 1)
			{
				Crash();
			}
			if (dataArray[i]->_RefCount != INIT_COUNT + 1)
			{
				Crash();
			}
		}
		//Sleep(5);

		for (int i = 0; i < DATA_COUNT; ++i)
		{
			InterlockedDecrement64(&dataArray[i]->_Data);
			InterlockedDecrement(&dataArray[i]->_RefCount);
		}
		//Sleep(5);
		for (int i = 0; i < DATA_COUNT; ++i)
		{
			if (dataArray[i]->_Data != INIT_DATA)
			{
				Crash();
			}
			if (dataArray[i]->_RefCount != INIT_COUNT)
			{
				Crash();
			}
		}

		for (int i = 0; i < DATA_COUNT; ++i)
		{
			g_Q.EnQ(dataArray[i]);
		}

		//Sleep(5);

		memset(dataArray, 0, sizeof(TestData*) * DATA_COUNT);

		int failCount = 0;

		for (int i = 0; i < DATA_COUNT; ++i)
		{
			if (!g_Q.DeQ(&dataArray[i]))
			{
				//Crash();
				--i;
				failCount++;
			}
		}
		//Sleep(5);
		for (int i = 0; i < DATA_COUNT; ++i)
		{
			if (dataArray[i] == nullptr)
			{
				Crash();
			}
			if (dataArray[i]->_Data != INIT_DATA)
			{
				Crash();
			}
			if (dataArray[i]->_RefCount != INIT_COUNT)
			{
				Crash();
			}
		}
	}


	return 0;
}
int main()
{
	TestData* dataArray[THREAD_NUM][DATA_COUNT];
	std::map<int64_t, TestData*> DataReUseTest;

	for (int i = 0; i < THREAD_NUM; ++i)
	{
		for (int j = 0; j < DATA_COUNT; ++j)
		{
			dataArray[i][j] = new TestData;

			DataReUseTest.insert(std::make_pair((int64_t)dataArray[i][j], dataArray[i][j]));

			if (dataArray[i][j] == nullptr)
			{
				Crash();
			}
			if (dataArray[i][j]->_Data != INIT_DATA)
			{
				Crash();
			}
			if (dataArray[i][j]->_RefCount != INIT_COUNT)
			{
				Crash();
			}
		}

	}

	for (int i = 0; i < THREAD_NUM; ++i)
	{
		for (int j = 0; j < DATA_COUNT; ++j)
		{
			
			auto iter= DataReUseTest.find((int64_t)dataArray[i][j]);
			
			if (iter == DataReUseTest.end())
			{
				Crash();
			}
			if (dataArray[i][j] == nullptr)
			{
				Crash();
			}
			if (dataArray[i][j]->_Data != INIT_DATA)
			{
				Crash();
			}
			if (dataArray[i][j]->_RefCount != INIT_COUNT)
			{
				Crash();
			}
		}

	}
	for (int i = 0; i < THREAD_NUM; ++i)
	{
		g_Thread[i] = (HANDLE)_beginthreadex(NULL, 0, TestThread, (void*)dataArray[i], 0, NULL);
	}


	while (true)
	{
		wprintf(L"Q Count: %d\n", g_Q.m_Count);
		wprintf(L"m_Rear: %p\n", g_Q.m_RearCheck->_NodePtr);
		wprintf(L"m_Rear Next: %p\n", g_Q.m_RearCheck->_NodePtr->_Next);
	
		Sleep(1000);

	}
}


