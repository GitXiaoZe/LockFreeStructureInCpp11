#ifndef NODE_H
#define NODE_H

template <typename T>
class Node{
    public :
        Node(T* value_) : value(value_), next(nullptr){}
        Node<T>* next;
        T* value;

};

template<>
class Node<int>{
    public :
        Node(int value_) : value(value_), next(nullptr){}
        Node<int>* next;
        int value;
};

#endif // !NODE_H