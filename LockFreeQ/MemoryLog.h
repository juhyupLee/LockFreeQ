#pragma once
#include <Windows.h>
#include <iostream>

enum class ePOS
{
	ENTRY_ENQ,
	ALLOC_NODE_ENQ,
	TEMP_REAR_SETTING_ENQ,
	CHANGEVALUE_SETTING_ENQ,
	CAS1_FAIL_ENQ,
	CAS1_SUC_ENQ,
	CAS2_FAIL_ENQ,
	CAS2_SUC_ENQ,
	ENTRY_DEQ,
	NODE_ZERO_DEQ1,
	NODE_ZERO_DEQ2,
	TEMP_FRONT_SETTING_DEQ,
	CHANGEVALUE_SETTING_DEQ,
	LOOP_OUT_DEQ,
	FREE_NODE_DEQ,
	RETURN_DATA_DEQ,
	TEMPREAR_NEWNODE_SAME,
	NEWNODE_NEXT_NOT_NULL


};
struct Q_LOG
{
	int64_t _No;
	ePOS _POS;
	DWORD _ThreadID;
	int64_t _FrontPtr;
	int64_t _RearPtr;
	int64_t _TempFrontPtr;
	int64_t _TempRearPtr;
	int64_t _NewNodePtr;
	int64_t _Data;
	int64_t _RearNext;
	int64_t _CurDummyNode;
	LONG _Count;

	void DataSettiong(int64_t no, ePOS pos, DWORD threadid, int64_t frontPtr, int64_t rearPtr, int64_t _tempFrontPtr, int64_t tempRearPtr, int64_t newNodePtr, int64_t data, int64_t rearNext, int64_t curDummy, LONG count=-1)
	{
		_No = no;
		_POS = pos;
		_ThreadID = threadid;
		_FrontPtr = frontPtr;
		_RearPtr = rearPtr;
		_TempFrontPtr = _tempFrontPtr;
		_TempRearPtr = tempRearPtr;
		_NewNodePtr = newNodePtr;
		_Data = data;
		_RearNext = rearNext;
		_CurDummyNode = curDummy;
		_Count = count;
	}
};

template <size_t size = 5000 >
class MemoryLogging_ST
{

public:
	enum
	{
		USE_LOG = 1,
		MAX_SIZE = size
	};
public:
	MemoryLogging_ST()
		:m_Index(0),
		m_Count(0),
		m_Buffer{0,}
	{
	}

public:
	void MemoryLogging(Q_LOG logData);
	void Clear();

private:
	Q_LOG  m_Buffer[MAX_SIZE];
	uint64_t m_Index;
	uint64_t m_Count;
};

template<size_t size>
inline void MemoryLogging_ST<size>::MemoryLogging(Q_LOG logData)
{
	if (!USE_LOG)
	{
		return;
	}
	uint64_t tempIndex = InterlockedIncrement(&m_Index) % MAX_SIZE;
	ZeroMemory(&m_Buffer[tempIndex],  sizeof(Q_LOG));
	m_Buffer[tempIndex] = logData;

}

template<size_t size>
inline void MemoryLogging_ST<size>::Clear()
{
	memset(m_Buffer, 0, sizeof(Q_LOG) * MAX_SIZE);
	m_Index = 0;
	m_Count = 0;
}

//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

template <typename T,size_t size=5000 >
class MyMemoryLog
{
public:
	enum
	{
		USE_LOG = 1,
		MAX_SIZE = size,
		MAX_PROPERTY = 10
	};
public:
	MyMemoryLog()
		:m_Index(0),
		 m_Count(0)
	{
		memset(m_Buffer, 0, sizeof(T) * MAX_SIZE * MAX_PROPERTY);
	}

public:
	void MemoryLogging(T pos, T threadID, T data);
	void MemoryLogging(T pos, T threadID, T sessionID, T data);
	void MemoryLogging(T pos, T threadID, T sessionID, T data1, T data2);
	void MemoryLogging(T pos, T threadID, T sessionID, T data1, T data2, T data3);
	void MemoryLogging(T pos, T threadID, T sessionID, T data1, T data2, T data3, T data4);
	void MemoryLogging(T pos, T threadID, T sessionID, T data1, T data2, T data3, T data4, T data5);
	void MemoryLogging(T pos, T threadID, T sessionID, T data1, T data2, T data3, T data4, T data5, T data6);

	void Clear();

private:
	T  m_Buffer[MAX_SIZE][MAX_PROPERTY];
	uint64_t m_Index;
	uint64_t m_Count;
};

template<typename T, size_t size>
inline void MyMemoryLog<T, size>::MemoryLogging(T pos, T threadID, T data)
{
	if (!USE_LOG)
	{
		return;
	}
	uint64_t tempIndex = InterlockedIncrement(&m_Index) % MAX_SIZE;
	ZeroMemory(m_Buffer[tempIndex], MAX_PROPERTY * sizeof(T));
	m_Buffer[tempIndex][0] = (T)InterlockedIncrement(&m_Count);
	m_Buffer[tempIndex][1] = threadID;
	m_Buffer[tempIndex][2] = pos;
	m_Buffer[tempIndex][3] = data;
}

template <typename T, size_t size >
inline void MyMemoryLog<T,size>::MemoryLogging(T pos, T threadID, T sessionID, T data)
{
	if (!USE_LOG)
	{
		return;
	}
	uint64_t tempIndex = InterlockedIncrement(&m_Index) % MAX_SIZE;
	ZeroMemory(m_Buffer[tempIndex], MAX_PROPERTY * sizeof(T));
	m_Buffer[tempIndex][0] = (T)InterlockedIncrement(&m_Count);
	m_Buffer[tempIndex][1] = sessionID;
	m_Buffer[tempIndex][2] = threadID;
	m_Buffer[tempIndex][3] = pos;
	m_Buffer[tempIndex][4] = data;
}

template <typename T, size_t size >
inline void MyMemoryLog<T,size>::MemoryLogging(T pos, T threadID, T sessionID, T data1, T data2)
{
	if (!USE_LOG)
	{
		return;
	}
	uint64_t tempIndex = InterlockedIncrement(&m_Index) % MAX_SIZE;
	ZeroMemory(m_Buffer[tempIndex], MAX_PROPERTY * sizeof(T));

	m_Buffer[tempIndex][0] = (T)InterlockedIncrement(&m_Count);
	m_Buffer[tempIndex][1] = sessionID;
	m_Buffer[tempIndex][2] = threadID;
	m_Buffer[tempIndex][3] = pos;
	m_Buffer[tempIndex][4] = data1;
	m_Buffer[tempIndex][5] = data2;
}

template <typename T, size_t size >
inline void MyMemoryLog<T,size>::MemoryLogging(T pos, T threadID, T sessionID, T data1, T data2, T data3)
{
	if (!USE_LOG)
	{
		return;
	}
	uint64_t tempIndex = InterlockedIncrement(&m_Index) % MAX_SIZE;

	ZeroMemory(m_Buffer[tempIndex], MAX_PROPERTY * sizeof(T));

	m_Buffer[tempIndex][0] = (T)InterlockedIncrement(&m_Count);
	m_Buffer[tempIndex][1] = sessionID;
	m_Buffer[tempIndex][2] = threadID;
	m_Buffer[tempIndex][3] = pos;
	m_Buffer[tempIndex][4] = data1;
	m_Buffer[tempIndex][5] = data2;
	m_Buffer[tempIndex][6] = data3;
}

template <typename T, size_t size >
inline void MyMemoryLog<T,size>::MemoryLogging(T pos, T threadID, T sessionID, T data1, T data2, T data3, T data4)
{
	if (!USE_LOG)
	{
		return;
	}
	uint64_t tempIndex = InterlockedIncrement(&m_Index) % MAX_SIZE;

	ZeroMemory(m_Buffer[tempIndex], MAX_PROPERTY * sizeof(T));

	m_Buffer[tempIndex][0] = (T)InterlockedIncrement(&m_Count);
	m_Buffer[tempIndex][1] = sessionID;
	m_Buffer[tempIndex][2] = threadID;
	m_Buffer[tempIndex][3] = pos;
	m_Buffer[tempIndex][4] = data1;
	m_Buffer[tempIndex][5] = data2;
	m_Buffer[tempIndex][6] = data3;
	m_Buffer[tempIndex][7] = data4;
}

template <typename T, size_t size >
inline void MyMemoryLog<T,size>::MemoryLogging(T pos, T threadID, T sessionID, T data1, T data2, T data3, T data4, T data5)
{
	if (!USE_LOG)
	{
		return;
	}
	uint64_t tempIndex = InterlockedIncrement(&m_Index) % MAX_SIZE;

	ZeroMemory(m_Buffer[tempIndex], MAX_PROPERTY * sizeof(T));

	m_Buffer[tempIndex][0] = (T)InterlockedIncrement(&m_Count);
	m_Buffer[tempIndex][1] = sessionID;
	m_Buffer[tempIndex][2] = threadID;
	m_Buffer[tempIndex][3] = pos;
	m_Buffer[tempIndex][4] = data1;
	m_Buffer[tempIndex][5] = data2;
	m_Buffer[tempIndex][6] = data3;
	m_Buffer[tempIndex][7] = data4;
	m_Buffer[tempIndex][8] = data5;
}

template <typename T, size_t size >
inline void MyMemoryLog<T,size>::MemoryLogging(T pos, T threadID, T sessionID, T data1, T data2, T data3, T data4, T data5, T data6)
{
	if (!USE_LOG)
	{
		return;
	}
	uint64_t tempIndex = InterlockedIncrement(&m_Index) % MAX_SIZE;

	ZeroMemory(m_Buffer[tempIndex], MAX_PROPERTY * sizeof(T));

	m_Buffer[tempIndex][0] = (T)InterlockedIncrement(&m_Count);
	m_Buffer[tempIndex][1] = sessionID;
	m_Buffer[tempIndex][2] = threadID;
	m_Buffer[tempIndex][3] = pos;
	m_Buffer[tempIndex][4] = data1;
	m_Buffer[tempIndex][5] = data2;
	m_Buffer[tempIndex][6] = data3;
	m_Buffer[tempIndex][7] = data4;
	m_Buffer[tempIndex][8] = data5;
	m_Buffer[tempIndex][9] = data6;
}

template<typename T, size_t size>
inline void MyMemoryLog<T, size>::Clear()
{
	memset(m_Buffer, 0, sizeof(T) * MAX_SIZE );	
	m_Index = 0;
	m_Count = 0;
}

