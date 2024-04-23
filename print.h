#include <stdio.h> 
#include <pthread.h> 
#include <semaphore.h> 
#include <unistd.h> 

#ifndef PRINT_H
#define PRINT_H
// semaphored print
sem_t sem_print;
void pr_init(){}
void print(char * msg){
    sem_wait(&sem_print); 
    printf(msg);
    fflush(stdout);
    sem_post(&sem_print);
}

void pr_deinit(){
    sem_destroy(&sem_print);
}

#endif