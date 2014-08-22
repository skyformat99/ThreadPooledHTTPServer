//
//  RequestWorker.h
//  ThreadPooled Server
//
//  Created by Sourabh Desai on 8/11/14.
//  Copyright (c) 2014 Sourabh Desai. All rights reserved.
//

#ifndef P2P_Server_RequestWorker_h
#define P2P_Server_RequestWorker_h

#include "TaskStore.h"
#include <pthread.h>

class RequestWorker {

public:
    
    // Public instance methods and c'tors
    
    RequestWorker(size_t thread_index, pthread_t *thread, TaskStore & task_store, bool * server_on, void (*thread_birth_cb) (), void (*thread_death_cb) (size_t));
    
    ~RequestWorker();
    
    static void *work_request(void *request_worker_void);
    
    void lock_is_working();
    void unlock_is_working();
    pthread_t *get_thread();
    
    bool is_working();
    
private:

    // Private member variables
    size_t thread_index;
    pthread_t *thread;
    bool working;
    pthread_mutex_t is_working_mutex;
    void (*on_birth) ();
    void (*on_death) (size_t);
    
    TaskStore *task_store;
    const bool *server_on;
    
};

#endif
