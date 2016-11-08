/** ****************************************************************
 * @file    aufgabe2/philosophers.h
 * @author  Moritz Hoewer (Moritz.Hoewer@haw-hamburg.de)
 * @author  Jesko Treffler (Jesko.Treffler@haw-hamburg.de)
 * @version 1.0
 * @date    07.11.2016
 * @brief   Header for the Philosophers module
 ******************************************************************
 */

#ifndef PHILOSOPHERS_H_
#define PHILOSOPHERS_H_
#define BLOCK_PHILOSOPHER 'b'
#define UNBLOCK_PHILOSOPHER 'u'
#define PROCEED_PHILOSOPHER 'p'
#define QUIT_PHILOSOPHER 'q'
#define UPPER_QUIT_PHILOSOPHER 'Q'

/**
 * @brief How many Philosophers will be created
 */
#define PHILOSOPHERS_COUNT 5

/**
 * @brief The weights for the Philosophers
 */
#define PHILOSOPHERS_WEIGHTS 6, 8, 12, 12, 14

/**
 * @brief Threshold for the arg_barrier
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
