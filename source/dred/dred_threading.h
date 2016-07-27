// Copyright (C) 2016 David Reid. See included LICENSE file.

// Multithreading is implemented differently depending on the platform. Of note is the entry point which is slightly different. This means
// every thread entry point needs a simple #ifdef check to use the correct signature. Use DRED_THREAD_PROC_SIGNATURE to help with this.

#ifdef DRED_WIN32
typedef DWORD (WINAPI * dred_thread_entry_proc)(void* pData);
typedef HANDLE dred_thread;
typedef HANDLE dred_mutex;
typedef HANDLE dred_semaphore;

#define DRED_THREAD_PROC_SIGNATURE(name, data) DWORD name(void* data)
#else
typedef void* (* dred_thread_entry_proc)(void* pData);
typedef pthread_t dred_thread;
typedef pthread_mutex_t dred_mutex;
typedef sem_t dred_semaphore;

#define DRED_THREAD_PROC_SIGNATURE(name, data) void* name(void* data)
#endif


//// Thread ////

// Starts a new thread.
//
// There is no delete function. Threads should be terminated naturally.
bool dred_thread_create(dred_thread* pThread, dred_thread_entry_proc entryProc, void* pData);

// Waits for a thread to return.
void dred_thread_wait(dred_thread* pThread);


//// Mutex ////

// Creates a mutex.
bool dred_mutex_create(dred_mutex* pMutex);

// Deletes a mutex.
void dred_mutex_delete(dred_mutex* pMutex);

// Locks a mutex.
void dred_mutex_lock(dred_mutex* pMutex);

// Unlocks a mutex.
void dred_mutex_unlock(dred_mutex* pMutex);


//// Semaphore ///

// Creates a semaphore.
bool dred_semaphore_create(dred_semaphore* pSemaphore, int initialValue);

// Delets a semaphore.
void dred_semaphore_delete(dred_semaphore* pSemaphore);

// Waits on the given semaphore object and decrements it's counter by one upon returning.
bool dred_semaphore_wait(dred_semaphore* pSemaphore);

// Releases the given semaphore and increments it's counter by one upon returning.
bool dred_semaphore_release(dred_semaphore* pSemaphore);