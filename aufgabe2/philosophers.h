/** ****************************************************************
 * @file    aufgabe2/philosophers.h
 * @author  Moritz Hoewer (Moritz.Hoewer@haw-hamburg.de)
 * @author  Jesko Treffler (Jesko.Treffler@haw-hamburg.de)
 * @version 1.0
 * @date    14.11.2016
 * @brief   Header for the Philosophers module
 ******************************************************************
 */

#ifndef PHILOSOPHERS_H_
#define PHILOSOPHERS_H_

/**
 * @brief How many Philosophers will be created
 *
 * If this is changed, PHILOSOPHERS_WEIGHTS must also be changed to match
 */
#define PHILOSOPHERS_COUNT 5

/**
 * @brief The weights for the Philosophers
 *
 * If the number of weights is changed, PHILOSOPHERS_COUNT must also be changed
 * to match.
 * This does not apply for changes in amount.
 */
#define PHILOSOPHERS_WEIGHTS 6, 8, 12, 12, 14

/**
 * @brief Displays the current status on the screen
 *
 * Will print state and held weights information for all philosophers and the
 * gym.
 *
 * @param gym_weights the weights currently present in the gym.
 *                    used for displaying and sync error checking
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
int philosophers_unblock(int philo_id);

/**
 * @brief Proceeds the specified philosopher behind the current loop
 *
 * @param philo_id the ID of the philosopher to proceed
 * @return error code
 * @retval 0                     no error
 * @retval E_NO_SUCH_PHILOSOPHER the given ID does not exist
 */
int philosophers_proceed(int philo_id);

#endif /* PHILOSOPHERS_H_ */
