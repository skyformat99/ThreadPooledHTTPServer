//
//  main.cpp
//  ThreadPooled Server
//
//  Created by Sourabh Desai on 8/10/14.
//  Copyright (c) 2014 Sourabh Desai. All rights reserved.
//
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <vector>
#include "queue.cpp"
#include "queue.h"
#include "RequestWorker.cpp"
#include "RequestWorker.h"

typedef std::vector<RequestWorker *> WorkerVector;

// Global variables
bool server_on;
const int BACKLOG = SOMAXCONN;

Queue<int> taskQueue(-1);

int thread_count = 0;
WorkerVector worker_pool;

// Global mutexes
pthread_mutex_t taskQueue_mutex;
pthread_mutex_t worker_pool_mutex;

// Method declarations
void *thread_manager_start_routine(void *null_ptr);
void on_thread_birth();
void on_thread_death();

// Function for calculating number of threads to create given current thread pool size and number of waiting requests
size_t num_threads_to_create(const int thread_pool_size, const int num_pending_requests);

void sigint_handler(int signum) {
    signal(SIGINT, sigint_handler); // Resets this as the SIGINT handler
    server_on = false;
}

int main(int argc, const char * argv[])
{
    
    if (argv[1] == nullptr) {
        printf("No Port Number Given, going to exit\n");
        return EXIT_FAILURE;
    }
    
    const char *port_string = argv[1];
    
    int port_number = atoi(port_string);
    
    printf("Server will start on Port %d\n", port_number);
    
    char *cwd = getcwd(NULL, 0);
    printf("cwd: %s\n", cwd);
    
    if (!( (port_number < 60000) && (port_number > 1023) ) ) {
        printf("Invalid Port Number: %d\nPort Number must be between 60000 and 1023 exclusive\n", port_number);
        return EXIT_FAILURE;
    }
    
    pthread_mutex_init(&taskQueue_mutex, NULL);
    pthread_mutex_init(&worker_pool_mutex, NULL);
    
    signal(SIGINT, sigint_handler);
    
    struct addrinfo hints;
    struct addrinfo *servinfo;
    
    memset(&hints, 0x00, sizeof(hints));
    
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;
    
    if ( getaddrinfo(NULL, port_string, &hints, &servinfo) != 0) {
        printf("Call to \"getaddrinfo(...)\" returned an error\n");
        return EXIT_FAILURE;
    }
    
    int server_socketfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    
    if (server_socketfd == -1) {
        printf("Call to socket() for getting server socket returned error\n");
        return EXIT_FAILURE;
    }
    
    if ( bind(server_socketfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1 ) {
        printf("Call to bind(...) returned error\n");
        return EXIT_FAILURE;
    }
    
    struct sockaddr_storage their_addr;
    socklen_t addr_storage_size = sizeof(their_addr);
    
    pthread_t manager_thread;
    pthread_create(&manager_thread, NULL, thread_manager_start_routine, NULL);
    
    server_on = true;
    
    while (server_on) {
        printf("Listening on port %d with backlog of %d\n", port_number, BACKLOG);
        int err = listen(server_socketfd, BACKLOG);
        
        if (err == -1) {
            printf("listen() returned error\n");
            continue;
        }
        
        if (!server_on)
            break;
        
        int conn_fd = accept(server_socketfd, (struct sockaddr *) &their_addr, &addr_storage_size);
        if (conn_fd >= 0)
            taskQueue.enqueue(conn_fd);
        
        
    }
    
    
    pthread_mutex_destroy(&taskQueue_mutex);
    close(server_socketfd);
    
    // Join all live threads in thread pool
    WorkerVector::iterator it;
    for(it = worker_pool.begin(); it != worker_pool.end(); it++) {
        RequestWorker *worker = *it;
        if (worker != nullptr) {
            void *useless_ret_val;
            pthread_join(*worker->get_thread(), &useless_ret_val);
            delete worker;
        }
    }
    
    // Join thread_manager thread
    void *useless;
    pthread_join(manager_thread, &useless);
    
    return 0;
}

// Callback functions to be called on thread creation and deletion
void on_thread_birth() {
    printf("on_thread_birth() called\n"); // Doesn't do anything useful at the moment
}

void on_thread_death(size_t thread_index) {
    pthread_mutex_lock(&worker_pool_mutex);
    thread_count--;
    delete worker_pool[thread_index];
    worker_pool[thread_index] = nullptr;
    pthread_mutex_unlock(&worker_pool_mutex);
}

void *thread_manager_start_routine(void *null_ptr) {
    
    while (server_on) {
        if (num_threads_to_create(thread_count, taskQueue.getSize()) > 0) {
            pthread_mutex_lock(&worker_pool_mutex);
            taskQueue.lock_size(); // Will halt all handling of *pending* requests. Ongoing requests will continue though
            size_t num_new_threads = num_threads_to_create(thread_count, taskQueue.getSize());
            if (num_new_threads > 0) {
                // Calculation may have changed so must recalculate to make sure. Important to make sure num_threads_to_create(...) is O(1) and *fast*
                // Add threads to thread pool, which is represented as the vector worker_pool
                size_t i;
                for (i = 0; i < num_new_threads; i++) {
                    pthread_t *thread_id = (pthread_t *) malloc(sizeof(pthread_t));
                    RequestWorker *worker = new RequestWorker(thread_count + i, thread_id, taskQueue, &server_on, on_thread_birth, on_thread_death);
                    worker_pool.push_back(worker);
                    pthread_create(thread_id, NULL, RequestWorker::work_request, worker);
                }
                
                thread_count += num_new_threads;
            }
            taskQueue.unlock_size();
            pthread_mutex_unlock(&worker_pool_mutex);
        }
    }
    
    return nullptr;
}

size_t num_threads_to_create(const int thread_pool_size, const int num_pending_requests) {
    return num_pending_requests < thread_pool_size ? 0 : num_pending_requests - thread_pool_size;
}

