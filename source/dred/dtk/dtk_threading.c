// Copyright (C) 2018 David Reid. See included LICENSE file.

#ifdef DTK_WIN32
#define DTK_THREADING_WIN32
#else
#define DTK_THREADING_POSIX
#endif


///////////////////////////////////////////////////////////////////////////////
//
// Win32
//
///////////////////////////////////////////////////////////////////////////////
#ifdef DTK_THREADING_WIN32
dtk_result dtk_thread_create__win32(dtk_thread* pThread, dtk_thread_entry_proc entryProc, void* pData)
{
    *pThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)entryProc, pData, 0, NULL);
    if (*pThread == NULL) {
        return dtk_win32_error_to_result(GetLastError());
    }

    return DTK_SUCCESS;
}

void dtk_thread_wait__win32(dtk_thread* pThread)
{
    WaitForSingleObject(*pThread, INFINITE);
}



dtk_result dtk_mutex_init__win32(dtk_mutex* pMutex)
{
    *pMutex = CreateEventA(NULL, FALSE, TRUE, NULL);
    if (*pMutex == NULL) {
        return dtk_win32_error_to_result(GetLastError());
    }

    return DTK_SUCCESS;
}

void dtk_mutex_uninit__win32(dtk_mutex* pMutex)
{
    CloseHandle(*pMutex);
}

void dtk_mutex_lock__win32(dtk_mutex* pMutex)
{
    WaitForSingleObject(*pMutex, INFINITE);
}

void dtk_mutex_unlock__win32(dtk_mutex* pMutex)
{
    SetEvent(*pMutex);
}



dtk_result dtk_semaphore_init__win32(dtk_semaphore* pSemaphore, int initialValue)
{
    *pSemaphore = CreateSemaphoreA(NULL, initialValue, LONG_MAX, NULL);
    if (*pSemaphore == NULL) {
        return dtk_win32_error_to_result(GetLastError());
    }

    return DTK_SUCCESS;
}

void dtk_semaphore_uninit__win32(dtk_semaphore* pSemaphore)
{
    CloseHandle(*pSemaphore);
}

dtk_result dtk_semaphore_wait__win32(dtk_semaphore* pSemaphore)
{
    if (WaitForSingleObject(*pSemaphore, INFINITE) != WAIT_OBJECT_0) {
        return DTK_ERROR;
    }

    return DTK_SUCCESS;
}

dtk_result dtk_semaphore_release__win32(dtk_semaphore* pSemaphore)
{
    if (ReleaseSemaphore(*pSemaphore, 1, NULL) == 0) {
        return DTK_ERROR;
    }

    return DTK_SUCCESS;
}
#endif  // Win32


///////////////////////////////////////////////////////////////////////////////
//
// Posix
//
///////////////////////////////////////////////////////////////////////////////
#ifdef DTK_THREADING_POSIX
dtk_result dtk_thread_create__posix(dtk_thread* pThread, dtk_thread_entry_proc entryProc, void* pData)
{
    if (pthread_create(pThread, NULL, entryProc, pData) != 0) {
        return DTK_ERROR;
    }

    return DTK_SUCCESS;
}

void dtk_thread_wait__posix(dtk_thread* pThread)
{
    pthread_join(*pThread, NULL);
}



dtk_result dtk_mutex_init__posix(dtk_mutex* pMutex)
{
    if (pthread_mutex_init(pMutex, NULL) != 0) {
        return DTK_ERROR;
    }

    return DTK_SUCCESS;
}

void dtk_mutex_uninit__posix(dtk_mutex* pMutex)
{
    pthread_mutex_destroy(pMutex);
}

void dtk_mutex_lock__posix(dtk_mutex* pMutex)
{
    pthread_mutex_lock(pMutex);
}

void dtk_mutex_unlock__posix(dtk_mutex* pMutex)
{
    pthread_mutex_unlock(pMutex);
}



dtk_result dtk_semaphore_init__posix(dtk_semaphore* pSemaphore, int initialValue)
{
    if (sem_init(pSemaphore, 0, (unsigned int)initialValue) == -1) {
        return dtk_errno_to_result(errno);
    }

    return DTK_SUCCESS;
}

void dtk_semaphore_uninit__posix(dtk_semaphore* pSemaphore)
{
    sem_close(pSemaphore);
}

dtk_result dtk_semaphore_wait__posix(dtk_semaphore* pSemaphore)
{
    if (sem_wait(pSemaphore) == -1) {
        return dtk_errno_to_result(errno);
    }

    return DTK_SUCCESS;
}

dtk_result dtk_semaphore_release__posix(dtk_semaphore* pSemaphore)
{
    if (sem_post(pSemaphore) == -1) {
        return dtk_errno_to_result(errno);
    }

    return DTK_SUCCESS;
}
#endif  // Posix




//// Thread ////

dtk_result dtk_thread_create(dtk_thread* pThread, dtk_thread_entry_proc entryProc, void* pData)
{
    if (pThread == NULL || entryProc == NULL) {
        return DTK_INVALID_ARGS;
    }

#ifdef DTK_THREADING_WIN32
    return dtk_thread_create__win32(pThread, entryProc, pData);
#endif
#ifdef DTK_THREADING_POSIX
    return dtk_thread_create__posix(pThread, entryProc, pData);
#endif
}

void dtk_thread_wait(dtk_thread* pThread)
{
    if (pThread == NULL) {
        return;
    }

#ifdef DTK_THREADING_WIN32
    dtk_thread_wait__win32(pThread);
#endif
#ifdef DTK_THREADING_POSIX
    dtk_thread_wait__posix(pThread);
#endif
}


//// Mutex ////

dtk_result dtk_mutex_init(dtk_mutex* pMutex)
{
    if (pMutex == NULL) {
        return DTK_INVALID_ARGS;
    }

#ifdef DTK_THREADING_WIN32
    return dtk_mutex_init__win32(pMutex);
#endif
#ifdef DTK_THREADING_POSIX
    return dtk_mutex_init__posix(pMutex);
#endif
}

void dtk_mutex_uninit(dtk_mutex* pMutex)
{
    if (pMutex == NULL) {
        return;
    }

#ifdef DTK_THREADING_WIN32
    dtk_mutex_uninit__win32(pMutex);
#endif
#ifdef DTK_THREADING_POSIX
    dtk_mutex_uninit__posix(pMutex);
#endif
}

void dtk_mutex_lock(dtk_mutex* pMutex)
{
    if (pMutex == NULL) {
        return;
    }

#ifdef DTK_THREADING_WIN32
    dtk_mutex_lock__win32(pMutex);
#endif
#ifdef DTK_THREADING_POSIX
    dtk_mutex_lock__posix(pMutex);
#endif
}

void dtk_mutex_unlock(dtk_mutex* pMutex)
{
    if (pMutex == NULL) {
        return;
    }

#ifdef DTK_THREADING_WIN32
    dtk_mutex_unlock__win32(pMutex);
#endif
#ifdef DTK_THREADING_POSIX
    dtk_mutex_unlock__posix(pMutex);
#endif
}


//// Semaphore ///

dtk_result dtk_semaphore_init(dtk_semaphore* pSemaphore, int initialValue)
{
    if (pSemaphore == NULL) {
        return DTK_INVALID_ARGS;
    }

#ifdef DTK_THREADING_WIN32
    return dtk_semaphore_init__win32(pSemaphore, initialValue);
#endif
#ifdef DTK_THREADING_POSIX
    return dtk_semaphore_init__posix(pSemaphore, initialValue);
#endif
}

void dtk_semaphore_uninit(dtk_semaphore* pSemaphore)
{
    if (pSemaphore == NULL) {
        return;
    }

#ifdef DTK_THREADING_WIN32
    dtk_semaphore_uninit__win32(pSemaphore);
#endif
#ifdef DTK_THREADING_POSIX
    dtk_semaphore_uninit__posix(pSemaphore);
#endif
}

dtk_result dtk_semaphore_wait(dtk_semaphore* pSemaphore)
{
    if (pSemaphore == NULL) {
        return DTK_INVALID_ARGS;
    }

#ifdef DTK_THREADING_WIN32
    return dtk_semaphore_wait__win32(pSemaphore);
#endif
#ifdef DTK_THREADING_POSIX
    return dtk_semaphore_wait__posix(pSemaphore);
#endif
}

dtk_result dtk_semaphore_release(dtk_semaphore* pSemaphore)
{
    if (pSemaphore == NULL) {
        return DTK_INVALID_ARGS;
    }

#ifdef DTK_THREADING_WIN32
    return dtk_semaphore_release__win32(pSemaphore);
#endif
#ifdef DTK_THREADING_POSIX
    return dtk_semaphore_release__posix(pSemaphore);
#endif
}