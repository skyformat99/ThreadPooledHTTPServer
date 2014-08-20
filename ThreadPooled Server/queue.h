//
//  queue.h
//  ThreadPooled Server
//
//  Created by Sourabh Desai on 8/10/14.
//  Copyright (c) 2014 Sourabh Desai. All rights reserved.
//

#ifndef P2P_Server_queue_h
#define P2P_Server_queue_h

#include <pthread.h>

/**
 * Queue class: represents a standard queue. Templated to hold elements of
 * any type. **You must only use the two private member Stacks as your
 * storage space! You cannot create new private member variables to hold
 * your objects!** It is up to you to determine how to use them, however.
 *
 * Your Queue class should have O(1) running time over n operations
 * (amortized). There is an obvious solution that takes O(n) running time,
 * but this will not recieve all of the available points.
 *
 * You **should not** modify this file for the MP!
 *
 * @author CS 225 course staff
 * @date Spring 2007
 *
 * @author Daniel Hoodin
 * @date Spring 2008
 *
 * @author Chase Geigle
 * @date Fall 2012
 */
template<class T>
class Queue {

public:
    
    Queue<T>(T default_ret_value);
    ~Queue<T>();
    
    /**
     * Adds the parameter object to the back of the Queue.
     *
     * @note This fuction should have O(1) behavior over n operations!
     *
     * @param newItem object to be added to the Queue.
     */
    void enqueue( const T & newItem );
    
    /**
     * Removes the object at the front of the Queue, and returns it to
     * the caller. You may assume that this function is only called
     * when the Queue is non-empty.
     *
     * @note This function should have O(1) behavior over n operations!
     *
     * @return The item that used to be at the front of the Queue.
     */
    T dequeue();
    
    /**
     * Determines if the Queue is empty.
     *
     * @note This function should have O(1) behavior over n operations!
     *
     * @return bool which is true if the Queue is empty, false
     *	otherwise.
     */
    bool isEmpty() const;
    
    unsigned int getSize() const;
    
    void lock_size();
    void unlock_size();
    
    T default_value() const;
    
private:
    
    class Node {
    private:
        T value;
        
    public:
        
        Node(const T & item) {
            this->next = nullptr;
            this->value = item;
        }
        
        Node *get_next() {
            return this->next;
        }
        
        T get_value() {
            return this->value;
        }
        
        Node *next;
        
    };
    
    T default_ret_value;
    
    Node *head;
    Node *tail;
    unsigned int size = 0;
    
    void destroy_nodes_with_free_values();
    void destroy_nodes();
    
    pthread_mutex_t head_mutex;
    pthread_mutex_t tail_mutex;
    pthread_mutex_t size_mutex;
    
};

#endif
