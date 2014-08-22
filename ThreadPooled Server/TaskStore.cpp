//
//  TaskStore.cpp
//  ThreadPooled Server
//
//  Created by Sourabh Desai on 8/20/14.
//  Copyright (c) 2014 Sourabh Desai. All rights reserved.
//

#include "TaskStore.h"

TaskStore::TaskStore() {
    pthread_mutex_init(&this->get_mutex, NULL);
    pthread_mutex_init(&this->set_mutex, NULL);
    this->num_tasks_pending = 0;

    // Initialize first task queue
    queue<int> q;
    
    pair<pthread_mutex_t, queue<int>> *new_pair = new pair<pthread_mutex_t, queue<int>>;
    *new_pair = { pthread_mutex_t(), queue<int>() };
    
    
    this->get_queues.push(new_pair);
    this->set_queues.push(new_pair);
}

TaskStore::~TaskStore() {
    pthread_mutex_destroy(&this->get_mutex);
    pthread_mutex_destroy(&this->set_mutex);
    
    // Close all remaining socket descriptors and destroy all queue mutexes
    while (!this->get_queues.empty()) {
        pair<pthread_mutex_t, queue<int>> * p = this->get_queues.front();
        pthread_mutex_destroy(&p->first);
        while (!p->second.empty()) {
            int socketfd = p->second.front();
            close(socketfd);
            p->second.pop();
        }
        delete p;
    }
}

int TaskStore::get_next_task() {
    while (num_tasks_pending > 0) {
        pthread_mutex_lock(&this->get_mutex);
        
        pair<pthread_mutex_t, queue<int>> * task_queue = (pair<pthread_mutex_t, queue<int>> *) this->get_queues.front();
        this->get_queues.pop();
        this->get_queues.push(task_queue);
        
        pthread_mutex_unlock(&this->get_mutex);
        
        pthread_mutex_lock(&task_queue->first);
        
        if (task_queue->second.empty()) {
            pthread_mutex_unlock(&task_queue->first);
            continue;
        }
        int nextfd = task_queue->second.front();
        task_queue->second.pop();
        this->num_tasks_pending--;
        
        pthread_mutex_unlock(&task_queue->first);
        return nextfd;
    }

    return -1;
}

void TaskStore::store_task(int taskfd) {
    pthread_mutex_lock(&this->set_mutex);
    
    pair<pthread_mutex_t, queue<int> > *task_queue =  this->set_queues.front();
    this->set_queues.pop(); // pop from front
    this->set_queues.push(task_queue); // push to back
    
    pthread_mutex_unlock(&this->set_mutex);
    
    pthread_mutex_lock(&task_queue->first);
    
    task_queue->second.push(taskfd); // Add new task to this queue
    this->num_tasks_pending++;
    
    pthread_mutex_unlock(&task_queue->first);
}

// NOT THREAD SAFE
void TaskStore::add_task_queue() {
    printf("TaskStore::add_task_queue() { message: \"%s\" }\n", "Called");
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    
    pair<pthread_mutex_t, queue<int>> *new_pair = new pair<pthread_mutex_t, queue<int>>;
    *new_pair = { pthread_mutex_t() , queue<int>() };
    pthread_mutex_init(&new_pair->first, NULL);
    
    queue<int> & q = new_pair->second;
    
    size_t avg_q_size = this->num_tasks_pending / this->get_queues.size();
    
    while (q.size() < avg_q_size) {
        pthread_mutex_lock(&this->get_mutex);
        
        if (!this->get_queues.front()->second.empty()) {
            pthread_mutex_lock(&this->get_queues.front()->first);
            q.push(this->get_queues.front()->second.front());
            this->get_queues.front()->second.pop();
            pthread_mutex_unlock(&this->get_queues.front()->first);
        }
        
        pair<pthread_mutex_t, queue<int>> * old_pair = this->get_queues.front();
        this->get_queues.pop();
        this->get_queues.push(old_pair);
        
        pthread_mutex_unlock(&this->get_mutex);
    }
    
    pthread_mutex_lock(&this->get_mutex);
    this->get_queues.push(new_pair);
    pthread_mutex_unlock(&this->get_mutex);
    
    pthread_mutex_lock(&this->set_mutex);
    this->set_queues.push(this->get_queues.back());
    pthread_mutex_unlock(&this->set_mutex);
    
    printf("TaskStore::add_task_queue() { message: \"%s\" }\n", "Finished");
}

int TaskStore::num_tasks() {
    return this->num_tasks_pending;
}