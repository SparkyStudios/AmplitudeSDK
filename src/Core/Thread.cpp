// Copyright (c) 2021-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#include <processthreadsapi.h>
#else
#include <inttypes.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#endif

#include <SparkyStudios/Audio/Amplitude/Core/Thread.h>

namespace SparkyStudios::Audio::Amplitude::Thread
{
#ifdef AM_WINDOWS_VERSION
    struct AmThreadHandleData
    {
        HANDLE thread;
    };

    AmVoidPtr CreateMutexAm()
    {
        auto* cs = new CRITICAL_SECTION;
        InitializeCriticalSectionAndSpinCount(cs, 100);
        return (AmVoidPtr)cs;
    }

    void DestroyMutex(AmVoidPtr handle)
    {
        auto* cs = (CRITICAL_SECTION*)handle;
        DeleteCriticalSection(cs);
        delete cs;
    }

    void LockMutex(AmVoidPtr handle)
    {
        auto* cs = (CRITICAL_SECTION*)handle;
        if (cs)
        {
            EnterCriticalSection(cs);
        }
    }

    void UnlockMutex(AmVoidPtr handle)
    {
        auto* cs = (CRITICAL_SECTION*)handle;
        if (cs)
        {
            LeaveCriticalSection(cs);
        }
    }

    struct AmThreadData
    {
        AmThreadFunction mFunc;
        AmVoidPtr mParam;
    };

    static DWORD WINAPI threadfunc(LPVOID d)
    {
        auto p = (AmThreadData*)d;
        p->mFunc(p->mParam);
        delete p;
        return 0;
    }

    AmThreadHandle CreateThread(AmThreadFunction threadFunction, AmVoidPtr parameter)
    {
        auto* d = new AmThreadData;
        d->mFunc = threadFunction;
        d->mParam = parameter;
        HANDLE h = ::CreateThread(nullptr, 0, threadfunc, (LPVOID)d, 0, nullptr);
        if (nullptr == h)
        {
            return nullptr;
        }
        auto* threadHandle = new AmThreadHandleData;
        threadHandle->thread = h;
        return threadHandle;
    }

    void Sleep(int milliseconds)
    {
        ::Sleep(milliseconds);
    }

    void Wait(AmThreadHandle threadHandle)
    {
        WaitForSingleObject(threadHandle->thread, INFINITE);
    }

    void Release(AmThreadHandle threadHandle)
    {
        CloseHandle(threadHandle->thread);
        delete threadHandle;
    }

    int GetTimeMillis()
    {
        return GetTickCount();
    }

#else // pthreads
    struct AmThreadHandleData
    {
        pthread_t thread;
    };

    AmVoidPtr CreateMutex()
    {
        pthread_mutex_t* mutex;
        mutex = new pthread_mutex_t;

        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);

        pthread_mutex_init(mutex, &attr);

        return (AmVoidPtr)mutex;
    }

    void destroyMutex(AmVoidPtr handle)
    {
        pthread_mutex_t* mutex = (pthread_mutex_t*)handle;

        if (mutex)
        {
            pthread_mutex_destroy(mutex);
            delete mutex;
        }
    }

    void lockMutex(AmVoidPtr handle)
    {
        pthread_mutex_t* mutex = (pthread_mutex_t*)handle;
        if (mutex)
        {
            pthread_mutex_lock(mutex);
        }
    }

    void unlockMutex(AmVoidPtr handle)
    {
        pthread_mutex_t* mutex = (pthread_mutex_t*)handle;
        if (mutex)
        {
            pthread_mutex_unlock(mutex);
        }
    }

    struct AmThreadData
    {
        threadFunction mFunc;
        AmVoidPtr mParam;
    };

    static AmVoidPtr threadfunc(AmVoidPtr d)
    {
        AmThreadData* p = (AmThreadData*)d;
        p->mFunc(p->mParam);
        delete p;
        return 0;
    }

    AmThreadHandle createThread(threadFunction threadFunction, AmVoidPtr parameter)
    {
        AmThreadData* d = new AmThreadData;
        d->mFunc = threadFunction;
        d->mParam = parameter;

        AmThreadHandleData* threadHandle = new AmThreadHandleData;
        pthread_create(&threadHandle->thread, NULL, threadfunc, (AmVoidPtr)d);
        return threadHandle;
    }

    void sleep(int milliseconds)
    {
        // usleep(milliseconds * 1000);
        struct timespec req = { 0 };
        req.tv_sec = 0;
        req.tv_nsec = milliseconds * 1000000L;
        nanosleep(&req, (struct timespec*)NULL);
    }

    void wait(AmThreadHandle threadHandle)
    {
        pthread_join(threadHandle->thread, 0);
    }

    void release(AmThreadHandle threadHandle)
    {
        delete threadHandle;
    }

    int getTimeMillis()
    {
        struct timespec spec;
        clock_gettime(CLOCK_REALTIME, &spec);
        return spec.tv_sec * 1000 + (int)(spec.tv_nsec / 1.0e6);
    }
#endif

    static void poolWorker(AmVoidPtr param)
    {
        Pool* myPool = (Pool*)param;
        while (myPool->mRunning)
        {
            PoolTask* t = myPool->GetWork();
            if (!t)
            {
                Sleep(1);
            }
            else
            {
                t->Work();
            }
        }
    }

    Pool::Pool()
    {
        mRunning = 0;
        mThreadCount = 0;
        mThread = nullptr;
        mWorkMutex = nullptr;
        mRobin = 0;
        mMaxTask = 0;
        for (auto & i : mTaskArray)
            i = nullptr;
    }

    Pool::~Pool()
    {
        mRunning = 0;
        int i;
        for (i = 0; i < mThreadCount; i++)
        {
            Wait(mThread[i]);
            Release(mThread[i]);
        }
        delete[] mThread;
        if (mWorkMutex)
            DestroyMutex(mWorkMutex);
    }

    void Pool::Init(int threadCount)
    {
        if (threadCount > 0)
        {
            mMaxTask = 0;
            mWorkMutex = CreateMutexAm();
            mRunning = 1;
            mThreadCount = threadCount;
            mThread = new AmThreadHandle[threadCount];
            int i;
            for (i = 0; i < mThreadCount; i++)
            {
                mThread[i] = CreateThread(poolWorker, this);
            }
        }
    }

    void Pool::AddWork(PoolTask* task)
    {
        if (mThreadCount == 0)
        {
            task->Work();
        }
        else
        {
            if (mWorkMutex)
                LockMutex(mWorkMutex);
            if (mMaxTask == MAX_THREAD_POOL_TASKS)
            {
                // If we're at max tasks, do the task on calling thread
                // (we're in trouble anyway, might as well slow down adding more Work)
                if (mWorkMutex)
                    UnlockMutex(mWorkMutex);
                task->Work();
            }
            else
            {
                mTaskArray[mMaxTask] = task;
                mMaxTask++;
                if (mWorkMutex)
                    UnlockMutex(mWorkMutex);
            }
        }
    }

    PoolTask* Pool::GetWork()
    {
        PoolTask* t = nullptr;
        if (mWorkMutex)
            LockMutex(mWorkMutex);
        if (mMaxTask > 0)
        {
            int r = mRobin % mMaxTask;
            mRobin++;
            t = mTaskArray[r];
            mTaskArray[r] = mTaskArray[mMaxTask - 1];
            mMaxTask--;
        }
        if (mWorkMutex)
            UnlockMutex(mWorkMutex);
        return t;
    }
} // namespace SparkyStudios::Audio::Amplitude::Thread
