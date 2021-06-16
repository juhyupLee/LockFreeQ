#pragma once
#include <iostream>
#include <Windows.h>
#include "MemoryLog.h"


#define MARK_FRONT 0x12345678
#define MARK_REAR 0x87654321


enum
{
	DEQ_POINTER=1, //1
	ENQ_POINTER,   //2
	FREE_FLAG_CHECK, //3
	FREE_OKAY,  //4
	ALLOC_OKAY,//5
	ALLOC_MEMORY//6

};
//extern MemoryLogging_ST<int64_t, 10000> g_MemoryLog;

extern long long g_Mark;

class FreeListException
{
public:
	FreeListException(const wchar_t* str, int line)
		:m_Line(line)
	{
		wcscpy_s(m_String,str);
	}
	void what()
	{
		wprintf(L"%s  [File:FreeList.h] [Line:%d]\n", m_String, m_Line);
	}
	int m_Line;
	wchar_t m_String[128];
};

template <typename T>
class FreeList
{
public:
	struct Mark
	{
		int64_t _MarkID;
		int32_t _MarkValue;
		int32_t _FreeFlag;
	};
	struct Node
	{
		Node()
			:_Next(nullptr)
		{

		}
		T _Data;
		Node* _Next;
		int64_t _ID;
	};

	struct TopCheck
	{
		TopCheck()
		{
			_TopPtr = nullptr;
			_ID = -1;
		}
		Node* _TopPtr;
		int64_t _ID;
	};
	struct AllocMemory
	{
		Mark _FrontMark;
		Node _Node;
		Mark _RearMark;
	};

	FreeList(bool placementNew = false)
		:
		m_bPlacementNew(placementNew),
		m_UseCount(0),
		m_PoolCount(0),
		m_MarkValue(++g_Mark)
	{
		m_NodeID = -1;
		m_TopCheck = new TopCheck;

	}
	FreeList(int32_t blockNum, bool placementNew=false)
		:
		m_bPlacementNew(placementNew),
		m_MarkValue(++g_Mark)
	{
		for (int i = 0; i < blockNum; i++)
		{
			T* data = AllocateNewMemory();
			Free(data);
		}
	}
	~FreeList()
	{
		//--------------------------------------------------------------------------
		// 더미노드를 제외한, 데이터 노드들은 기존 포인터에서 128바이트 만큼 땡겨, delete를 해야한다
		//--------------------------------------------------------------------------
		Node* curNode = m_TopCheck->_TopPtr;

		while (curNode != nullptr)
		{
			char* delNode = (char*)curNode;

			if (!m_bPlacementNew)
			{
				//---------------------------------------------------------------------------------
				// 맨처음 메모리 malloc으로 할당할때, 객체화를 위해 해준, PlacementNew로 인한 생성자 호출때문에,
				// 소멸자를 맨마지막에는 호출해준다.
				//---------------------------------------------------------------------------------
				((T*)delNode)->~T();
			}

			delNode = delNode - (sizeof(int64_t)*2);

			curNode = curNode->_Next;
			
			free(delNode);
		}

		delete m_TopCheck;

	}

	int32_t GetPoolCount();
	int32_t GetUseCount();
	bool Free(T* data);
	T* Alloc();

private:
	T* AllocateNewMemory();

private:
	TopCheck* m_TopCheck;

	bool m_bPlacementNew;
	LONG m_UseCount;
	LONG m_PoolCount;
	int64_t m_NodeID;

	const int64_t m_MarkValue;

};


//
//template <int size>
//class Data
//{
//public:
//	Data() { memset(m_Buffer, 7, size); };
//	~Data() {};
//private:
//	char m_Buffer[size];
//};


//class MemoryPool
//{
//
//public:
//	MemoryPool() {};
//
//	Data<16>* Alloc(int size)
//	{
//		m_128 = new FreeList<Data<16>>();
//		
//		return m_128->Alloc();
//	}
//private:
//	FreeList<Data<16>>* m_128;
//
//};

template<typename T>
inline int32_t FreeList<T>::GetPoolCount()
{
	return m_PoolCount;
}

template<typename T>
inline int32_t FreeList<T>::GetUseCount()
{
	return m_UseCount;
}

template<typename T>
inline T* FreeList<T>::AllocateNewMemory()
{
	
	AllocMemory* allocMemory;
	//--------------------------------------------------------------------------
	// 언더플로우 체크용 mark ID  + data(Payload)  + 오버플로우 체크용 mark ID  할당
	//--------------------------------------------------------------------------
	allocMemory = (AllocMemory*)malloc(sizeof( AllocMemory));
	new(&allocMemory->_Node)T;
	allocMemory->_FrontMark._FreeFlag = 0;
	allocMemory->_FrontMark._MarkID = m_MarkValue;
	allocMemory->_FrontMark._MarkValue = MARK_FRONT;
	allocMemory->_RearMark._MarkID = m_MarkValue;
	allocMemory->_RearMark._MarkValue = MARK_REAR;

	Node* tempNode = &allocMemory->_Node;
	//tempNode->_ID = InterlockedIncrement((unsigned long long*)&m_NodeID);

//	g_MemoryLog.MemoryLogging((int64_t)ALLOC_MEMORY, (int64_t)GetCurrentThreadId(),-7, (int64_t)tempNode, (int64_t)m_TopCheck->_TopPtr);
	return (T*)&allocMemory->_Node;
}

template<typename T>
bool FreeList<T>::Free(T* data)
{
	Node* freeNode = (Node*)data;
	char* byteDataPtr = (char*)freeNode;

	AllocMemory* allocMemory = (AllocMemory*)(byteDataPtr - sizeof(int64_t) * 2);

	//----------------------------------------------------
	// 반납된 포인터가 언더플로우 한 경우
	//----------------------------------------------------
	if (allocMemory->_FrontMark._MarkID != m_MarkValue || allocMemory->_FrontMark._MarkValue != MARK_FRONT)
	{
		throw(FreeListException(L"Underflow Violation", __LINE__));
		return false;
	}
	//----------------------------------------------------
	// 반납된 포인터가 오버플로우 한 경우
	//----------------------------------------------------
	if (allocMemory->_RearMark._MarkID != m_MarkValue || allocMemory->_RearMark._MarkValue != MARK_REAR)
	{
		throw(FreeListException(L"Overflow Violation", __LINE__));
		return false;
	}

//	g_MemoryLog.MemoryLogging((int64_t)FREE_FLAG_CHECK, (int64_t)GetCurrentThreadId(), -7, (int64_t)freeNode, (int64_t)allocMemory->_FrontMark._FreeFlag);

	if (0 != InterlockedExchange((LONG*)&(allocMemory->_FrontMark._FreeFlag), 1))
	{
		throw(FreeListException(L"Free X 2", __LINE__));
		//wprintf(L"Free  X2\n");
		return false;
	}

	InterlockedDecrement(&m_UseCount);

	if (0 > InterlockedIncrement((LONG*)&m_PoolCount))
	{
		int a = 10;
	}

	if (m_bPlacementNew)
	{
		((T*)freeNode)->~T();
	}

	TopCheck tempTop;
	TopCheck changeValue;
	freeNode->_ID = InterlockedIncrement64(&m_NodeID);
	do
	{
		tempTop._TopPtr = m_TopCheck->_TopPtr;
		tempTop._ID = m_TopCheck->_ID;

		freeNode->_Next = tempTop._TopPtr;

		changeValue._TopPtr = freeNode;
		changeValue._ID = freeNode->_ID;

	} while (!InterlockedCompareExchange128((LONG64*)m_TopCheck, (LONG64)changeValue._ID, (LONG64)changeValue._TopPtr, (LONG64*)&tempTop));

//	g_MemoryLog.MemoryLogging((int64_t)FREE_OKAY, (int64_t)GetCurrentThreadId(), -7, (int64_t)freeNode, (int64_t)m_TopCheck->_TopPtr);

	return true;
}

template <typename T>
T* FreeList<T>::Alloc()
{

	TopCheck tempTop;
	TopCheck changeValue;

	InterlockedIncrement(&m_UseCount);

	do
	{
		tempTop._TopPtr = m_TopCheck->_TopPtr;
		tempTop._ID = m_TopCheck->_ID;

		if (tempTop._TopPtr == nullptr)
		{
			return AllocateNewMemory();
		}

		Node* nextNode = tempTop._TopPtr->_Next;

		changeValue._TopPtr = nextNode;
		if (nextNode == nullptr)
		{
			changeValue._ID = -1;
		}
		else
		{
			changeValue._ID = nextNode->_ID;
		}

	} while (!InterlockedCompareExchange128((LONG64*)m_TopCheck, (LONG64)changeValue._ID, (LONG64)changeValue._TopPtr, (LONG64*)&tempTop));


	Node* rtnNode = tempTop._TopPtr;

	//--------------------------------------------------------------------------
	// 만일, Placement New라면, 그냥 생성자만 호출해준다.
	//--------------------------------------------------------------------------
	if (m_bPlacementNew)
	{
		new(rtnNode) T;
	}


	char* byteDataPtr = (char*)rtnNode;
	AllocMemory* allocMemory = (AllocMemory*)(byteDataPtr - sizeof(int64_t) * 2);
	InterlockedExchange((LONG*)&(allocMemory->_FrontMark._FreeFlag), 0);

	InterlockedDecrement(&m_PoolCount);


//	g_MemoryLog.MemoryLogging((int64_t)ALLOC_OKAY, (int64_t)GetCurrentThreadId(), -7, (int64_t)rtnNode, (int64_t)m_TopCheck->_TopPtr);
	return &rtnNode->_Data;
}
