/** ****************************************************************
 * @file    main.c 
 * @author  Moritz Hoewer (Moritz.Hoewer@haw-hamburg.de)
 * @author  Jesko Treffler (Jesko.Treffler@haw-hamburg.de)
 * @version 1.0
 * @date    14.11.2016
 * @brief   Control Module
 ******************************************************************
 */
#include <stdio.h>
#include <stdbool.h>
#include "gym.h"
#include "errors.h"
#include "philosophers.h"

/**
 * @brief Command used to block a philosopher thread
 */
#define BLOCK_CMD 'b'

/**
 * @brief Command used to unblock a philosopher thread
 */
#define UNBLOCK_CMD 'u'

/**
 * @brief Command used to proceed a philosopher thread
 */
#define PROCEED_CMD 'p'

/**
 * @brief Command used to quit all philosopher threads
 */
#define QUIT_CMD 'q'

/**
 * @brief Alternative Command used to quit all philosopher threads
 */
#define ALTERNATIVE_QUIT_CMD 'Q'

/**
 * @brief The size of the read buffer
 */
#define BUFFER_SIZE 10

/**
 * @brief Checks for an invalid ID and prints an error message
 */
#define CHECK_INVALID_ID(res) if((res) == E_NO_SUCH_PHILOSOPHER){ \
    printf("\n!!! "); \
    printf("There is no Philosopher with id %d - valid ids are [0-%d]", \
        philosophers_ID, (PHILOSOPHERS_COUNT - 1)); \
    printf(" !!!\n\n"); \
    }

/**
 * @brief Program entry
 */
int main(void) {
    // set up locla variables for comandline processing
    char input[BUFFER_SIZE] = { 0 };
    bool run = true;
    int philosophers_ID;

    // initialize modules
    gym_init();
    philosophers_init();

    // start reading from commandline
    while (run) {
        if (fgets(input, BUFFER_SIZE, stdin) != NULL) {
            switch (input[0]) {
                case QUIT_CMD:
                case ALTERNATIVE_QUIT_CMD:
                    run = false;
                    break;
                case BLOCK_CMD:
                    if (EOF != sscanf(&input[1], "%d", &philosophers_ID)) {
                        int res = philosophers_block(philosophers_ID);
                        CHECK_INVALID_ID(res)
                    }
                    break;
                case UNBLOCK_CMD:
                    if (EOF != sscanf(&input[1], "%d", &philosophers_ID)) {
                        int res = philosophers_unblock(philosophers_ID);
                        CHECK_INVALID_ID(res)
                    }
                    break;
                case PROCEED_CMD:
                    if (EOF != sscanf(&input[1], "%d", &philosophers_ID)) {
                        int res = philosophers_proceed(philosophers_ID);
                        CHECK_INVALID_ID(res)
                    }
                    break;
                default: /* Unknown command */
                    printf("\n!!! Commands have structure \"CMD [ID]\" !!!\n");
                    printf("Use \"%c[0-%d]\" for blocking \n", BLOCK_CMD,
                            (PHILOSOPHERS_COUNT - 1));
                    printf("Use \"%c[0-%d]\" for unblocking \n", UNBLOCK_CMD,
                            (PHILOSOPHERS_COUNT - 1));
                    printf("Use \"%c[0-%d]\" for proceeding \n", PROCEED_CMD,
                            (PHILOSOPHERS_COUNT - 1));
                    printf("Use \"%c\" or \"%c\" for quitting \n", QUIT_CMD,
                            ALTERNATIVE_QUIT_CMD);
                    printf("\n");
            }
        }
    }

    // quit philosophers
    philosophers_quit();
    return 0;
}

