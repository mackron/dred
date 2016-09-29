// Copyright (C) 2016 David Reid. See included LICENSE file.

// Multithreading is implemented differently depending on the platform. Of note is the entry point which is slightly different. The thread
// entry point needs to be declared like this:
//     dred_thread_result DRED_THREADCALL MyThreadEntryProc(void* pData);

#ifdef DRED_WIN32
#define DRED_THREADCALL WINAPI
typedef DWORD dred_thread_result;
typedef HANDLE dred_thread;
typedef HANDLE dred_mutex;
typedef HANDLE dred_semaphore;
#else
#define DRED_THREADCALL
typedef void* dred_thread_result;
typedef pthread_t dred_thread;
typedef pthread_mutex_t dred_mutex;
typedef sem_t dred_semaphore;
#endif
typedef dred_thread_result (DRED_THREADCALL * dred_thread_entry_proc)(void* pData);



//// Thread ////

// Starts a new thread.
//
// There is no delete function. Threads should be terminated naturally.
drBool32 dred_thread_create(dred_thread* pThread, dred_thread_entry_proc entryProc, void* pData);

// Waits for a thread to return.
void dred_thread_wait(dred_thread* pThread);


//// Mutex ////

// Creates a mutex.
drBool32 dred_mutex_create(dred_mutex* pMutex);

// Deletes a mutex.
void dred_mutex_delete(dred_mutex* pMutex);

// Locks a mutex.
void dred_mutex_lock(dred_mutex* pMutex);

// Unlocks a mutex.
void dred_mutex_unlock(dred_mutex* pMutex);


//// Semaphore ///

// Creates a semaphore.
drBool32 dred_semaphore_create(dred_semaphore* pSemaphore, int initialValue);

// Delets a semaphore.
void dred_semaphore_delete(dred_semaphore* pSemaphore);

// Waits on the given semaphore object and decrements it's counter by one upon returning.
drBool32 dred_semaphore_wait(dred_semaphore* pSemaphore);

// Releases the given semaphore and increments it's counter by one upon returning.
drBool32 dred_semaphore_release(dred_semaphore* pSemaphore);