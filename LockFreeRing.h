#ifndef LOCKFREERING_H
#define LOCKFREERING_H

#include <atomic>
#include <cstdlib>
#include <unistd.h>
#include <cassert>
#include <cstring>
#include <emmintrin.h>
#include <malloc.h>

/*
    LockFree Queue
*/

#define CACHE_LINE 64
#define QUEUE_SIZE 4096
#define SIZE 4096
#define ULONGMAX 0xFFFFFFFFFFFFFFFF

#define __align_cache __attribute__((aligned(CACHE_LINE)))
#define mfence asm volatile("" ::: "memory")

thread_local int threadId = -1;

static std::atomic<int> IdCounter(0);

struct ThreadPosition{
    volatile unsigned long head, tail;
};

/* The value of SIZE shoule be power of 2*/
template<typename T>
class LockFreeRing{
    public :
        LockFreeRing(int n = 100) : maxThreadNumber(n), head(0), tail(0), last_head(0), last_tail(0){
            IdCounter = 0;

            position = (ThreadPosition*)::memalign(getpagesize(), sizeof(ThreadPosition) * n);
            assert(position);
            std::memset((void*)position, 0xFF, sizeof(ThreadPosition) * n);
            array = (T**)::memalign(getpagesize(), SIZE * sizeof(void*));
            assert(array);
        }

        ~LockFreeRing(){
            ::free(position);
            ::free(array);
        }

        void push(T* value){
            if(threadId < 0)
                threadId = ++IdCounter;
            
            assert(threadId < maxThreadNumber);

            ThreadPosition& thp = position[threadId];

            thp.head = head.load();
            thp.head = head++;

            while(thp.head >= last_tail + SIZE){
                
                unsigned long min = tail.load();

                for(int i=0; i < maxThreadNumber; i++){
                    unsigned long tmp = position[i].tail;

                    //std::atomic_thread_fence();
                    mfence;

                    if(tmp < min)
                        min = tmp;
                }

                last_tail = min;

                if(thp.head < last_tail + SIZE)
                    break;
                _mm_pause();
            }

            array[thp.head & MASK] = value;

            thp.head = ULONGMAX;
        }

        T* pop(){
            if(threadId < 0)
                threadId = ++IdCounter;
            assert(threadId < maxThreadNumber);

            ThreadPosition& thp = position[threadId];

            thp.tail = tail.load();
            thp.tail = tail++;

            while(thp.tail >= last_head){
                unsigned long min = head.load();
                for(int i=0; i < maxThreadNumber; i++){
                    unsigned long tmp = position[i].head;
                    
                    mfence;

                    if(tmp < min)
                        min = tmp;
                }
                last_head = min;

                if(thp.tail < last_head)
                    break;
                _mm_pause();
            }

            T* ret = array[thp.tail & MASK];
            thp.tail = ULONGMAX;

            return ret;
        }


        //unsigned long head __align_cache;
        //unsigned long tail __align_cache;

        unsigned long last_head __align_cache;
        unsigned long last_tail __align_cache;

        ThreadPosition* position;
        T** array;

        std::atomic<unsigned long> head;
        std::atomic<unsigned long> tail;

        const int maxThreadNumber;
        std::atomic<int> IdCounter;
        static const unsigned int MASK = SIZE - 1;
};



template<>
class LockFreeRing<int>{
    public :
        LockFreeRing(int n = 100) : maxThreadNumber(n), head(0), tail(0), last_head(0), last_tail(0){
            IdCounter = 0;

            position = (ThreadPosition*)::memalign(getpagesize(), sizeof(ThreadPosition) * n);
            assert(position);
            std::memset((void*)position, 0xFF, sizeof(ThreadPosition) * n);
            array = (int*)::memalign(getpagesize(), SIZE * sizeof(int));
            assert(array);
        }
        ~LockFreeRing(){
            ::free(position);
            ::free(array);
        }

        bool empty(){
            return head.load() == tail.load();
        }

        void push(int value){
            if(threadId < 0)
                threadId = IdCounter++;
            assert(threadId < maxThreadNumber);
            ThreadPosition& thp = position[threadId];

            thp.head = head.load();
            thp.head = head++;

            while(thp.head >= last_tail + SIZE){
                unsigned long min = tail.load();

                for(int i=0; i < maxThreadNumber; i++){
                    unsigned long tmp = position[i].tail;

                    //mfence;
                    asm volatile("" ::: "memory");

                    if(tmp < min)
                        min = tmp;
                }
                last_tail = min;

                if(thp.head < last_tail + SIZE)
                    break;
                _mm_pause();
            }

            array[thp.head & MASK] = value;
            thp.head = ULONGMAX;
        }

        int pop(){
            if(threadId < 0)
                threadId = IdCounter++;
            assert(threadId < maxThreadNumber);

            ThreadPosition& thp = position[threadId];

            thp.tail = tail.load();
            thp.tail = tail++;

            while(thp.tail >= last_head){
                unsigned long min = head.load();

                for(int i=0 ; i < maxThreadNumber; i++){
                    unsigned long tmp = position[i].head;

                    //mfence;
                    asm volatile("" ::: "memory");

                    if(tmp < min)
                        min = tmp;
                }

                last_head = min;
                if(thp.tail < last_head)
                    break;
                _mm_pause();
            }

            int res = array[thp.tail & MASK];
            thp.tail = ULONGMAX;
            return res;
        }

        //unsigned long head __align_cache;
        //unsigned long tail __align_cache;

        unsigned long last_head __align_cache;
        unsigned long last_tail __align_cache;

        ThreadPosition* position;
        int* array;

        std::atomic<unsigned long> head;
        std::atomic<unsigned long> tail;

        const int maxThreadNumber;
        static const unsigned int MASK = SIZE - 1;
} __align_cache;

#endif // !LOCKFREERING_H