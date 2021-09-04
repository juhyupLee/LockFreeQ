#include "MemoryLog.h"
MemoryLogging_New<Q_LOG, 10000> g_MemoryLogQ;
int64_t g_MemoryCount = 0;

void Crash()
{
	int* p = nullptr;
	*p = 20;
}