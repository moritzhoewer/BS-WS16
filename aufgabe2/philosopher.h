/** ****************************************************************
 * @file    aufgabe2/philosopher.h
 * @author  Moritz Hoewer (Moritz.Hoewer@haw-hamburg.de)
 * @author  Jesko Treffler (Jesko.Treffler@haw-hamburg.de)
 * @version 1.0
 * @date    30.10.2016
 * @brief   Header for the Philosopher module
 ******************************************************************
 */

#ifndef PHILOSOPHER_H_
#define PHILOSOPHER_H_

#include <pthread.h>

/**
 * @brief Creates a new philosopher thread and returns its pthread_id
 *
 * @param[in] philo_id   the ID for the new philosopher
 * @param[out] thread_id will contain the thread ID of the philosopher thread
 *                       if method returns 0
 * @return error code
 * @retval 0 no error
 */
int create_philosopher(int philo_id, pthread_t *thread_id);

#endif /* PHILOSOPHER_H_ */
