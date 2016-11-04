/** ****************************************************************
 * @file    main.c 
 * @author  Moritz Hoewer (Moritz.Hoewer@haw-hamburg.de)
 * @author  Jesko Treffler (Jesko.Treffler@haw-hamburg.de)
 * @version 1.0
 * @date    30.10.2016
 * @brief   Control Module
 ******************************************************************
 */
#include <stdio.h>
#include <stdbool.h>
#include "philosopher.h"
#include "output.h"
#include "gym.h"
#include "errors.h"

void test_is_combination_possible() {
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
}

int main(void) {
    if (gym_init()) {
        printf("Error with Gym!\n");
        return -1;
    }

    test_is_combination_possible();

    return 0;
}

