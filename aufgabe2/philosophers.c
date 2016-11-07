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
 * @brief How many Philosophers will be created
 */
#define PHILOSOPHERS_COUNT 5

/**
 * @brief Threshold for the barrier
 */
#define BARRIER_THRESHOLD 2

/**
 * @brief Counter for the Rest Loop
 */
#define REST_LOOP 1000000000

/**
 * @brief Counter for Workout Loop
 */
#define WORKOUT_LOOP 500000000

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
    PROCEED,          //!< thrwead should proceed to the end of the loop
    QUIT,             //!< thread should finish and end itself
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
    int *weights;
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
 * @brief Barrier, to make sure threads get initialized properly
 */
static pthread_barrier_t barrier;

/**
 * @brief Array to manage all Threads
 */
static Philosopher philos[PHILOSOPHERS_COUNT];

/**
 * @brief Array that contains the training weights for each philosopher
 */
static const int training_weights[PHILOSOPHERS_COUNT] = { 6, 8, 12, 12, 14 };

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
        if (philos[i].weights != NULL) {
            printf("%d(%d)%c:%c:[%d, %d, %d] ", i, training_weights[i],
                    philo_command_names[philos[i].command],
                    philo_state_names[philos[i].state], philos[i].weights[0],
                    philos[i].weights[1], philos[i].weights[2]);

            // accumulate
            weight_acum[0] += philos[i].weights[0];
            weight_acum[1] += philos[i].weights[1];
            weight_acum[2] += philos[i].weights[2];
        } else {
            printf("%d(%d)  UNDEFINED   ", i, training_weights[i]);
        }
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
    // Initialize Barrier
    if (pthread_barrier_init(&barrier, NULL, BARRIER_THRESHOLD)) {
        perror("[philosophers_init] Failed to create barrier");
        exit(EXIT_FAILURE);
    }

    // Initialize all Philosophers
    for (int i = 0; i < PHILOSOPHERS_COUNT; i++) {
        create_philothread(i);
    }

    // Cleanup barrier
    pthread_barrier_destroy(&barrier);
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
int philosopher_unblock(int philo_id) {
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
int philosopher_proceed(int philo_id) {
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
    philos[philo_id].weights = NULL;

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
    pthread_barrier_wait(&barrier);
}

/*
 * Loop for the philosopher threads
 */
static void* philo_loop(void *attributes) {
    // get attributes and store them in local variables
    PhiloAttributes *attr = (PhiloAttributes*) attributes;
    int id = attr->id;
    int total = attr->total;
    int weights[GYM_WEIGHTS_AVAILIABLE_SIZE] = { 0 };
    philos[id].weights = weights;

    // tell main that we finished initialization
    pthread_barrier_wait(&barrier);

    // philoloop "main" code
    while (philos[id].command != QUIT) {
        // GET WEIGHTS
        philos[id].state = GET_WEIGHTS;
        if (philos[id].command == BLOCK) {
            sem_wait(&(philos[id].block_semaphore));
        }
        gym_get_weights(total, weights);

        // WORKOUT
        philos[id].state = WORKOUT;
        for (int i = 0; i < WORKOUT_LOOP; i++) {
            if (philos[id].command == PROCEED || philos[id].command == QUIT) {
                break;
            } else if (philos[id].command == BLOCK) {
                sem_wait(&(philos[id].block_semaphore));
            }
        }

        // PUT WEIGHTS
        philos[id].state = RETURN_WEIGHTS;
        if (philos[id].command == BLOCK) {
            sem_wait(&(philos[id].block_semaphore));
        }
        gym_return_weights(weights);

        // REST
        philos[id].state = REST;
        for (int i = 0; i < REST_LOOP; i++) {
            if (philos[id].command == PROCEED || philos[id].command == QUIT) {
                break;
            } else if (philos[id].command == BLOCK) {
                sem_wait(&(philos[id].block_semaphore));
            }
        }
    }
    // cleanup
    philos[id].weights = NULL;

    return NULL;
}
