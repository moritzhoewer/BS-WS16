/** ****************************************************************
 * @file    main.c 
 * @author  Moritz Hoewer (Moritz.Hoewer@haw-hamburg.de)
 * @author  Jesko Treffler (Jesko.Treffler@haw-hamburg.de)
 * @version 1.0
 * @date    07.11.2016
 * @brief   Control Module
 ******************************************************************
 */
#include <stdio.h>
#include <stdbool.h>
#include "gym.h"
#include "errors.h"
#include "philosophers.h"

/**
 * @brief The size of the read buffer
 */
#define BUFFER_SIZE 10

/*
 * No longer works, because of condition variable
 */
/*void test_is_combination_possible() {
    // take out some weights
    int my_weights[GYM_WEIGHTS_AVAILIABLE_SIZE] = { 0 };
    //gym_get_weights(25, my_weights);

    // check which combinations are possible
    for (int total = 2; total <= 45; total++) {
        int my_weights[GYM_WEIGHTS_AVAILIABLE_SIZE] = { 0 };
        if (gym_get_weights(total, my_weights) == 0) {
            // nice, it's possible
            printf("[%2d] - Got them! ==> ", total);
            printf("%dx 2kg + %dx 3kg + %dx 5kg\n", my_weights[0], my_weights[1], my_weights[2]);

            // verifiy that we didn't get bullshitted
            if(my_weights[0] * 2 + my_weights[1] * 3  + my_weights[2] * 5 != total){
                printf("Ohhh ha, das sollte aber nicht sein...\n");
                return;
            }
        } else {
            // not possible? Oh well...
            printf("[%2d] - Darn...\n", total);
        }
        gym_return_weights(my_weights);
    }

    gym_return_weights(my_weights);
}*/

/**
 * @brief Program entry
 */
int main(void) {
	char input[BUFFER_SIZE] = { 0 };
	bool run=true;
	int philosophers_ID;
    gym_init();
    philosophers_init();
    while(run){
		if(fgets(input, BUFFER_SIZE, stdin) != NULL){
			switch(input[0]){
				case QUIT_PHILOSOPHER:
				case UPPER_QUIT_PHILOSOPHER:
					run=false;	
					break;
				case BLOCK_PHILOSOPHER:
			        if(EOF != sscanf(&input[1],"%d",&philosophers_ID)){
						philosophers_block(philosophers_ID);
				    }
					break;
				case UNBLOCK_PHILOSOPHER:
					if(EOF != sscanf(&input[1],"%d",&philosophers_ID)){
						philosophers_unblock(philosophers_ID);
				    }
					break;
				case PROCEED_PHILOSOPHER:
					if(EOF != sscanf(&input[1],"%d",&philosophers_ID)){
						philosophers_proceed(philosophers_ID);
				    }
					break;
				default: 
				    printf("\n!!!\nEs sind nur folgende Kombinationen erlaubt");
					printf(" \"%c\"[0-%d],", BLOCK_PHILOSOPHER, PHILOSOPHERS_COUNT);
					printf(" \"%c\"[0-%d],", UNBLOCK_PHILOSOPHER, PHILOSOPHERS_COUNT);
					printf(" \"%c\"[0-%d] ",PROCEED_PHILOSOPHER, PHILOSOPHERS_COUNT);
					printf(" und \"%c\" bzw. \"%c\" ",QUIT_PHILOSOPHER,UPPER_QUIT_PHILOSOPHER);
					printf("\n!!!\n");
			}
		}
	}
    philosophers_quit();
    return 0;
}

