// Avraham Bar Ilan 205937949

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "threadPool.h"

// handle situation that sys call failed
void sysCallError(ThreadPool* threadPool, char* error){
    perror(error);
    free(threadPool->threads);
    free(threadPool);
    exit(-1);
}

// init threadPool with default values
void initThreadPool(ThreadPool* tp, pthread_t *threads, OSQueue *queue) {
    tp->isRunning = 1;
    tp->hasRunningTasks = 1;
    tp->threads = threads;
    tp->tasksQueue = queue;
}

// free threadPool memory
void freeThreadPool(ThreadPool *tp){
    free(tp->threads);
    free(tp);
}

// execute the given task
void* runTask(void* task) {
    ThreadPool* threadPool = (ThreadPool*) task;
    int tempStatus;
    while(threadPool->hasRunningTasks != 0) {
        tempStatus = pthread_mutex_lock(&(threadPool->mutex));
        if (tempStatus != 0) {
            sysCallError(threadPool, "Error in pthread_mutex_lock\n");
            exit(-1);
        } else {
            tempStatus = osIsQueueEmpty(threadPool->tasksQueue);
            // if queue is not empty yet
            if(tempStatus == 0) {
                Task* currentTask = osDequeue(threadPool->tasksQueue);
                tempStatus = pthread_mutex_unlock(&(threadPool->mutex));
                if (tempStatus != 0) {
                    sysCallError(threadPool, "Error in pthread_mutex_unlock\n");
                }
                currentTask->function(currentTask->param);
                free(currentTask);

            } else {
                if(threadPool->isRunning == 0) {
                    tempStatus = pthread_mutex_unlock(&(threadPool->mutex));
                    if(tempStatus != 0) {
                        sysCallError(threadPool, "Error in pthread_mutex_unlock\n");
                    }
                    break;
                }
                pthread_cond_wait(&(threadPool->cond), &threadPool->mutex);
                tempStatus = pthread_mutex_unlock((&(threadPool->mutex)));
                if (tempStatus != 0) {
                    sysCallError(threadPool, "Error in pthread_mutex_unlock\n");
                }
            }
        }
    }
}

ThreadPool* tpCreate(int numOfThreads){

    ThreadPool *threadPool = (ThreadPool*) malloc(sizeof(ThreadPool));
    // if allocation failed
    if (!threadPool) {
        perror("Error in malloc\n");
        return NULL;
    }
    OSQueue *queue = osCreateQueue();
    threadPool->numOfThreads = numOfThreads;
    pthread_t * threads = (pthread_t *) calloc (threadPool->numOfThreads, sizeof(pthread_t));
    // if allocation failed
    if(!threads) {
        free(threads);
        perror("Error in calloc\n");
        return NULL;
    }
    initThreadPool(threadPool, threads, queue);
    int initStatus = pthread_mutex_init(&threadPool->mutex, NULL);
    if(initStatus != 0) {
        perror("ERROR in pthread_mutex_init\n");
    }
    initStatus = pthread_cond_init(&threadPool->cond, NULL);
    if(initStatus != 0) {
        perror("ERROR in pthread_cond_init\n");
    }
    int i = 0, j;
    for(; i < numOfThreads; i++) {
        initStatus = pthread_create(&(threadPool->threads[i]), NULL, runTask, (void*)threadPool);
        // if pthread create failed
        if(initStatus != 0) {
            perror("ERROR in pthread_create\n");
            j = 0;
            // cancel others threads
            for (j; j < i; j++) {
                pthread_cancel(threadPool->threads[j]);
                pthread_join(threadPool->threads[j], NULL);
            }
            freeThreadPool(threadPool);
        }
    }
    return threadPool;
}

void destroy(ThreadPool* threadPool) {
    pthread_mutex_destroy(&(threadPool->mutex));
    pthread_cond_destroy(&(threadPool->cond));
    osDestroyQueue(threadPool->tasksQueue);
    freeThreadPool(threadPool);
}

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks) {
    threadPool->isRunning = 0;
    int status;
    threadPool->hasRunningTasks = shouldWaitForTasks;
    status = pthread_cond_broadcast(&threadPool->cond);
    if(status != 0) {
        perror("ERROR in pthread_cond_broadcast\n");
    }
    status = pthread_mutex_unlock(&threadPool->mutex);
    if(status != 0) {
        perror("ERROR in pthread_mutex_unlock\n");
    }
    int i = 0;
    for(; i < threadPool->numOfThreads; i++){
        status = pthread_join(threadPool->threads[i], NULL);
        if (status != 0) {
            sysCallError(threadPool, "Error in pthread_join\n");
        }
    }
    // deQueue all tasks from queue
    int isQueueEmpty = osIsQueueEmpty(threadPool->tasksQueue);
    while(isQueueEmpty == 0) {
        //osDequeue(threadPool->tasksQueue);
        Task *temp = osDequeue(threadPool->tasksQueue);
        free(temp);
        isQueueEmpty = osIsQueueEmpty(threadPool->tasksQueue);
    }
    // finish destroy process
    destroy(threadPool);
    return;
}


int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param){
    int tempStatus;
    // if tpDestroy already have been called
    if(!threadPool) {return -1;}
    Task *task = (Task*) malloc(sizeof(Task));
    if(!task){
        sysCallError(threadPool, "Error in pthread_mutex_lock\n");
    }
    task->param = param;
    task->function = computeFunc;
    tempStatus = pthread_mutex_lock(&(threadPool->mutex));
    if(tempStatus != 0) {
        free(task);
        sysCallError(threadPool, "Error in pthread_mutex_lock\n");
    }
    osEnqueue(threadPool->tasksQueue, task);
    tempStatus = pthread_cond_signal(&threadPool->cond);
    if (tempStatus != 0) {
        free(task);
        sysCallError(threadPool, "Error in pthread_cond_signal\n");
    }
    tempStatus = pthread_mutex_unlock(&(threadPool->mutex));
    if (tempStatus != 0) {
        free(task);
        sysCallError(threadPool, "Error in pthread_mutex_unlock\n");
    }
    return 0;
}