/** ****************************************************************
 * @file    aufgabe2/philosophers.c
 * @author  Moritz Hoewer (Moritz.Hoewer@haw-hamburg.de)
 * @author  Jesko Treffler (Jesko.Treffler@haw-hamburg.de)
 * @version 1.0
 * @date    07.11.2016
 * @brief   Implementation of the Philosophers Module
 ******************************************************************
 */
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#include "philosophers.h"
#include "errors.h"
#include "gym.h"


/**
 * @brief Enum for the possible states of a philosopher
 */
typedef enum {
    GET_WEIGHTS,    //!< getting weights from the gym
    WORKOUT,        //!< working out
    RETURN_WEIGHTS, //!< returning weights to the gym
    REST,           //!< resting / thinking
    UNDEFINED       //!< state is undefined or unknown
} PhiloState;
static const char philo_state_names[] = { 'G', 'W', 'P', 'R', 'U' };

/**
 * @brief Enum for the possible commands to a philosopher
 */
typedef enum {
    NORMAL_EXECUTION, //!< no special command, just do what you are supposed to
    BLOCK,            //!< thread should block
    PROCEED,          //!< thread should proceed to the end of the loop
    QUIT             //!< thread should finish and end itself
} PhiloCommand;
static const char philo_command_names[] = { 'n', 'b', 'p', 'q' };

/**
 * @brief Structure to represent and manage a Philosopher
 */
typedef struct {
    PhiloCommand command;
    PhiloState state;
    pthread_t id;
    sem_t block_semaphore;
    int weights[GYM_WEIGHTS_AVAILIABLE_SIZE];
} Philosopher;

/**
 * @brief Structure to pass to new philosopher thread
 */
typedef struct {
    int id;
    int total;
} PhiloAttributes;

/**
 * @brief Loop for the philosopher threads
 *
 * @param attributes the id of the philosopher (PhiloAttributes)
 */
static void* philo_loop(void *attributes);

/**
 * @brief Creates a new philosopher thread and initializes it
 *
 * @param philo_id  the id of the philosopher
 */
static void create_philothread(int philo_id);

/**
 * @brief Barrier, to make sure threads get their arguments properly
 */
static pthread_barrier_t arg_barrier;

/**
 * @brief Barrier, to make sure all threads get initialized
 */
static pthread_barrier_t init_barrier;

/**
 * @brief Array to manage all Threads
 */
static Philosopher philos[PHILOSOPHERS_COUNT];

/**
 * @brief Array that contains the training weights for each philosopher
 */
static const int training_weights[PHILOSOPHERS_COUNT] = { PHILOSOPHERS_WEIGHTS };

/*
 * Displays the current status on the screen
 *
 * Will print state and held weights information for all philosophers.
 */
void philosophers_display_status(const int gym_weights[]) {
    // initialize accumulator for weights
    int weight_acum[GYM_WEIGHTS_AVAILIABLE_SIZE] = { 0 };

    // print information for each philosopher
    for (int i = 0; i < PHILOSOPHERS_COUNT; i++) {
		printf("%d(%d)%c:%c:[%d, %d, %d] ", i, training_weights[i],
			philo_command_names[philos[i].command],
			philo_state_names[philos[i].state], philos[i].weights[0],
			philos[i].weights[1], philos[i].weights[2]);
			
		// accumulate
		weight_acum[0] += philos[i].weights[0];
		weight_acum[1] += philos[i].weights[1];
		weight_acum[2] += philos[i].weights[2];
   
    }

    // print information about gym
    printf("Gym: [%d, %d, %d]\n", gym_weights[0], gym_weights[1],
            gym_weights[2]);
    // accumulate
    weight_acum[0] += gym_weights[0];
    weight_acum[1] += gym_weights[1];
    weight_acum[2] += gym_weights[2];

    // check validity
    if (weight_acum[0] != gym_weights_availiable[0]
            || weight_acum[1] != gym_weights_availiable[1]
            || weight_acum[2] != gym_weights_availiable[2]) {
        perror("Sync Failure!!");
        exit(EXIT_FAILURE);
    }
}

/*
 * Initializes the philosophers
 */
void philosophers_init() {
    // Initialize arg_barrier
    if (pthread_barrier_init(&arg_barrier, NULL, BARRIER_THRESHOLD)) {
        perror("[philosophers_init] Failed to create arg_barrier");
        exit(EXIT_FAILURE);
    }
    
    // Initialize init_barrier
    // threshold is PHILOSOPHERS_COUNT + 1 because of main thread
    if (pthread_barrier_init(&init_barrier, NULL, PHILOSOPHERS_COUNT + 1)) {
        perror("[philosophers_init] Failed to create init_barrier");
        exit(EXIT_FAILURE);
    }

    // Initialize all Philosophers
    for (int i = 0; i < PHILOSOPHERS_COUNT; i++) {
        create_philothread(i);
    }
    
    // Wait for initialization of all threads
    pthread_barrier_wait(&init_barrier);

    // Cleanup barriers
    pthread_barrier_destroy(&arg_barrier);
    pthread_barrier_destroy(&init_barrier);
}

/*
 * Stops all the philosopher threads
 */
void philosophers_quit() {
    // Send quit command to all Philosophers
    for (int i = 0; i < PHILOSOPHERS_COUNT; i++) {
        philos[i].command = QUIT;
    }

    // Wakeup all philosophers
    for (int i = 0; i < PHILOSOPHERS_COUNT; i++) {
        sem_post(&(philos[i].block_semaphore));
    }

    // Wait for all philosophers to quit
    for (int i = 0; i < PHILOSOPHERS_COUNT; i++) {
        pthread_join(philos[i].id, NULL);
    }

    // Cleanup
    for (int i = 0; i < PHILOSOPHERS_COUNT; i++) {
        sem_destroy(&(philos[i].block_semaphore));
    }
}

/*
 * Blocks the specified philosopher
 */
int philosophers_block(int philo_id) {
    if (philo_id < 0 || philo_id >= PHILOSOPHERS_COUNT) {
        return E_NO_SUCH_PHILOSOPHER;
    }

    // send block command
    philos[philo_id].command = BLOCK;

    return 0;
}

/*
 * Unblocks the specified philosopher
 */
int philosophers_unblock(int philo_id) {
    if (philo_id < 0 || philo_id >= PHILOSOPHERS_COUNT) {
        return E_NO_SUCH_PHILOSOPHER;
    }

    // clear  block command
    if (philos[philo_id].command == BLOCK) {
        philos[philo_id].command = NORMAL_EXECUTION;
    }

    // wakeup thread
    sem_post(&(philos[philo_id].block_semaphore));
    return 0;
}

/*
 * Proceeds the specified philosopher behind the current loop
 */
int philosophers_proceed(int philo_id) {
    if (philo_id < 0 || philo_id >= PHILOSOPHERS_COUNT) {
        return E_NO_SUCH_PHILOSOPHER;
    }

    // send proceed command
    philos[philo_id].command = PROCEED;

    return 0;
}

/*
 * Creates a new philosopher thread and initializes it
 */
static void create_philothread(int philo_id) {
    // initialize
    philos[philo_id].command = NORMAL_EXECUTION;
    philos[philo_id].state = UNDEFINED;
    
    for(int i = 0; i < GYM_WEIGHTS_AVAILIABLE_SIZE; i++){
		philos[philo_id].weights[i] = 0;
	}

    // initialize condition variable
    if (sem_init(&(philos[philo_id].block_semaphore), 0, 0)) {
        perror("[create_philothread] Failed to initialize semaphore");
        exit(EXIT_FAILURE);
    }

    // wrap options in struct
    PhiloAttributes attr;
    attr.id = philo_id;
    attr.total = training_weights[philo_id];

    // create thread
    if (pthread_create(&(philos[philo_id].id), NULL, philo_loop, &attr)) {
        perror("[create_philothread] Failed to create thread");
        exit(EXIT_FAILURE);
    }

    // Wait until thread has initialized itself
    pthread_barrier_wait(&arg_barrier);
}


static void philo_block(int id){
	 if (philos[id].command == BLOCK) {
            sem_wait(&(philos[id].block_semaphore));
        }
}
static void philo_idle(int time, int id){
	 for (int i = 0; i < time; i++) {
            if(philos[id].command == QUIT) {
				break;
			}
            else if (philos[id].command == PROCEED){
				philos[id].command = NORMAL_EXECUTION;
                break;
            } else philo_block(id);
            
        }
	
}
/*
 * 
 * Loop for the philosopher threads
 */

static void philo_workout(int id){
	philos[id].state = WORKOUT;
    philo_idle(WORKOUT_LOOP, id);
}
static void philo_rest(int id){
	philos[id].state = REST;
    philo_idle(REST_LOOP,id);
}
static void get_weight(int id,int total){
	philos[id].state = GET_WEIGHTS;
    philo_block(id);
    gym_get_weights(total, philos[id].weights);
}

static void put_weights(int id){
	philos[id].state = RETURN_WEIGHTS;
    philo_block(id);
    gym_return_weights(philos[id].weights);
}

static void* philo_loop(void *attributes) {
    // get attributes and store them in local variables
    PhiloAttributes *attr = (PhiloAttributes*) attributes;
    int id = attr->id;
    int total = attr->total;

    // tell create_philothread that we finished initialization
    pthread_barrier_wait(&arg_barrier);
    
    // Wait for initialization of all threads
    pthread_barrier_wait(&init_barrier);

    // philoloop "main" code
    while (philos[id].command != QUIT) { 
        get_weight(id,total);
        philo_workout(id);
        put_weights(id);
        philo_rest(id); 
    }

    return NULL;
}
