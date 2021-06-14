#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "threadPool.h"



void* runTask(void* task) {

}


ThreadPool* tpCreate(int numOfThreads){

    ThreadPool *threadPool = (ThreadPool*) malloc(sizeof(ThreadPool));
    // if allocation failed
    if (!threadPool) {
        perror("ERROR");
        return NULL;
    }
    OSQueue *queue = osCreateQueue();
    pthread_t * threads = (pthread_t *) malloc (numOfThreads * sizeof(pthread_t));
    // if allocation failed
    if(!threads) {
        free(threads);
        perror("ERROR");
        return NULL;
    }
    // todo move it to a function
    threadPool->threads = threads;
    threadPool->tasksQueue = queue;
    threadPool->numOfThreads = numOfThreads;
    threadPool->isRunning = 1;

    int initStatus = pthread_mutex_init(&threadPool->mutex, NULL);
    if(initStatus != 0) {
        perror("ERROR");
    }

    initStatus = pthread_cond_init(&threadPool->cond, NULL);
    if(initStatus != 0) {
        perror("ERROR");
    }

    int i = 0;
    for(i; i < numOfThreads; i++) {
        initStatus = pthread_create(&(threadPool->threads[i]), NULL, runTask, (void*)threadPool)




    }




}