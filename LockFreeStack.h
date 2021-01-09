#ifndef LOCKFREESTACK_H

#define LOCKFREESTACK_H

#include "Node.h"
#include <atomic>
#include <cstdio>
#include <mutex>
#include <vector>
#include <cstdlib>

#include "Time.h"

/*
    Using TimeStamp to slove ABA Problem
*/




template<typename T> 
class NodeWithTS{
    public : 
        Node<T>* node;
        unsigned long timestamp;
        NodeWithTS() noexcept : node(nullptr), timestamp(0){};
        NodeWithTS(Node<T>* node_, unsigned long timestamp_) : node(node_), timestamp(timestamp_){}
};


template<typename T>
class LockFreeStack{
    public :
        //std::atomic<Node<T>*> head;
        std::atomic<NodeWithTS<T>> head;
        LockFreeStack(){
            head = NodeWithTS<T>();
        }

        bool empty(){
            return head.load().node == nullptr;
        }

        void push(T* value){
            Node<T>* newnode = new Node<T>(value);
            unsigned long currentTS = Timer::GetCurrentTime();
            NodeWithTS<T> newhead(newnode, currentTS);
            NodeWithTS<T> nexthead;
            do{
                nexthead = head.load();
                newnode->next = nexthead.node;
            }while(!head.compare_exchange_strong(nexthead, newhead));
        }

        T* pop(){
            Node<T>* res;
            unsigned long currentTS = Timer::GetCurrentTime();
            NodeWithTS<T> retnode;
            NodeWithTS<T> nexthead;

            do{
                retnode = head.load();
                res = retnode.node;
                if(res == nullptr)
                    return nullptr;
                nexthead.node = res->next;//null pointer exception???
                nexthead.timestamp = currentTS;
            }while(!head.compare_exchange_strong(retnode, nexthead));

            T* ret = res->value;
            delete res;
            return ret;
        }

};


template<>
class NodeWithTS<int>{
    public : 
        Node<int>* node;
        unsigned long timestamp;
        NodeWithTS() noexcept : node(nullptr), timestamp(0){};
        NodeWithTS(Node<int>* node_, unsigned long timestamp_) : node(node_), timestamp(timestamp_){}
};

template<>
class LockFreeStack<int>{
    public:
        std::atomic<NodeWithTS<int>> head;

        LockFreeStack(){
            head = NodeWithTS<int>();
        }

        bool empty(){
            return head.load().node == nullptr;
        }

        void push(int value){
            Node<int>* newnode = new Node<int>(value);
            unsigned long currentTS = Timer::GetCurrentTime();
            NodeWithTS<int> newhead(newnode, currentTS);
            NodeWithTS<int> nexthead;
            do{
                nexthead = head.load();
                newnode->next = nexthead.node;
            }while(!head.compare_exchange_strong(nexthead, newhead));
        }

        int pop(){
            Node<int>* res;

            unsigned long currentTS = Timer::GetCurrentTime();
            NodeWithTS<int> retnode;
            NodeWithTS<int> nexthead;

            do{
                retnode = head.load();
                res = retnode.node;
                if(res == nullptr)
                    throw "Empty Stack";

                nexthead.node = res->next;//null pointer exception
                nexthead.timestamp = currentTS;
            }while(!head.compare_exchange_strong(retnode, nexthead));
            
            int ret = res->value;
            delete res;
            return ret;
        }
};




#endif // !LOCKFREESTACK_H