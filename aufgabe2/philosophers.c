/** ****************************************************************
 * @file    aufgabe2/philosophers.c
 * @author  Moritz Hoewer (Moritz.Hoewer@haw-hamburg.de)
 * @author  Jesko Treffler (Jesko.Treffler@haw-hamburg.de)
 * @version 1.0
 * @date    14.11.2016
 * @brief   Implementation of the Philosophers Module
 ******************************************************************
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

#include "philosophers.h"
#include "errors.h"
#include "gym.h"

/* ****************************************************************************
 * constants
 *************************************************************************** */

/**
 * @brief Threshold for the arg_barrier
 */
#define ARG_BARRIER_THRESHOLD 2

/**
 * @brief Threshold for the init_barrier
 */
#define INIT_BARRIER_THRESHOLD (PHILOSOPHERS_COUNT + 1)

/**
 * @brief the semapore is not shared between processes
 */
#define SEM_NOT_SHARED 0

/**
 * @brief The initila value for the semaphore
 */
#define SEM_INITIAL_VALUE 0

/**
 * @brief Counter for the Rest Loop
 */
#define REST_LOOP 1000000000

/**
 * @brief Counter for Workout Loop
 */
#define WORKOUT_LOOP 500000000

/* ****************************************************************************
 * static type definitions
 *************************************************************************** */

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
    QUIT              //!< thread should finish and end itself
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

/* ****************************************************************************
 * static function definitions
 *************************************************************************** */
/**
 * @brief Loop for the philosopher threads
 *
 * @param arg the id of the philosopher (int)
 */
static void* philo_loop(void *arg);

/**
 * @brief Creates a new philosopher thread and initializes it
 *
 * @param philo_id  the id of the philosopher
 */
static void create_philothread(int philo_id);

/**
 * @brief Checks if this thread should block
 *
 * If yes, then it blocks on the associated semaphore
 *
 * @param id the thread id
 */
static void check_block(int id);

/**
 * @brief Busy witing loop
 *
 * Executes a loop time times, checking command flag on every iteration.
 *
 * @param time the amount of iterations
 * @param id   the thread id (for flag checking)
 */
static void idle_loop(int time, int id);

/**
 * @brief Workout phase
 *
 * @param id the thread id
 */
static void workout(int id);

/**
 * @brief Resting phase
 *
 * @param id the thread id
 */
static void rest(int id);

/**
 * @brief Getting weights from gym phase
 *
 * @param id the thread id
 */
static void get_weights(int id);

/**
 * @brief Returning weights to gym phase
 *
 * @param id the thread id
 */
static void put_weights(int id);

/* ****************************************************************************
 * global variables
 *************************************************************************** */

/**
 * @brief Barrier, to make sure threads get their arguments properly
 */
static pthread_barrier_t arg_barrier;

/**
 * @brief Barrier, to make sure all threads get initialized before the first
 *        one starts.
 */
static pthread_barrier_t init_barrier;

/**
 * @brief Array to manage all threads
 */
static Philosopher philos[PHILOSOPHERS_COUNT];

/**
 * @brief Array that contains the training weights for each philosopher
 */
static const int training_weights[PHILOSOPHERS_COUNT] = { PHILOSOPHERS_WEIGHTS };

/* ****************************************************************************
 * function declarations
 *************************************************************************** */

/*
 * Displays the current status on the screen
 *
 * Will print state and held weights information for all philosophers and the
 * gym.
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
    bool invalid = weight_acum[0] != gym_weights_availiable[0]
            || weight_acum[1] != gym_weights_availiable[1]
            || weight_acum[2] != gym_weights_availiable[2];
    FATAL_ERROR_HANDLING(invalid, "Synchronization error!")
}

/*
 * Initializes the philosophers
 */
void philosophers_init() {
    // Initialize arg_barrier
    int res = pthread_barrier_init(&arg_barrier, NULL, ARG_BARRIER_THRESHOLD);
    FATAL_ERROR_HANDLING(res,
            "[philosophers_init] Failed to create arg_barrier")

    // Initialize init_barrier
    res = pthread_barrier_init(&init_barrier, NULL, INIT_BARRIER_THRESHOLD);
    FATAL_ERROR_HANDLING(res,
            "[philosophers_init] Failed to create init_barrier")

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

    for (int i = 0; i < GYM_WEIGHTS_AVAILIABLE_SIZE; i++) {
        philos[philo_id].weights[i] = 0;
    }

    // initialize condition variable
    int res = sem_init(&(philos[philo_id].block_semaphore), SEM_NOT_SHARED,
    SEM_INITIAL_VALUE);
    FATAL_ERROR_HANDLING(res,
            "[create_philothread] Failed to initialize semaphore")

    // create thread
    res = pthread_create(&(philos[philo_id].id), NULL, philo_loop, &philo_id);
    FATAL_ERROR_HANDLING(res, "[create_philothread] Failed to create thread")

    // Wait until thread has initialized itself
    pthread_barrier_wait(&arg_barrier);
}

/*
 *
 * Loop for the philosopher threads
 */
static void* philo_loop(void *arg) {
    // get id and store it in local variables
    int id = *((int*) arg);

    // tell create_philothread that we finished initialization
    pthread_barrier_wait(&arg_barrier);

    // Wait for initialization of all threads
    pthread_barrier_wait(&init_barrier);

    // philoloop "main" code
    while (philos[id].command != QUIT) {
        get_weights(id);
        workout(id);
        put_weights(id);
        rest(id);
    }

    return NULL;
}

/*
 * check if thread should block
 */
static void check_block(int id) {
    if (philos[id].command == BLOCK) {
        sem_wait(&(philos[id].block_semaphore));
    }
}

/*
 * busy waiting loop
 */
static void idle_loop(int time, int id) {
    // busy waiting loop that checks command flags
    for (int i = 0; i < time; i++) {
        if (philos[id].command == QUIT) {
            break;
        } else if (philos[id].command == PROCEED) {
            philos[id].command = NORMAL_EXECUTION;
            break;
        } else {
            check_block(id);
        }
    }
}
/*
 * workout phase
 */
static void workout(int id) {
    philos[id].state = WORKOUT;
    idle_loop(WORKOUT_LOOP, id);
}

/*
 * resting phase
 */
static void rest(int id) {
    philos[id].state = REST;
    idle_loop(REST_LOOP, id);
}

/*
 * getting weights from the gym phase
 */
static void get_weights(int id) {
    philos[id].state = GET_WEIGHTS;
    check_block(id);
    gym_get_weights(training_weights[id], philos[id].weights);
}

/*
 * returning weights to the gym phase
 */
static void put_weights(int id) {
    philos[id].state = RETURN_WEIGHTS;
    check_block(id);
    gym_return_weights(philos[id].weights);
}
