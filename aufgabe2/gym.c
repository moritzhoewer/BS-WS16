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
#include "gym.h"
#include "errors.h"

static pthread_mutex_t mtx;
static int weights_availiable[GYM_WEIGHTS_AVAILIABLE_SIZE] = { 4, 4, 5 };
static int weight_masses[GYM_WEIGHTS_AVAILIABLE_SIZE] = { 2, 3, 5 };

/**
 * @brief Checks whether the amount specified can be achieved
 *
 * Assumes that weights is empty and has the same size and structure as
 * weights_availiable
 *
 * @param[in]  total         the total weight requested
 * @param[out] weights       will contain weights if message returns true
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

    if (total < weight_masses[current_index] || weights_availiable[current_index] == 0) {
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
int gym_init() {
    if (pthread_mutex_init(&mtx, NULL) != 0) {
        return E_MUTEX_ERROR;
    }
    return 0;
}

/*
 * [MONITOR METHOD] Hands out weights to the caller.
 *
 * This method will check which weights are available and then try to find a
 * combination that sums up to the required total. If it finds one, the weights
 * will be handed out (signaled by a return value of 0). If not, an error code
 * is returned
 */
int gym_get_weights(int total, int weight_counts[]) {
    if (pthread_mutex_lock(&mtx) != 0) {
        return E_MUTEX_ERROR;
    }
    int result = E_NO_WEIGHTS;
    if (is_combination_possible(total, weight_counts,
            GYM_WEIGHTS_AVAILIABLE_SIZE - 1)) {
        result = 0;
    }
    pthread_mutex_unlock(&mtx);
    return result;
}

/*
 * [MONITOR METHOD] Returns weights from the caller to the gym.
 */
int gym_return_weights(int weight_counts[]) {
    if (pthread_mutex_lock(&mtx) != 0) {
        return E_MUTEX_ERROR;
    }
    for(int i = 0; i < GYM_WEIGHTS_AVAILIABLE_SIZE; i++){
        // "transfer" weights to gym
        weights_availiable[i] += weight_counts[i];
        weight_counts[i] = 0;
    }
    pthread_mutex_unlock(&mtx);
    return 0;
}
