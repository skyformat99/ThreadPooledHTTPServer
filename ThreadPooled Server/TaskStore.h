//
//  TaskStore.h
//  ThreadPooled Server
//
//  Created by Sourabh Desai on 8/20/14.
//  Copyright (c) 2014 Sourabh Desai. All rights reserved.
//

#ifndef ThreadPooled_Server_TaskStore_h
#define ThreadPooled_Server_TaskStore_h

#include <atomic>
#include <vector>
#include <queue>
#include <pthread.h>

using std::atomic_int;
using std::vector;
using std::queue;
using std::pair;


class TaskStore {

public:
    
    TaskStore();
    ~TaskStore();
    
    int get_next_task();
    
    void store_task(int taskfd);
    
    int num_tasks();
    
private:
    
    atomic_int num_tasks_pending;
    
    queue<pair<pthread_mutex_t, queue<int>> *> get_queues;
    queue<pair<pthread_mutex_t, queue<int>> *> set_queues;
    
    pthread_mutex_t get_mutex;
    pthread_mutex_t set_mutex;
    
    void add_task_queue();

};

#endif
