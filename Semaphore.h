#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <semaphore.h>


class Semaphore{
    public : 
        Semaphore(){
            sem_init(&sem, 0, 0);
        }
        ~Semaphore(){
            sem_destroy(&sem);
        }

        void Wait(){
            sem_wait(&sem);
        }

        void Post(){
            sem_post(&sem);
        }

        sem_t sem;
};


#endif // !SEMAPHORE_H