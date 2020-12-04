/*
Name: Rupanshoo Saxena
Roll no. - 2019096
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>


typedef struct{
    int cntr;
    pthread_mutex_t mutexForThreadBlock;   //to lock semaphore
    pthread_mutex_t mutexCounter; 
} my_semaphore;

void initialise(my_semaphore *s, int cntr){   //to initialize the semaphore structure
    if(cntr<=0){
        perror("Initialization failed.\n");
        exit(0);
    }
    else{
        s->cntr = cntr;
        int checkTM = pthread_mutex_init(&(s->mutexForThreadBlock), NULL);
        int checkCM = pthread_mutex_init(&(s->mutexCounter),NULL);

        if(checkTM != 0){perror("Initialization of thread mutex failed!!\n"); exit(0);}
        if(checkCM != 0){perror("Initialization of counter mutex failed!!\n"); exit(0);}

    }
}

void wait(my_semaphore *s){
    pthread_mutex_lock();
}

void signal(my_semaphore *s){

}

void printValue(my_semaphore *s){   //prints current value of semaphore
    pthread_mutex_lock(&(s->mutexCounter));
    printf("Current Semaphore Value is = %d\n", s->cntr);  //lock then print so that value isn't altered in the meanwhile.
    pthread_mutex_unlock(&(s->mutexCounter));
}


int main(){

    int k;
    scanf("Enter no. of philosophers: %d\n", k);

    if(k<=1){
        printf("Invalid k. Exiting...\n");
        exit(0);
    }
}