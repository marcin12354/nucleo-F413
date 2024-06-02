/*
 * bsp.h
 *
 *  Created on: May 30, 2024
 *      Author: marcin
 */

#ifndef INC_BSP_H_
#define INC_BSP_H_

#include "inttypes.h"

#define ENC_CLK_PORT		GPIOF
#define ENC_CLK_PIN			GPIO_PIN_14

#define ENC_DT_PORT			GPIOE
#define ENC_DT_PIN			GPIO_PIN_9

#define ENC_SW_PORT			GPIOE
#define ENC_SW_PIN			GPIO_PIN_11

#define KBD_C1_PORT			GPIOF
#define KBD_C1_PIN			GPIO_PIN_8

#define KBD_C2_PORT			GPIOF
#define KBD_C2_PIN			GPIO_PIN_7

#define KBD_C3_PORT			GPIOF
#define KBD_C3_PIN			GPIO_PIN_9

#define KBD_C4_PORT			GPIOG
#define KBD_C4_PIN			GPIO_PIN_1

#define KBD_R1_PORT			GPIOE
#define KBD_R1_PIN			GPIO_PIN_3

#define KBD_R2_PORT			GPIOE
#define KBD_R2_PIN			GPIO_PIN_6

#define KBD_R3_PORT			GPIOE
#define KBD_R3_PIN			GPIO_PIN_5

#define KBD_R4_PORT			GPIOE
#define KBD_R4_PIN			GPIO_PIN_4

void bsp_kbd_set_row (uint8_t data);

uint8_t bsp_kbd_get_col(void);


#endif /* INC_BSP_H_ */
