/*
 * bsp.c
 *
 *  Created on: May 31, 2024
 *      Author: marci
 */

#include "bsp.h"
#include "stm32f4xx_hal.h"

uint8_t get_bit_value(uint8_t data,uint8_t bit){
	uint8_t bit_state;

	if (data & (1 << bit) ) {
		bit_state = 1;
	} else {
		bit_state = 0;
	}
	return bit_state;
}

void bsp_kbd_set_row (uint8_t data){

	HAL_GPIO_WritePin(KBD_R1_PORT, KBD_R1_PIN, get_bit_value(data,0));
	HAL_GPIO_WritePin(KBD_R2_PORT, KBD_R2_PIN, get_bit_value(data,1));
	HAL_GPIO_WritePin(KBD_R3_PORT, KBD_R3_PIN, get_bit_value(data,2));
	HAL_GPIO_WritePin(KBD_R4_PORT, KBD_R4_PIN, get_bit_value(data,3));
}

uint8_t bsp_kbd_get_col(void){
	uint8_t value;

	value = 0;
	value |= HAL_GPIO_ReadPin(KBD_C1_PORT, KBD_C1_PIN) << 0;
	value |= HAL_GPIO_ReadPin(KBD_C2_PORT, KBD_C2_PIN) << 1;
	value |= HAL_GPIO_ReadPin(KBD_C3_PORT, KBD_C3_PIN) << 2;
	value |= HAL_GPIO_ReadPin(KBD_C4_PORT, KBD_C4_PIN) << 3;

	return value;
}
