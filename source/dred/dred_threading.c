// Copyright (C) 2016 David Reid. See included LICENSE file.

#ifdef DRED_WIN32
#define DRED_THREADING_WIN32
#else
#define DRED_THREADING_POSIX
#endif


///////////////////////////////////////////////////////////////////////////////
//
// Win32
//
///////////////////////////////////////////////////////////////////////////////
#ifdef DRED_THREADING_WIN32
dr_bool32 dred_thread_create__win32(dred_thread* pThread, dred_thread_entry_proc entryProc, void* pData)
{
    *pThread = CreateThread(NULL, 0, entryProc, pData, 0, NULL);
    if (*pThread == NULL) {
        return DR_FALSE;
    }

    return DR_TRUE;
}

void dred_thread_wait__win32(dred_thread* pThread)
{
    WaitForSingleObject(*pThread, INFINITE);
}



dr_bool32 dred_mutex_create__win32(dred_mutex* pMutex)
{
    *pMutex = CreateEventA(NULL, FALSE, TRUE, NULL);
    if (*pMutex == NULL) {
        return DR_FALSE;
    }

    return DR_TRUE;
}

void dred_mutex_delete__win32(dred_mutex* pMutex)
{
    CloseHandle(*pMutex);
}

void dred_mutex_lock__win32(dred_mutex* pMutex)
{
    WaitForSingleObject(*pMutex, INFINITE);
}

void dred_mutex_unlock__win32(dred_mutex* pMutex)
{
    SetEvent(*pMutex);
}



dr_bool32 dred_semaphore_create__win32(dred_semaphore* pSemaphore, int initialValue)
{
    *pSemaphore = CreateSemaphoreA(NULL, initialValue, LONG_MAX, NULL);
    if (*pSemaphore == NULL) {
        return DR_FALSE;
    }

    return DR_TRUE;
}

void dred_semaphore_delete__win32(dred_semaphore* pSemaphore)
{
    CloseHandle(*pSemaphore);
}

dr_bool32 dred_semaphore_wait__win32(dred_semaphore* pSemaphore)
{
    return WaitForSingleObject(*pSemaphore, INFINITE) == WAIT_OBJECT_0;
}

dr_bool32 dred_semaphore_release__win32(dred_semaphore* pSemaphore)
{
    return ReleaseSemaphore(*pSemaphore, 1, NULL) != 0;
}
#endif  // Win32


///////////////////////////////////////////////////////////////////////////////
//
// Posix
//
///////////////////////////////////////////////////////////////////////////////
#ifdef DRED_THREADING_POSIX
dr_bool32 dred_thread_create__posix(dred_thread* pThread, dred_thread_entry_proc entryProc, void* pData)
{
    return pthread_create(pThread, NULL, entryProc, pData) == 0;
}

void dred_thread_wait__posix(dred_thread* pThread)
{
    pthread_join(*pThread, NULL);
}



dr_bool32 dred_mutex_create__posix(dred_mutex* pMutex)
{
    return pthread_mutex_init(pMutex, NULL) == 0;
}

void dred_mutex_delete__posix(dred_mutex* pMutex)
{
    pthread_mutex_destroy(pMutex);
}

void dred_mutex_lock__posix(dred_mutex* pMutex)
{
    pthread_mutex_lock(pMutex);
}

void dred_mutex_unlock__posix(dred_mutex* pMutex)
{
    pthread_mutex_unlock(pMutex);
}



dr_bool32 dred_semaphore_create__posix(dred_semaphore* pSemaphore, int initialValue)
{
    return sem_init(pSemaphore, 0, (unsigned int)initialValue) != -1;
}

void dred_semaphore_delete__posix(dred_semaphore* pSemaphore)
{
    sem_close(pSemaphore);
}

dr_bool32 dred_semaphore_wait__posix(dred_semaphore* pSemaphore)
{
    return sem_wait(pSemaphore) != -1;
}

dr_bool32 dred_semaphore_release__posix(dred_semaphore* pSemaphore)
{
    return sem_post(pSemaphore) != -1;
}
#endif  // Posix




//// Thread ////

dr_bool32 dred_thread_create(dred_thread* pThread, dred_thread_entry_proc entryProc, void* pData)
{
    if (pThread == NULL || entryProc == NULL) {
        return DR_FALSE;
    }

#ifdef DRED_THREADING_WIN32
    return dred_thread_create__win32(pThread, entryProc, pData);
#endif

#ifdef DRED_THREADING_POSIX
    return dred_thread_create__posix(pThread, entryProc, pData);
#endif
}

void dred_thread_wait(dred_thread* pThread)
{
    if (pThread == NULL) {
        return;
    }

#ifdef DRED_THREADING_WIN32
    dred_thread_wait__win32(pThread);
#endif

#ifdef DRED_THREADING_POSIX
    dred_thread_wait__posix(pThread);
#endif
}


//// Mutex ////

dr_bool32 dred_mutex_create(dred_mutex* pMutex)
{
    if (pMutex == NULL) {
        return DR_FALSE;
    }

#ifdef DRED_THREADING_WIN32
    return dred_mutex_create__win32(pMutex);
#endif

#ifdef DRED_THREADING_POSIX
    return dred_mutex_create__posix(pMutex);
#endif
}

void dred_mutex_delete(dred_mutex* pMutex)
{
    if (pMutex == NULL) {
        return;
    }

#ifdef DRED_THREADING_WIN32
    dred_mutex_delete__win32(pMutex);
#endif

#ifdef DRED_THREADING_POSIX
    dred_mutex_delete__posix(pMutex);
#endif
}

void dred_mutex_lock(dred_mutex* pMutex)
{
    if (pMutex == NULL) {
        return;
    }

#ifdef DRED_THREADING_WIN32
    dred_mutex_lock__win32(pMutex);
#endif

#ifdef DRED_THREADING_POSIX
    dred_mutex_lock__posix(pMutex);
#endif
}

void dred_mutex_unlock(dred_mutex* pMutex)
{
    if (pMutex == NULL) {
        return;
    }

#ifdef DRED_THREADING_WIN32
    dred_mutex_unlock__win32(pMutex);
#endif

#ifdef DRED_THREADING_POSIX
    dred_mutex_unlock__posix(pMutex);
#endif
}


//// Semaphore ///

dr_bool32 dred_semaphore_create(dred_semaphore* pSemaphore, int initialValue)
{
    if (pSemaphore == NULL) {
        return DR_FALSE;
    }

#ifdef DRED_THREADING_WIN32
    return dred_semaphore_create__win32(pSemaphore, initialValue);
#endif

#ifdef DRED_THREADING_POSIX
    return dred_semaphore_create__posix(pSemaphore, initialValue);
#endif
}

void dred_semaphore_delete(dred_semaphore* pSemaphore)
{
    if (pSemaphore == NULL) {
        return;
    }

#ifdef DRED_THREADING_WIN32
    dred_semaphore_delete__win32(pSemaphore);
#endif

#ifdef DRED_THREADING_POSIX
    dred_semaphore_delete__posix(pSemaphore);
#endif
}

dr_bool32 dred_semaphore_wait(dred_semaphore* pSemaphore)
{
    if (pSemaphore == NULL) {
        return DR_FALSE;
    }

#ifdef DRED_THREADING_WIN32
    return dred_semaphore_wait__win32(pSemaphore);
#endif

#ifdef DRED_THREADING_POSIX
    return dred_semaphore_wait__posix(pSemaphore);
#endif
}

dr_bool32 dred_semaphore_release(dred_semaphore* pSemaphore)
{
    if (pSemaphore == NULL) {
        return DR_FALSE;
    }

#ifdef DRED_THREADING_WIN32
    return dred_semaphore_release__win32(pSemaphore);
#endif

#ifdef DRED_THREADING_POSIX
    return dred_semaphore_release__posix(pSemaphore);
#endif
}
