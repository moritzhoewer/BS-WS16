/* critsect02 -- with semaphores */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>


#define NLOOP 100
#define NIDLE_1 300000
#define NIDLE_2 250000
#define TVALUE_1 1
#define TVALUE_2 2


void *thread2(void *arg);
sem_t mutex;
int value = 0;
int display_value = 0;



int main(void) {
    int res = 0;
    int loop = 0;
    char* thread_msg = "Thread 1: ";
    pthread_t tid_2;
    void *thread_result;

    res = sem_init(&mutex, 0, 1);
    if(res != 0){
       perror("Semaphore initialization failed");
       exit(EXIT_FAILURE);
    }
    res = pthread_create(&tid_2, NULL, thread2, NULL);
    if (res != 0) {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }

   for(loop=0; loop < NLOOP; loop++)
   {
/*       long idleloop = 0; */
      /* Critical section STARTS here */
      sem_wait(&mutex);
      value = TVALUE_1;
/*       for (idleloop = 0; idleloop < NIDLE_1; idleloop++){} */
      display_value = value;
      printf("%sValue = %d\n", thread_msg, display_value);
      fflush (stdout);
      sem_post(&mutex);
      /* Critical setion ENDS here */
      sleep(rand()%3);
   }
    
    
    printf("\nWaiting for thread to finish...\n");
    res = pthread_join(tid_2, &thread_result);
    if (res != 0) {
        perror("Thread join failed");
        exit(EXIT_FAILURE);
    }
    printf("Thread joined\n");
    sem_destroy(&mutex);
    exit(EXIT_SUCCESS);
}

void *thread2(void *arg) {
   int loop = 0;
/*    long idleloop = 0; */
   char* thread_msg = "Thread 2: ";
   
   for(loop=0; loop < NLOOP; loop++)
   {
      /* Critical section STARTS here */
      sem_wait(&mutex);
      value = TVALUE_2;
/*       for (idleloop = 0; idleloop < NIDLE_2; idleloop++){} */
      display_value =  value;
      printf("%sValue = %d\n", thread_msg, display_value);
      fflush (stdout);
      sem_post(&mutex);
      /* Critical section ENDS here */
      sleep(rand()%4);
   }

   pthread_exit(NULL);
}
