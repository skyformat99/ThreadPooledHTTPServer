//
//  queue.cpp
//  ThreadPooled Server
//
//  Created by Sourabh Desai on 8/10/14.
//  Copyright (c) 2014 Sourabh Desai. All rights reserved.
//
#include "queue.h"
#include <pthread.h>

template<class T>
Queue<T>::Queue(T default_ret_value) {
    this->default_ret_value = default_ret_value;
    this->head = nullptr;
    this->tail = nullptr;
    this->size = 0;
    
    pthread_mutex_init(&this->head_mutex, NULL);
    pthread_mutex_init(&this->tail_mutex, NULL);
    pthread_mutex_init(&this->size_mutex, NULL);
    
}

template<class T>
Queue<T>::~Queue() {
    this->destroy_nodes();
    // Set head and tail to null
    this->head = nullptr;
    this->tail = nullptr;
}

template<class T>
void Queue<T>::destroy_nodes() {
    Node *node = this->head;
    while (node != nullptr) {
        Node *nextNode = node->get_next();
        delete node;
        node = nextNode;
    }
}

template<class T>
void Queue<T>::lock_size() {
    pthread_mutex_lock(&this->size_mutex);
}

template<class T>
void Queue<T>::unlock_size() {
    pthread_mutex_unlock(&this->size_mutex);
}

template<class T>
T Queue<T>::default_value() const {
    return this->default_ret_value;
}

/**
 * Adds the parameter object to the back of the Queue.
 *
 * @param newItem object to be added to the Queue.
 */
template<class T>
void Queue<T>::enqueue(T const & newItem)
{
    pthread_mutex_lock(&this->size_mutex);
    if (this->size == 0) {
        
        this->size++;
        pthread_mutex_unlock(&this->size_mutex);
        
        pthread_mutex_lock(&this->head_mutex);
        pthread_mutex_lock(&this->tail_mutex);
        
        /********CRITICAL SECTION START******/
        this->head = new Node(newItem);
        this->tail = this->head;
        /********CRITICAL SECTION END********/
        
        pthread_mutex_unlock(&this->tail_mutex);
        pthread_mutex_unlock(&this->head_mutex);
        
    } else if(size == 1) { // Means head == tail
        
        this->size++;
        pthread_mutex_unlock(&this->size_mutex);
        
        pthread_mutex_lock(&this->head_mutex);
        pthread_mutex_lock(&this->tail_mutex);
        
        /********CRITICAL SECTION START******/
        this->tail->next = new Node(newItem);
        this->tail = this->tail->next;
        /********CRITICAL SECTION END********/
        
        pthread_mutex_unlock(&this->tail_mutex);
        pthread_mutex_unlock(&this->head_mutex);
        
    } else {
        
        this->size++;
        pthread_mutex_unlock(&this->size_mutex);
        
        pthread_mutex_lock(&this->tail_mutex);
        
        /********CRITICAL SECTION START******/
        this->tail->next = new Node(newItem);
        this->tail = this->tail->next;
        /********CRITICAL SECTION END********/
        
        pthread_mutex_unlock(&this->tail_mutex);
        
    }
}

/**
 * Removes the object at the front of the Queue, and returns it to the
 * caller.
 *
 * @return The item that used to be at the front of the Queue.
 */
template<class T>
T Queue<T>::dequeue()
{
    pthread_mutex_lock(&this->size_mutex);
    if (this->size == 0) {
        
        pthread_mutex_unlock(&this->size_mutex);
        return this->default_ret_value;
    } else if (size <= 2) { // Need to have block for size <= 2 because when we are setting new head with size == 2, we are referencing tail
        
        this->size--;
        pthread_mutex_unlock(&this->size_mutex);
        
        pthread_mutex_lock(&this->tail_mutex);
        pthread_mutex_lock(&this->head_mutex);
        
        /********CRITICAL SECTION START******/
        bool head_is_tail = this->head == this->tail;
        Node *next_head = this->head->next;
        T value = this->head->get_value();
        delete this->head;
        this->head = next_head;
        if (head_is_tail)
            this->tail = nullptr; // head_is_tail means head and tail were pointing to same node, which is now dequeued. Must set tail to null now. Head is already set to null
        /********CRITICAL SECTION END********/
        
        pthread_mutex_unlock(&this->head_mutex);
        pthread_mutex_unlock(&this->tail_mutex);
        
        return value;
    } else {
        
        this->size--;
        pthread_mutex_unlock(&this->size_mutex);
        
        pthread_mutex_lock(&this->head_mutex);
        
        /********CRITICAL SECTION START******/
        Node *next_head = this->head->next;
        T value = this->head->get_value();
        delete this->head;
        this->head = next_head;
        /********CRITICAL SECTION END********/
        
        pthread_mutex_unlock(&this->head_mutex);
        return value;
    }
}

/**
 * Determines if the Queue is empty.
 *
 * @return bool which is true if the Queue is empty, false otherwise.
 */
template<class T>
bool Queue<T>::isEmpty() const
{
    return this->size == 0;
}

template<class T>
unsigned int Queue<T>::getSize() const {
    return this->size;
}
