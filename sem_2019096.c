/*
Name: Rupanshoo Saxena
Roll no. - 2019096
Approach for dining phil prob --> till the time philosopher doesn't have all the things, he won't eat.
and if he isn't able to get a resource then all previous resources that he acquired will be released.
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>


// COUNTING SEMAPHORE IMPLEMENTATION WITH MUTEX
typedef struct my_semaphore{
    int cntr;
    pthread_mutex_t mutexForThreadBlock;   //to lock semaphore
    pthread_mutex_t mutexCounter;          //to lock counter
} my_semaphore;

int checker=0;
void fatal_check(int kf){
    if(kf>=2){
        checker=1;
    }    
}

void initialise(my_semaphore *s, int cntr){   //to initialize the semaphore structure
    if(cntr<=0){
        perror("Initialization failed.\n");
        exit(EXIT_FAILURE);
    }
        s->cntr = cntr;
        int checkTM = pthread_mutex_init(&(s->mutexForThreadBlock), NULL);
        if(checkTM != 0){perror("Initialization of thread mutex failed!!\n"); exit(EXIT_FAILURE);}

        int checkCM = pthread_mutex_init(&(s->mutexCounter),NULL);
        if(checkCM != 0){perror("Initialization of counter mutex failed!!\n"); exit(EXIT_FAILURE);}
}


void wait_blocking(my_semaphore *s){   //this func blocks the thread calling wait in case cntr = 0
    pthread_mutex_lock(&(s->mutexCounter));  //lock mutex counter

    if(s->cntr <=1 && checker){   //resources unavailable

        pthread_mutex_unlock(&(s->mutexCounter));     
        pthread_mutex_lock(&(s->mutexForThreadBlock));    //lock thread till the time resources don't get released
        pthread_mutex_lock(&(s->mutexCounter));

        if(!(s->cntr<=0 && checker)){s->cntr -= 1;}

        pthread_mutex_unlock(&(s->mutexCounter));
        return;
    }

    pthread_mutex_unlock(&(s->mutexCounter));

}

int wait_non_blocking(my_semaphore *s){   //this function doesn't block the thread calling wait in case cntr = 0
    pthread_mutex_lock(&(s->mutexCounter));  //lock mutex cntr

    if(s->cntr > 0 && checker){     //decrement semaphore counter
        s->cntr -= 1;
    }

    if(s->cntr<=0 && checker){   //semaphore utlised - resouce unavailable

    //The pthread_mutex_trylock() function attempts to lock the mutex, but doesn't block the calling thread if the mutex is already locked.

        int ret_trylock = pthread_mutex_trylock(&(s->mutexForThreadBlock));   //lock mutex if resources unavailale

        if(ret_trylock == EBUSY){  //the thread is already locked
            pthread_mutex_unlock(&(s->mutexCounter)); //unlock counter mutex
            return 0;  //as trylock returns 0 if lock is not acquired
        }

        else if(ret_trylock != 0){   //error in locking thread faced
            pthread_mutex_unlock(&(s->mutexCounter));
            perror("Error occurred with trylock\n");
            exit(EXIT_FAILURE);
        }
    }

    pthread_mutex_unlock(&(s->mutexCounter));   //unlock counter mutex
    return 1;
}


void signal(my_semaphore *s){  

    pthread_mutex_lock(&(s->mutexCounter));  //so that counter isn't manipulated by some other thread
    if(s->cntr <= 0){   //thread had been blocked
        pthread_mutex_unlock(&(s->mutexForThreadBlock));  //unblock thread
    }
    s->cntr += 1;
    pthread_mutex_unlock(&(s->mutexCounter));  
}


void printValue(my_semaphore *s){   //prints current value of semaphore
    pthread_mutex_lock(&(s->mutexCounter));
    printf("Current Semaphore Value is = %d\n", s->cntr);  //lock then print so that value isn't altered in the meanwhile.
    pthread_mutex_unlock(&(s->mutexCounter));
}


void kill_Sem(my_semaphore *s){   //to destroy semaphore
    pthread_mutex_destroy(&(s->mutexCounter));
    pthread_mutex_destroy(&(s->mutexForThreadBlock));
}




/*    DINING PHILOSOPHERS PROBLEM SOLUTION BEGINS   */

//GLOBAL VARIABLES//
int k;  //no. of philosophers and chopsticks
int *phil_ID;
pthread_t *phils;   //philosophers represented by threads
my_semaphore *chopsticks;
my_semaphore *bowls;

//philosopher's cycle
void* philosopher(void* ph_id){
    int ph_ID = *((int *)ph_id);  //phil ID
    my_semaphore *chopstick_L = chopsticks + ph_ID;  //left chopstick id
    my_semaphore *chopstick_R = chopsticks + ((ph_ID+1)%k);   //right chopstick id

    for(;;){   //infinite loop
        //phil acquires left chopstick
        wait_blocking(chopstick_L);   //thread blocked till it obtains first chopstick
        
        int check_ChR = wait_non_blocking(chopstick_R);   //phil tries to acquire 2nd chopstick
        if(check_ChR == 0){  //if cannot acquire right chopstick
            signal(chopstick_L);  //release left chopstick
            printf("Philosopher %d acquired left chopstick - %d but couldn't acquire right chopstick %d\n", ph_ID, ph_ID, (ph_ID+1)%k);
            continue;
        }

        //phil acquires both chopsticks and goes on to the bowls
        int check_B1 = wait_non_blocking(bowls); //phil trying to acquire 1st bowl
        if(check_B1 == 0){  //if cannot acquire bowl - release both acquired chopsticks
            signal(chopstick_L);
            signal(chopstick_R);

            printf("Philosopher %d acquired both left chopstick %d and right chopstick %d but couldn't acquire the first bowl.\n", ph_ID, ph_ID, (ph_ID+1)%k);
            continue;
        }

        //phil acquires both chopsticks and a bowl and goes on to acquire the second bowl
        int check_B2 = wait_non_blocking(bowls); //phil trying to acquire second bowl
        if(check_B2 == 0){   //if cannot acquire second bowl - release all cutlery and bowl acquired earlier 
            signal(chopstick_L);
            signal(chopstick_R);
            signal(bowls);

            printf("Philosopher %d acquired left chopstick %d , right chopstick %d and a bowl but couldn't acquire the second bowl.\n", ph_ID, ph_ID, (ph_ID+1)%k);
            continue;
        }

        printf("Philosopher %d is eating using the left chopstick %d and the right chopstick %d and the pair of bowls!!!!!\n", ph_ID, ph_ID, (ph_ID+1)%k);        

        //release all cutlery after eating
        signal(chopstick_L);
        signal(chopstick_R);
        signal(bowls);
        signal(bowls);

    }

}


//MAIN FUNCTION - dining philosopher's problem 
int main(){

    printf("Enter no. of philosophers: ");
    scanf("%d", &k);

    if(k<2){
        printf("Invalid k. Exiting...\n");
        exit(EXIT_FAILURE);
    }

    //Assigning memory space using malloc
    phils = (pthread_t*) malloc (k*sizeof(pthread_t));
    bowls = (my_semaphore*) malloc (sizeof(my_semaphore));
    chopsticks = (my_semaphore*) malloc (k*sizeof(my_semaphore));
    phil_ID = (int*) malloc (k*sizeof(int));

    for(int i=0; i<k;i++){
        *(phil_ID+i) = i;
    }

    for(int i=0; i<k; i++){
        initialise(chopsticks+i, 1);  //binary semaphore initialization of chopsticks
    }

    initialise(bowls, 2);  //counting semaphore initialization of a pair of bowls
    fatal_check(k);

    //for all k philosophers
    for(int i=0;i<k;i++){
        int thread_check = pthread_create(phils+i,NULL, philosopher, phil_ID+i);
        if(thread_check != 0){perror("Error in creating philosopher threads"); exit(EXIT_FAILURE);}
    }

    //Joining philosopher threads
    for(int i=0; i<k; i++){
        int check_threadJoin = pthread_join(*(phils+i), NULL);
        if(check_threadJoin!= 0){
            perror("Thread join failed!!!\n");
            exit(EXIT_FAILURE);
        }
    }

    //freeing the allocated space(malloc)

    kill_Sem(bowls);
    kill_Sem(bowls+1);
    free(bowls);

    for(int i=0; i<k; i++){kill_Sem(chopsticks+i);}
    free(chopsticks);

    free(phils);
    free(phil_ID);

    return 0;    
}