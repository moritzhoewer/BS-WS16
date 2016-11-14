/** ****************************************************************
 * @file    errors.h 
 * @author  Moritz Hoewer (Moritz.Hoewer@haw-hamburg.de)
 * @author  Jesko Treffler (Jesko.Treffler@haw-hamburg.de)
 * @version 2.0
 * @date    14.11.2016
 * @brief   Collection of all the Error handling for the Program
 ******************************************************************
 */


#ifndef ERRORS_H_
#define ERRORS_H_

#include <stdlib.h>
#include <stdio.h>

/**
 * @brief Checks for and handles a fatal error
 *
 * Checks if result is not 0 (which indicates an error), in which case it
 * prints message and quits with exit(EXIT_FAILURE)
 */
#define FATAL_ERROR_HANDLING(result, message) if( (result) ) { \
    perror( (message) ); \
    exit(EXIT_FAILURE); \
    }


/**
 * @brief Error code returned by command methods if they get an invalid ID
 */
#define E_NO_SUCH_PHILOSOPHER - 1

#endif /* ERRORS_H_ */
