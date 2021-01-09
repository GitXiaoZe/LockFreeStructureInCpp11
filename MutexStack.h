#ifndef MUTEXSTACK_H

#include "Node.h"
#include <mutex>
#include <atomic>

template <typename T>
class MutexStack{
    public :
        Node<T>* head;
        std::mutex mtx;
        MutexStack() : head(nullptr){}

        bool empty(){
            return head == nullptr;
        }

        void push(T* value){
            Node<T>* newhead = new Node<T>(value);
            std::lock_guard<std::mutex> lock(mtx);
            newhead->next = head;
            head = newhead;
        }

        T* pop(){
            Node<T>* tmp;
            {
                std::lock_guard<std::mutex> lock(mtx);
                tmp = head;
                head = head->next;
            }
            T* res = tmp->value;
            delete tmp;
            return res;
        }
};

template <>
class MutexStack<int>{
    public :
        Node<int>* head;
        std::mutex mtx;

        MutexStack() {
            head = nullptr;
        }

        bool empty(){
            return head == nullptr;
        }

        void push(int value){
            Node<int>* newhead = new Node<int>(value);
            std::lock_guard<std::mutex> lock(mtx);
            newhead->next = head;
            head = newhead;
        }

        int pop(){
            Node<int>* tmp;
            {
                std::lock_guard<std::mutex> lock(mtx);
                tmp = head;
                head = head->next;
            }
            int res = tmp->value;
            delete tmp;
            return res;
        }
};

#endif // !MUTEXSTACK_H