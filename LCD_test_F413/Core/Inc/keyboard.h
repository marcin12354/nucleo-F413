/*
 * keyboard.h
 *
 *  Created on: May 30, 2024
 *      Author: marcin
 */

#ifndef INC_KEYBOARD_H_
#define INC_KEYBOARD_H_

#include "inttypes.h"
#include "circular_buffer.h"

#define KBD_ROWS 4
#define KBD_COLS 4

typedef enum{
	KBD_MSG_SW_0 = 0,
	KBD_MSG_SW_1,
	KBD_MSG_SW_2,
	KBD_MSG_SW_3,
	KBD_MSG_SW_4,
	KBD_MSG_SW_5,
	KBD_MSG_SW_6,
	KBD_MSG_SW_7,
	KBD_MSG_SW_8,
	KBD_MSG_SW_9,
	KBD_MSG_DOT,
	KBD_MSG_SW_ENTER,
	KBD_MSG_SW_ESC,
	KBD_MSG_SW_SET_VOLTAGE,
	KBD_MSG_SW_SET_CURRENT_LIMIT,
	KBD_MSG_SW_ENABLE,
	KBD_MSG_SW_MENU,
	ENCODER_RIGHT,
	ENCODER_LEFT,
	ENCODER_SW
}kbd_msg_t;

typedef struct{
	uint8_t previous[KBD_ROWS];
	cbuf_handle_t buffor;
}matrix_keyboard_t;

void kbd_init(matrix_keyboard_t *self,cbuf_handle_t buffor);

void kbd_proc (matrix_keyboard_t *self);


void kbd_io_set_row(matrix_keyboard_t *self, uint8_t data);

uint8_t kbd_io_get_col(matrix_keyboard_t *self);

#endif /* INC_KEYBOARD_H_ */
