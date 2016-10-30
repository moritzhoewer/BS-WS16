/** ****************************************************************
 * @file    aufgabe2/gym.c
 * @author  Moritz Hoewer (Moritz.Hoewer@haw-hamburg.de)
 * @author  Jesko Treffler (Jesko.Treffler@haw-hamburg.de)
 * @version 1.0
 * @date    30.10.2016
 * @brief   Implementation for the Gym module
 ******************************************************************
 */

#include "gym.h"
#include "errors.h"

/*
 * Initializes the Gym
 */
int init_gym(int weights[]){
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
int get_weights(int total, int weights[]){
    return 0;
}

/*
 * [MONITOR METHOD] Returns weights from the caller to the gym.
 */
int return_weights(int weights[]){
    return 0;
}
