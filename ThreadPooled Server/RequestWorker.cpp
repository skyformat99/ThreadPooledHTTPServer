//
//  RequestWorker.cpp
//  ThreadPooled Server
//
//  Created by Sourabh Desai on 8/11/14.
//  Copyright (c) 2014 Sourabh Desai. All rights reserved.
//

#include "RequestWorker.h"
#include "queue.h"
#include <iostream>
#include "HTTPTalker.cpp"
#include "HTTPTalker.h"

RequestWorker::RequestWorker(size_t thread_index, pthread_t *thread, Queue<int> & taskQueue, bool * server_on, void (*thread_birth_cb) (), void (*thread_death_cb) (size_t)) {
    this->thread_index = thread_index;
    this->thread = thread;
    this->working = false;
    this->taskQueue = &taskQueue;
    this->server_on = server_on;
    this->on_birth = thread_birth_cb;
    this->on_death = thread_death_cb;
    pthread_mutex_init(&this->is_working_mutex, NULL);
}

RequestWorker::~RequestWorker() {
    free(this->thread);
}

bool RequestWorker::is_working() {
    return this->working;
}

void RequestWorker::lock_is_working() {
    pthread_mutex_lock(&this->is_working_mutex);
}

void RequestWorker::unlock_is_working() {
    pthread_mutex_unlock(&this->is_working_mutex);
}

pthread_t *RequestWorker::get_thread() {
    return this->thread;
}

// Static method
void *RequestWorker::work_request(void *request_worker_void) {
    RequestWorker & worker = *((RequestWorker *) request_worker_void); // Assignment by reference. I like using the dot operator where I can
    
    worker.on_birth();
    
    while (*worker.server_on) {
        int connfd = worker.taskQueue->dequeue();
        
        if (connfd == worker.taskQueue->default_value()) // Means taskQueue is empty
            break;
        
        printf("RequestWorker::work_request() 53: { message: \"Got connfd as %d\" }\n", connfd);
        
        HTTPTalker talker(connfd);
        do {
            if (!talker.receive_request()) {
                printf("RequestWorker::work_request() 56: { message: \"%s\" }\n", "receive_request() failed");
                break;
            } else {
                char *message = "Hello World";
                talker.send_response((void *) message, strlen(message), TEXT_PLAIN, 200);
            }
        } while (talker.keep_alive() && *worker.server_on);
        
        close(connfd);
    }
    
    worker.on_death(worker.thread_index); // Will free memory associated with this RequestWorker
    return nullptr;
}