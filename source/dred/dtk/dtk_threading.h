// Copyright (C) 2018 David Reid. See included LICENSE file.

// Multithreading is implemented differently depending on the platform. Of note is the entry point which is slightly different. The thread
// entry point needs to be declared like this:
//     dtk_thread_result DTK_THREADCALL MyThreadEntryProc(void* pData);

#ifdef DTK_WIN32
#ifdef WINAPI
#define DTK_THREADCALL WINAPI
#else
#define DTK_THREADCALL __stdcall
#endif
typedef /*DWORD*/ dtk_uint32 dtk_thread_result;
typedef /*HANDLE*/ dtk_handle dtk_thread;
typedef /*HANDLE*/ dtk_handle dtk_mutex;
typedef /*HANDLE*/ dtk_handle dtk_semaphore;
#else
#define DTK_THREADCALL
typedef void*           dtk_thread_result;
typedef pthread_t       dtk_thread;
typedef pthread_mutex_t dtk_mutex;
typedef sem_t           dtk_semaphore;
#endif
typedef dtk_thread_result (DTK_THREADCALL * dtk_thread_entry_proc)(void* pData);



//// Thread ////

// Starts a new thread.
//
// There is no delete function. Threads should be terminated naturally.
dtk_result dtk_thread_create(dtk_thread* pThread, dtk_thread_entry_proc entryProc, void* pData);

// Waits for a thread to return.
void dtk_thread_wait(dtk_thread* pThread);


//// Mutex ////

// Creates a mutex.
dtk_result dtk_mutex_init(dtk_mutex* pMutex);

// Deletes a mutex.
void dtk_mutex_uninit(dtk_mutex* pMutex);

// Locks a mutex.
void dtk_mutex_lock(dtk_mutex* pMutex);

// Unlocks a mutex.
void dtk_mutex_unlock(dtk_mutex* pMutex);


//// Semaphore ////

// Creates a semaphore.
dtk_result dtk_semaphore_init(dtk_semaphore* pSemaphore, int initialValue);

// Delets a semaphore.
void dtk_semaphore_uninit(dtk_semaphore* pSemaphore);

// Waits on the given semaphore object and decrements it's counter by one upon returning.
dtk_result dtk_semaphore_wait(dtk_semaphore* pSemaphore);

// Releases the given semaphore and increments it's counter by one upon returning.
dtk_result dtk_semaphore_release(dtk_semaphore* pSemaphore);
