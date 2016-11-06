/** ****************************************************************
 * @file    aufgabe2/philosophers.h
 * @author  Moritz Hoewer (Moritz.Hoewer@haw-hamburg.de)
 * @author  Jesko Treffler (Jesko.Treffler@haw-hamburg.de)
 * @version 1.0
 * @date    30.10.2016
 * @brief   Header for the Philosopher module
 ******************************************************************
 */

#ifndef PHILOSOPHERS_H_
#define PHILOSOPHERS_H_

/**
 * @brief Displays the current status on the screen
 *
 * Will print state and held weights information for all philosophers.
 */
void philosophers_display_status(const int gym_weights[]);

/**
 * @brief Initializes the philosophers
 */
void philosophers_init();

/**
 * @brief Stops all the philosopher threads
 */
void philosophers_quit();

/**
 * @brief Blocks the specified philosopher
 *
 * @param philo_id the ID of the philosopher to block
 * @return error code
 * @retval 0                     no error
 * @retval E_NO_SUCH_PHILOSOPHER the given ID does not exist
 */
int philosophers_block(int philo_id);

/**
 * @brief Unblocks the specified philosopher
 *
 * @param philo_id the ID of the philosopher to unblock
 * @return error code
 * @retval 0                     no error
 * @retval E_NO_SUCH_PHILOSOPHER the given ID does not exist
 */
int philosopher_unblock(int philo_id);

/**
 * @brief Proceeds the specified philosopher behind the current loop
 *
 * @param philo_id the ID of the philosopher to proceed
 * @return error code
 * @retval 0                     no error
 * @retval E_NO_SUCH_PHILOSOPHER the given ID does not exist
 */
int philosopher_proceed(int philo_id);

#endif /* PHILOSOPHERS_H_ */
