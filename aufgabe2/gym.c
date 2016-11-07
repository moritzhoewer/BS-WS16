/** ****************************************************************
 * @file    aufgabe2/gym.c
 * @author  Moritz Hoewer (Moritz.Hoewer@haw-hamburg.de)
 * @author  Jesko Treffler (Jesko.Treffler@haw-hamburg.de)
 * @version 1.0
 * @date    30.10.2016
 * @brief   Implementation for the Gym module
 ******************************************************************
 */

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "gym.h"
#include "philosophers.h"
#include "errors.h"

/**
 * @brief Mutex used to guard Monitor methods
 */
static pthread_mutex_t mtx;

/**
 * @brief Condition variable
 */
static pthread_cond_t no_weights;

/*
 * Array used to store how many of the different kinds of weights are
 *        available
 */
const int gym_weights_availiable[GYM_WEIGHTS_AVAILIABLE_SIZE] = { 4, 4, 5 };

/**
 * @brief Array used to store how many of the different kinds of weights are
 *        available
 */
static int weights_availiable[GYM_WEIGHTS_AVAILIABLE_SIZE];

/**
 * @brief The mass of the different kinds of weights
 */
static int weight_masses[GYM_WEIGHTS_AVAILIABLE_SIZE] = { 2, 3, 5 };

/**
 * @brief Checks whether the amount specified can be achieved
 *
 * Assumes that weights is empty and has the same size and structure as
 * weights_availiable
 *
 * @param[in]  total         the total weight requested
 * @param[out] weight_counts will contain weights if message returns true
 * @param[in]  current_index the current index to search for weights
 * @return true, if total can be achieved
 */
static bool is_combination_possible(int total, int weight_counts[],
        int current_index) {
    if (current_index < 0 || current_index >= GYM_WEIGHTS_AVAILIABLE_SIZE) {
        return false;
    }
    if (total == 0) {
        return true;
    }

    if (total < weight_masses[current_index]
            || weights_availiable[current_index] == 0) {
        return is_combination_possible(total, weight_counts, current_index - 1);
    } else {
        // "transfer" 1 weight from gym to philosopher
        total -= weight_masses[current_index];
        weight_counts[current_index]++;
        weights_availiable[current_index]--;

        if (is_combination_possible(total, weight_counts, current_index)) {
            return true;
        } else {
            // "transfer" 1 weight back from philosopher to gym
            total += weight_masses[current_index];
            weight_counts[current_index]--;
            weights_availiable[current_index]++;

            // try again one weight class deeper
            return is_combination_possible(total, weight_counts,
                    current_index - 1);
        }
    }
}

/*
 * Initializes the Gym
 */
void gym_init() {
    // init weight counts
    for(int i = 0; i < GYM_WEIGHTS_AVAILIABLE_SIZE; i++){
        weights_availiable[i] = gym_weights_availiable[i];
    }

    // init monitor
    if (pthread_mutex_init(&mtx, NULL)) {
        perror("[gym_init] Failed to create Mutex");
        exit(EXIT_FAILURE);
    }
    if (pthread_cond_init(&no_weights, NULL)) {
        perror("[gym_init] Failed to create condition variable");
        exit(EXIT_FAILURE);
    }
}

/*
 * [MONITOR METHOD] Hands out weights to the caller.
 *
 * This method will check which weights are available and then try to find a
 * combination that sums up to the required total. If it finds one, the weights
 * will be handed out (signaled by a return value of 0). If not, an error code
 * is returned
 */
void gym_get_weights(int total, int weight_counts[]) {
    if (pthread_mutex_lock(&mtx)) {
        perror("[gym_get_weights] Failed to lock mutex");
        exit(EXIT_FAILURE);
    }

    philosophers_display_status(weights_availiable);

    while (!is_combination_possible(total, weight_counts,
    GYM_WEIGHTS_AVAILIABLE_SIZE - 1)) {
        if (pthread_cond_wait(&no_weights, &mtx)) {
            perror("[gym_get_weights] Wait on condition variable failed");
            exit(EXIT_FAILURE);
        }
    }

    philosophers_display_status(weights_availiable);

    pthread_mutex_unlock(&mtx);
}

/*
 * [MONITOR METHOD] Returns weights from the caller to the gym.
 */
void gym_return_weights(int weight_counts[]) {
    if (pthread_mutex_lock(&mtx)) {
        perror("[gym_return_weights] Failed to lock mutex");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < GYM_WEIGHTS_AVAILIABLE_SIZE; i++) {
        // "transfer" weights to gym
        weights_availiable[i] += weight_counts[i];
        weight_counts[i] = 0;
    }

    philosophers_display_status(weights_availiable);

    // notify threads who might be blocking on the condition
    pthread_cond_broadcast(&no_weights);
    pthread_mutex_unlock(&mtx);
}
