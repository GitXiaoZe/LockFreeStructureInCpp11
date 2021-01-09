#ifndef NATIVEQUEUE_H

#define NATIVEQUEUE_H

#include <mutex>
#include <cassert>
#include <condition_variable>

#define QUEUE_SIZE 4096
#define SIZE 4096

template<class T>
class MutexRing{
    public :
        MutexRing(){

        }

        void push(T* x){
            std::unique_lock<std::mutex> lock(mtx_);

            cond_overflow_.wait(lock, [this](){
                    return tail_ + SIZE > head_;
                });
            ptr_array_[head_++ & MASK] = x;
            cond_empty_.notify_one();
        }
        T* pop(){
            std::unique_lock<std::mutex> lock(mtx_);
            cond_empty_.wait(lock, [this](){
                    return tail_ < head_;
                });
            T* x = ptr_array_[tail_++ & MASK];
            cond_overflow_.notify_one();
            return x;
        }

        T* ptr_array_[SIZE];
        unsigned long MASK = SIZE - 1;
        unsigned long head_, tail_;
        std::condition_variable cond_empty_;
        std::condition_variable cond_overflow_;
        std::mutex mtx_;
};



template<>
class MutexRing<int>{
    public :
        MutexRing(){
            array = new int[QUEUE_SIZE];
        }
        ~MutexRing(){
            if(array)
                delete [] array;
        }

        bool empty(){
            {
                std::unique_lock<std::mutex> lock(mtx);
                return head == tail;
            }
        }

        void push(int value){
            {
                std::unique_lock<std::mutex> lock(mtx);

                cond_overflow.wait(lock, [this]{
                    return tail + SIZE > head;
                });

                array[head++ & MASK] = value;
            }
            cond_empty.notify_one();
        }

        int pop(){
            int ret;
            {
                std::unique_lock<std::mutex> lock(mtx);
                cond_empty.wait(lock, [this](){
                    return tail < head;
                });

                ret = array[tail++ & MASK];
            }
            cond_overflow.notify_all();
            return ret;
        }

        int* array;
        std::condition_variable cond_empty;
        std::condition_variable cond_overflow;
        std::mutex mtx;
        unsigned long head, tail;
        static const unsigned long MASK = SIZE - 1;
};

#endif // !NATIVEQUEUE_H