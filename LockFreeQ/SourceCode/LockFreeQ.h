#pragma once
#include <iostream>
#include "FreeList.h"
#include "MemoryLog.h"

extern MemoryLogging_ST<10000> g_MemoryLogQ;
extern int64_t g_MemoryCount;
extern void Crash();


template <typename T>
class LockFreeQ
{
	struct Node
	{
		T _Data;
		Node* _Next;
	};

	struct QCheck
	{
		Node* _NodePtr;
		int64_t _ID;
	};

public:
	LockFreeQ()
	{
		m_ID = 0;
		m_Count = 0;
		m_FrontCheck = new QCheck;
		m_RearCheck = new QCheck;

		//-----------------------------------------------
		// 더미노드 생성
		// Front ->  Dummy Node  <- Rear
		//-----------------------------------------------
		
		m_FrontCheck->_NodePtr = m_MemoryPool.Alloc();
		m_FrontCheck->_NodePtr->_Next = nullptr;
		m_FrontCheck->_ID = _InterlockedIncrement64(&m_ID);

		m_RearCheck->_NodePtr = m_FrontCheck->_NodePtr;
		m_RearCheck->_ID = m_FrontCheck->_ID;
	}
	~LockFreeQ()
	{
		delete m_FrontCheck;
		delete m_RearCheck;
	}

	int32_t  GetMemoryPoolAllocCount()
	{
		return m_MemoryPool.GetAllocCount();
	}
	bool EnQ(T data);
	bool DeQ(T* data);

public:
	LONG m_Count;

public:
	QCheck* m_FrontCheck;
	QCheck* m_RearCheck;
	int64_t m_ID;
	FreeList<Node> m_MemoryPool;

};

template<typename T>
inline bool LockFreeQ<T>::EnQ(T data)
{

	Q_LOG logData;
	logData.DataSettiong(InterlockedIncrement64(&g_MemoryCount),ePOS::ENTRY_ENQ, GetCurrentThreadId(), (int64_t)m_FrontCheck->_NodePtr, (int64_t)m_RearCheck->_NodePtr, -1, -1, -1, (int64_t)data, -1, -1,m_Count);
	g_MemoryLogQ.MemoryLogging(logData);

	QCheck tempRear;
	//QCheck changeValue;

	Node* newNode = m_MemoryPool.Alloc();
	//if (newNode == m_RearCheck->_NodePtr)
	//{
	//	Crash();
	//}
	newNode->_Data = data;
	newNode->_Next = newNode;
	//newNode->_Next = nullptr;
	logData.DataSettiong(InterlockedIncrement64(&g_MemoryCount), ePOS::ALLOC_NODE_ENQ, GetCurrentThreadId(), (int64_t)m_FrontCheck->_NodePtr, (int64_t)m_RearCheck->_NodePtr, -1, -1, (int64_t)newNode, (int64_t)data, (int64_t)m_RearCheck->_NodePtr->_Next, -1, m_Count);
	g_MemoryLogQ.MemoryLogging(logData);

	do
	{
		tempRear._NodePtr = m_RearCheck->_NodePtr;
		tempRear._ID = m_RearCheck->_ID;

		if (tempRear._NodePtr == newNode)
		{
			logData.DataSettiong(InterlockedIncrement64(&g_MemoryCount), ePOS::TEMPREAR_NEWNODE_SAME, GetCurrentThreadId(), (int64_t)m_FrontCheck->_NodePtr, (int64_t)m_RearCheck->_NodePtr, -1, (int64_t)tempRear._NodePtr, (int64_t)newNode, (int64_t)data, (int64_t)m_RearCheck->_NodePtr->_Next, -1, m_Count);
			g_MemoryLogQ.MemoryLogging(logData);
			Crash();
		}
		//changeValue._NodePtr = newNode;
		//changeValue._ID = InterlockedIncrement64(&m_ID);

		logData.DataSettiong(InterlockedIncrement64(&g_MemoryCount), ePOS::TEMP_REAR_SETTING_ENQ, GetCurrentThreadId(), (int64_t)m_FrontCheck->_NodePtr, (int64_t)m_RearCheck->_NodePtr, -1, (int64_t)tempRear._NodePtr, (int64_t)newNode, (int64_t)data, (int64_t)m_RearCheck->_NodePtr->_Next, -1, m_Count);
		g_MemoryLogQ.MemoryLogging(logData);

		//logData.DataSettiong(InterlockedIncrement64(&g_MemoryCount), ePOS::CHANGEVALUE_SETTING_ENQ, GetCurrentThreadId(), (int64_t)m_FrontCheck->_NodePtr, (int64_t)m_RearCheck->_NodePtr, -1, (int64_t)tempRear._NodePtr, (int64_t)newNode, (int64_t)data, (int64_t)m_RearCheck->_NodePtr->_Next, -1, m_Count);
		//g_MemoryLogQ.MemoryLogging(logData);

		//if (newNode->_Next != nullptr)
		//{
		//	Crash();
		//}
	
		//--------------------------------------------------------------
		// Commit 1
		//--------------------------------------------------------------
		if ((int64_t)nullptr != InterlockedCompareExchange64((int64_t*)&(tempRear._NodePtr->_Next), (int64_t)newNode, (int64_t)nullptr))
		{
			logData.DataSettiong(InterlockedIncrement64(&g_MemoryCount), ePOS::CAS1_FAIL_ENQ, GetCurrentThreadId(), (int64_t)m_FrontCheck->_NodePtr, (int64_t)m_RearCheck->_NodePtr, -1, (int64_t)tempRear._NodePtr, (int64_t)newNode, (int64_t)data, (int64_t)m_RearCheck->_NodePtr->_Next, -1, m_Count);
			g_MemoryLogQ.MemoryLogging(logData);
			continue;
		}	
		else
		{
			logData.DataSettiong(InterlockedIncrement64(&g_MemoryCount), ePOS::CAS1_SUC_ENQ, GetCurrentThreadId(), (int64_t)m_FrontCheck->_NodePtr, (int64_t)m_RearCheck->_NodePtr, -1, (int64_t)tempRear._NodePtr, (int64_t)newNode, (int64_t)data, (int64_t)m_RearCheck->_NodePtr->_Next, -1, m_Count);
			g_MemoryLogQ.MemoryLogging(logData);
		}

		/*if (newNode->_Next != nullptr)
		{
			logData.DataSettiong(InterlockedIncrement64(&g_MemoryCount), ePOS::NEWNODE_NEXT_NOT_NULL, GetCurrentThreadId(), (int64_t)m_FrontCheck->_NodePtr, (int64_t)m_RearCheck->_NodePtr, -1, (int64_t)tempRear._NodePtr, (int64_t)newNode, (int64_t)data, (int64_t)m_RearCheck->_NodePtr->_Next, -1, m_Count);
			g_MemoryLogQ.MemoryLogging(logData);
			Crash();
		}*/

		//m_RearCheck->_ID = changeValue._ID;
		m_RearCheck->_NodePtr = newNode;
		newNode->_Next = nullptr;
		
		logData.DataSettiong(InterlockedIncrement64(&g_MemoryCount), ePOS::CAS2_SUC_ENQ, GetCurrentThreadId(), (int64_t)m_FrontCheck->_NodePtr, (int64_t)m_RearCheck->_NodePtr, -1, (int64_t)tempRear._NodePtr, (int64_t)newNode, (int64_t)data, (int64_t)m_RearCheck->_NodePtr->_Next, -1, m_Count);
		
		//if ((int64_t)tempRear._NodePtr == InterlockedCompareExchange64((int64_t*)&(m_RearCheck->_NodePtr), (int64_t)newNode, (int64_t)tempRear._NodePtr))
		//{
		//	logData.DataSettiong(InterlockedIncrement64(&g_MemoryCount), ePOS::CAS2_SUC_ENQ, GetCurrentThreadId(), (int64_t)m_FrontCheck->_NodePtr, (int64_t)m_RearCheck->_NodePtr, -1, (int64_t)tempRear._NodePtr, (int64_t)newNode, (int64_t)data, (int64_t)m_RearCheck->_NodePtr->_Next, -1, m_Count);
		//	g_MemoryLogQ.MemoryLogging(logData);
		//	
		//}
		//else
		//{
		//	logData.DataSettiong(InterlockedIncrement64(&g_MemoryCount), ePOS::CAS2_FAIL_ENQ, GetCurrentThreadId(), (int64_t)m_FrontCheck->_NodePtr, (int64_t)m_RearCheck->_NodePtr, -1, (int64_t)tempRear._NodePtr, (int64_t)newNode, (int64_t)data, (int64_t)m_RearCheck->_NodePtr->_Next, -1, m_Count);
		//	g_MemoryLogQ.MemoryLogging(logData);
		//}

		break;
		////--------------------------------------------------------------
		//// Commit 2
		////--------------------------------------------------------------
		//BOOL result =InterlockedCompareExchange128((LONG64*)m_RearCheck, (LONG64)changeValue._ID, (LONG64)changeValue._NodePtr, (LONG64*)&tempRear);
		//if (result == FALSE)
		//{
		//	logData.DataSettiong(InterlockedIncrement64(&g_MemoryCount), ePOS::CAS2_FAIL_ENQ, GetCurrentThreadId(), (int64_t)m_FrontCheck->_NodePtr, (int64_t)m_RearCheck->_NodePtr, -1,(int64_t)tempRear._NodePtr,  (int64_t)newNode, (int64_t)data, (int64_t)m_RearCheck->_NodePtr->_Next, -1, m_Count);
		//	g_MemoryLogQ.MemoryLogging(logData);
		//	Crash();
		//}
		//else
		//{
		//	logData.DataSettiong(InterlockedIncrement64(&g_MemoryCount), ePOS::CAS2_SUC_ENQ, GetCurrentThreadId(), (int64_t)m_FrontCheck->_NodePtr, (int64_t)m_RearCheck->_NodePtr,-1, (int64_t)tempRear._NodePtr,(int64_t)newNode, (int64_t)data, (int64_t)m_RearCheck->_NodePtr->_Next, -1, m_Count);
		//	//Crash();
		//}
		//break;

	} while (true);
	logData.DataSettiong(InterlockedIncrement64(&g_MemoryCount), ePOS::CAS2_SUC_ENQ, GetCurrentThreadId(), (int64_t)m_FrontCheck->_NodePtr, (int64_t)m_RearCheck->_NodePtr, -1,(int64_t)tempRear._NodePtr, (int64_t)newNode, (int64_t)data, (int64_t)m_RearCheck->_NodePtr->_Next, -1, m_Count);
	g_MemoryLogQ.MemoryLogging(logData);
	InterlockedIncrement(&m_Count);

	return true;
}

template<typename T>
inline bool LockFreeQ<T>::DeQ(T* data)
{

	Q_LOG logData;
	QCheck tempFront;
	QCheck changeValue;
	T tempData = NULL;

	if (InterlockedDecrement(&m_Count) < 0)
	{
		InterlockedIncrement(&m_Count);
		logData.DataSettiong(InterlockedIncrement64(&g_MemoryCount), ePOS::NODE_ZERO_DEQ1, GetCurrentThreadId(), (int64_t)m_FrontCheck->_NodePtr, (int64_t)m_RearCheck->_NodePtr, -1, -1, -1, -1, -1, -1, m_Count);
		g_MemoryLogQ.MemoryLogging(logData);
		Crash();
		return false;
	}
	//if (m_FrontCheck->_NodePtr->_Next == nullptr || m_FrontCheck->_NodePtr == m_FrontCheck->_NodePtr->_Next)
	//{
	//	wprintf(L"Front Next :%p\n", m_FrontCheck->_NodePtr->_Next);
	//	wprintf(L"Front[%p] -- Front Next[%p]\n", m_FrontCheck->_NodePtr, m_FrontCheck->_NodePtr->_Next);
	//	logData.DataSettiong(InterlockedIncrement64(&g_MemoryCount), ePOS::NODE_ZERO_DEQ1, GetCurrentThreadId(), (int64_t)m_FrontCheck->_NodePtr, (int64_t)m_RearCheck->_NodePtr, -1, -1, -1, -1, -1, -1, m_Count);
	//	g_MemoryLogQ.MemoryLogging(logData);

	//	Crash();
	//	return false;
	//}

	logData.DataSettiong(InterlockedIncrement64(&g_MemoryCount), ePOS::ENTRY_DEQ, GetCurrentThreadId(), (int64_t)m_FrontCheck->_NodePtr, (int64_t)m_RearCheck->_NodePtr, -1, -1, -1, -1, -1, -1, m_Count);
	g_MemoryLogQ.MemoryLogging(logData);



	do
	{
		tempFront._NodePtr = m_FrontCheck->_NodePtr;
		tempFront._ID = m_FrontCheck->_ID;

		logData.DataSettiong(InterlockedIncrement64(&g_MemoryCount), ePOS::TEMP_FRONT_SETTING_DEQ, GetCurrentThreadId(), (int64_t)m_FrontCheck->_NodePtr, (int64_t)m_RearCheck->_NodePtr, (int64_t)tempFront._NodePtr, -1, -1, -1, -1, -1, m_Count);
		g_MemoryLogQ.MemoryLogging(logData);
		//-----------------------------
		// Front는 항상 더미 노드를 가리키고있음
		//-----------------------------
		changeValue._NodePtr = tempFront._NodePtr->_Next;
		if (changeValue._NodePtr == nullptr)
		{
			logData.DataSettiong(InterlockedIncrement64(&g_MemoryCount), ePOS::NODE_ZERO_DEQ2, GetCurrentThreadId(), (int64_t)m_FrontCheck->_NodePtr, (int64_t)m_RearCheck->_NodePtr, (int64_t)tempFront._NodePtr, -1, -1, -1, -1, -1, m_Count);
			g_MemoryLogQ.MemoryLogging(logData);
			
			changeValue._ID = -1;
			continue;
			//Crash();
			//return false;
		}
		else
		{
			changeValue._ID = InterlockedIncrement64(&m_ID);//changeValue._NodePtr->_ID;
			tempData = changeValue._NodePtr->_Data;
		}
		logData.DataSettiong(InterlockedIncrement64(&g_MemoryCount), ePOS::CHANGEVALUE_SETTING_DEQ, GetCurrentThreadId(), (int64_t)m_FrontCheck->_NodePtr, (int64_t)m_RearCheck->_NodePtr, (int64_t)tempFront._NodePtr, -1, -1, -1, -1, (int64_t)changeValue._NodePtr, m_Count);
		g_MemoryLogQ.MemoryLogging(logData);
	} while (!InterlockedCompareExchange128((LONG64*)m_FrontCheck, (LONG64)changeValue._ID, (LONG64)changeValue._NodePtr, (LONG64*)&tempFront));

	logData.DataSettiong(InterlockedIncrement64(&g_MemoryCount), ePOS::LOOP_OUT_DEQ, GetCurrentThreadId(), (int64_t)m_FrontCheck->_NodePtr, (int64_t)m_RearCheck->_NodePtr, (int64_t)tempFront._NodePtr, -1, -1, -1, -1, (int64_t)changeValue._NodePtr, m_Count);
	g_MemoryLogQ.MemoryLogging(logData);

	m_MemoryPool.Free(tempFront._NodePtr);
	logData.DataSettiong(InterlockedIncrement64(&g_MemoryCount), ePOS::FREE_NODE_DEQ, GetCurrentThreadId(), (int64_t)m_FrontCheck->_NodePtr, (int64_t)m_RearCheck->_NodePtr, (int64_t)tempFront._NodePtr, -1, -1, -1, -1, (int64_t)changeValue._NodePtr, m_Count);
	g_MemoryLogQ.MemoryLogging(logData);

	*data = tempData;
	logData.DataSettiong(InterlockedIncrement64(&g_MemoryCount), ePOS::RETURN_DATA_DEQ, GetCurrentThreadId(), (int64_t)m_FrontCheck->_NodePtr, (int64_t)m_RearCheck->_NodePtr, (int64_t)tempFront._NodePtr, -1, -1, (int64_t)changeValue._NodePtr->_Data, -1, (int64_t)changeValue._NodePtr, m_Count);
	g_MemoryLogQ.MemoryLogging(logData);

	//InterlockedDecrement(&m_Count);
	return true;
}

