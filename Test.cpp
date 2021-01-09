#include <thread>
#include <vector>
#include <unistd.h>
#include <time.h>
#include <cstdio>

#include "Time.h"
#include "Semaphore.h"
#include "MutexStack.h"
#include "LockFreeStack.h"

#include "MutexRing.h"
#include "LockFreeRing.h"

#define THREAD 10
#define ITERATION 1000
#define SLEEPELAPSE 1


//MutexStack<int> Container;
//LockFreeStack<int> Container;
//MutexRing<int> Container;
LockFreeRing<int> Container;


Semaphore sem;
typedef std::thread* ThreadPtr;
std::vector<ThreadPtr> threads;
std::vector<Nanosecond> elapsedTime;

void fun(int i){
    sleep(SLEEPELAPSE);
    sem.Wait();
    elapsedTime[i] = Timer::GetCurrentTime();
    for(int ite = 0; ite < ITERATION; ite++){
        Container.push(i);
        Container.pop();
    }
    elapsedTime[i] = Timer::GetCurrentTime() - elapsedTime[i];
}


void Test(){
    threads.reserve(THREAD);
    elapsedTime.resize(THREAD, 0);
    for(int i=0; i < THREAD; i++){
        threads.push_back(new std::thread(&fun, i));
    }
    for(int i=0; i < THREAD; i++){
        sem.Post();
    }
    for(int i=0; i < THREAD; i++)
        if(threads[i]->joinable())
            threads[i]->join();
    Nanosecond total = 0;
    for(int i=0; i < THREAD; i++){
        total += elapsedTime[i];
        delete threads[i];
    }

    printf("total time = %ld, avg time = %ld\n", total, total/1000);
    printf("Container is empty : %d\n", Container.empty());
}

int main(){

    Test();

    return 0;
}


