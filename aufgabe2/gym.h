/** ****************************************************************
 * @file    aufgabe2/gym.h
 * @author  Moritz Hoewer (Moritz.Hoewer@haw-hamburg.de)
 * @author  Jesko Treffler (Jesko.Treffler@haw-hamburg.de)
 * @version 1.0
 * @date    30.10.2016
 * @brief   Header for the Gym module
 ******************************************************************
 */

#ifndef GYM_H_
#define GYM_H_

#define GYM_WEIGHTS_AVAILIABLE_SIZE 3

/**
 * @brief Initializes the Gym
 *
 * @return error code
 * @retval 0             no error
 * @retval E_MUTEX_ERROR initialization of mutex failed
 */
int gym_init();

/**
 * @brief [MONITOR METHOD] Hands out weights to the caller.
 *
 * This method will check which weights are available and then try to find a
 * combination that sums up to the required total. If it finds one, the weights
 * will be handed out (signaled by a return value of 0). If not, an error code
 * is returned
 *
 * @param[in] total          the desired total weight
 * @param[out] weight_counts will contain the weights if method returned 0
 * @return error code
 * @retval 0 no error
 */
int gym_get_weights(int total, int weight_counts[]);

/**
 * @brief [MONITOR METHOD] Returns weights from the caller to the gym.
 *
 * @param[in] weight_counts array with weights to return
 * @return error code
 * @retval 0 no error
 */
int gym_return_weights(int weight_counts[]);

#endif /* GYM_H_ */
