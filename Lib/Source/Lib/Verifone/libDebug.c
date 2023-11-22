#ifdef __VERIFONE_SRC__

#include <SVC.H>
#include "libMtiCommonApi.h"

#ifdef __DEBUG_ON__
static sem_t g_stLogLock;

void debugLogInit()
{
	sem_init(&g_stLogLock, 1);
}

void debugLogLock()
{
	sem_wait(&g_stLogLock);
}

void debugLogUnlock()
{
	sem_post(&g_stLogLock);
}

#else
void debugLogInit()
{
}

void debugLogLock()
{
}

void debugLogUnlock()
{
}
#endif // __DEBUG_ON__

#endif // __VERIFONE_SRC__
