// Avraham Bar Ilan 205937949

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "threadPool.h"

void* runTask(void* task) {
    ThreadPool* threadPool = (ThreadPool*) task;
    int tempStatus;
    while(threadPool->hasRunningTasks == 1) {
        tempStatus = pthread_mutex_lock(&(threadPool->mutex));
        if (tempStatus != 0) {
            // todo insert in to a function
            perror("Error");
            free(threadPool->threads);
            free(threadPool);
            exit(1);
        } else {
            tempStatus = osIsQueueEmpty(threadPool->tasksQueue);
            // if queue is not empty yet
            if(tempStatus == 0) {
                Task* task = osDequeue(threadPool->tasksQueue);
                tempStatus = pthread_mutex_unlock(&(threadPool->mutex);
                if (tempStatus != 0) {
                    // todo insert in to a function
                    perror("Error");
                    free(threadPool->threads);
                    free(threadPool);
                    exit(1);
                }
                task->function(task->param);
                free(task);

            } else {
                if(threadPool->isRunning == 0) {
                    tempStatus = pthread_mutex_unlock(&(threadPool->mutex);
                    if(tempStatus != 0) {
                        // todo insert in to a function
                        perror("Error");
                        free(threadPool->threads);
                        free(threadPool);
                        exit(1);
                    }
                    break;
                }
                pthread_cond_wait(&(threadPool->cond), &threadPool->mutex);
                tempStatus = pthread_mutex_unlock((&(threadPool->mutex));
                if (tempStatus != 0) {
                    // todo insert in to a function
                    perror("Error in unlock\n");
                    free(threadPool->threads);
                    free(threadPool);
                    exit(1);
                }
            }
        }
    }
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
    // todo insert into a function
    threadPool->threads = threads;
    threadPool->tasksQueue = queue;
    threadPool->numOfThreads = numOfThreads;
    threadPool->isRunning = 1;
    threadPool->hasRunningTasks = 1; // todo maybe remove

    int initStatus = pthread_mutex_init(&threadPool->mutex, NULL);
    if(initStatus != 0) {
        perror("ERROR");
    }

    initStatus = pthread_cond_init(&threadPool->cond, NULL);
    if(initStatus != 0) {
        perror("ERROR");
    }

    int i = 0, j;
    for(i; i < numOfThreads; i++) {
        initStatus = pthread_create(&(threadPool->threads[i]), NULL, runTask, (void*)threadPool);
        // if pthread create failed
        if(initStatus != 0) {
            perror("ERROR");
            j = 0;
            // cancel others threads
            for (j; j < i; j++) {
                pthread_cancel(threadPool->threads[j]);
                pthread_join(threadPool->threads[j], NULL);
            }
            // todo insert into a function
            free(threadPool->threads);
            free(threadPool);
        }
    }
    return threadPool;
}


void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks) {
    int status;
    threadPool->hasRunningTasks = shouldWaitForTasks;
    status = pthread_cond_broadcast(&threadPool->cond);
    if(status != 0) {
        perror("ERROR");
    }
    status = pthread_mutex_unlock(&threadPool->mutex);
    if(status != 0) {
        perror("ERROR");
    }
    int i = 0;
    for(i; i < threadPool->numOfThreads; i++){
        status = pthread_join(threadPool->threads[i], NULL);
        if (status != 0) {
            // todo insert into a function
            free(threadPool->threads);
            free(threadPool);
            perror("ERROR");
            exit(1);
        }
    }
    // deQueue all tasks from queue
    int isQueueEmpty = osIsQueueEmpty(threadPool->tasksQueue);
    while(isQueueEmpty != 0 ) {
        osDequeue(threadPool->tasksQueue);
        /*Task *temp = osDequeue(threadPool->tasksQueue);
        free(temp;*/
        isQueueEmpty = osIsQueueEmpty(threadPool->tasksQueue);
    }
    pthread_mutex_destroy(&(threadPool->mutex));
    pthread_cond_destroy(&(threadPool->cond));
    osDestroyQueue(threadPool->tasksQueue);
    // todo insert into a function
    free(threadPool->threads);
    free(threadPool);
    return;
}


int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param){
    int tempStatus;
    // if tpDestroy already have been called
    if(!threadPool) {return -1;}
    Task *task = (Task*) malloc(sizeof(Task));
    if(!task){
        // todo insert into a function
        free(threadPool->threads);
        free(threadPool);
        perror("ERROR");
        exit(1); // todo maybe should not be here exit()
    }
    task->param = param;
    task->function = computeFunc;
    tempStatus = pthread_mutex_lock(&(threadPool->mutex));
    if(tempStatus != 0) {
        // todo insert into a function
        free(task);
        free(threadPool->threads);
        free(threadPool);
        perror("Error");
        exit(1);
    }

    // todo maybe should do like line 105
    osEnqueue(threadPool->tasksQueue, task);
    tempStatus = pthread_cond_signal(&threadPool->cond);
    if (tempStatus != 0) {
        // todo insert into a function
        free(task);
        free(threadPool->threads);
        free(threadPool);
        perror("Error");
        exit(1);
    }
    return 0;
}