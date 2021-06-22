#include <iostream>
#include "MemoryDump.h"
#include "LockFreeQ.h"
#include <process.h>
#include <map>

CrashDump memoryDump;
#define THREAD_NUM 16
#define INIT_DATA 0x0000000055555555
#define INIT_COUNT  0
#define DATA_COUNT 3
#define TEST_MODE 1

HANDLE g_Thread[THREAD_NUM];
HANDLE g_EnQThread;
HANDLE g_DeQThread;
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

unsigned int __stdcall TestEnQ(LPVOID param)
{
	TestData data;

	while (true)
	{
		InterlockedIncrement64(&data._Data);
		InterlockedIncrement(&data._RefCount);


		if (data._Data != INIT_DATA + 1)
		{
			Crash();
		}
		if (data._RefCount != INIT_COUNT + 1)
		{
			Crash();
		}

		InterlockedDecrement64(&data._Data);
		InterlockedDecrement(&data._RefCount);

		if (data._Data != INIT_DATA)
		{
			Crash();
		}
		if (data._RefCount != INIT_COUNT)
		{
			Crash();
		}

		g_Q.EnQ(&data);
	}

}


unsigned int __stdcall TestDeQ(LPVOID param)
{
	TestData* data = nullptr;

	while (true)
	{


		if (!g_Q.DeQ(&data))
		{
			continue;
		}
	
		if (data->_Data != INIT_DATA)
		{
			Crash();
		}
		if (data->_RefCount != INIT_COUNT)
		{
			Crash();
		}

	}

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

		//Sleep(10);

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
		//Sleep(1);
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

#if TEST_MODE ==1
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

#endif

#if TEST_MODE==2

	g_EnQThread = (HANDLE)_beginthreadex(NULL, 0, TestEnQ, NULL, 0, NULL);
	g_DeQThread = (HANDLE)_beginthreadex(NULL, 0, TestDeQ, NULL, 0, NULL);
#endif


	while (true)
	{
		wprintf(L"Q Count: %d\n", g_Q.m_Count);
		wprintf(L"Memory AllocCount: %d\n", g_Q.GetMemoryPoolAllocCount());
		wprintf(L"m_Rear :%p\n", g_Q.m_RearCheck->_NodePtr);
		wprintf(L"m_Front :%p\n", g_Q.m_FrontCheck->_NodePtr);
		wprintf(L"m_ID :%lld\n", g_Q.m_ID);
		Sleep(1000);

	}
}


