/*
 * udm_utils.h
 *
 *  Created on: Dec 1, 2016
 *      Author: lcastrol
 */

#ifndef UDM_UTILS_H_
#define UDM_UTILS_H_




#if defined(__AVR_ATtiny24__)

typedef enum
{
    timer_clock_div_reset = 0,
    timer_clock_div_1 = 1,
    timer_clock_div_2 = 2,
    timer_clock_div_4 = 3,
    timer_clock_div_8 = 4,
    timer_clock_div_16 = 5,
    timer_clock_div_32 = 6,
    timer_clock_div_64 = 7
} timer_clock_div_t;



#endif


#endif /* UDM_UTILS_H_ */
