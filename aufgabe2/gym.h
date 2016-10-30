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

/**
 * @brief Initializes the Gym
 *
 * @param[in] weights array with weights
 * @return error code
 * @retval 0 no error
 */
int init_gym(int weights[]);

/**
 * @brief [MONITOR METHOD] Hands out weights to the caller.
 *
 * This method will check which weights are available and then try to find a
 * combination that sums up to the required total. If it finds one, the weights
 * will be handed out (signaled by a return value of 0). If not, an error code
 * is returned
 *
 * @param[in] total     the desired total weight
 * @param[out] weights  will contain the weights if method returned 0
 * @return error code
 * @retval 0 no error
 */
int get_weights(int total, int weights[]);

/**
 * @brief [MONITOR METHOD] Returns weights from the caller to the gym.
 *
 * @param[in] weights array with weights to return
 * @return error code
 * @retval 0 no error
 */
int return_weights(int weights[]);

#endif /* GYM_H_ */
