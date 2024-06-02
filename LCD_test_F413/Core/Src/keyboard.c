/*
 * keyboard.c
 *
 *  Created on: May 30, 2024
 *      Author: marcin
 */

#include "keyboard.h"
#include "circular_buffer.h"
#include "bsp.h"


const uint8_t key_table[KBD_COLS][KBD_ROWS]={
		{KBD_MSG_SW_1,KBD_MSG_SW_2,KBD_MSG_SW_3,KBD_MSG_SW_ESC},
		{KBD_MSG_SW_4,KBD_MSG_SW_5,KBD_MSG_SW_6,KBD_MSG_SW_ENTER},
		{KBD_MSG_SW_7,KBD_MSG_SW_8,KBD_MSG_SW_9,KBD_MSG_SW_SET_VOLTAGE},
		{KBD_MSG_SW_0,KBD_MSG_DOT,KBD_MSG_SW_MENU,KBD_MSG_SW_SET_CURRENT_LIMIT}
};

void kbd_init(matrix_keyboard_t *self,cbuf_handle_t buffor){
	for (uint8_t a = 0; a < KBD_ROWS; a++){
		self->previous[a]=0;
	}
	self->buffor = buffor;
}

void kbd_io_set_row(matrix_keyboard_t *self, uint8_t data){
	bsp_kbd_set_row(data);
}

uint8_t kbd_io_get_col(matrix_keyboard_t *self){
return bsp_kbd_get_col();
}

void kbd_proc (matrix_keyboard_t *self){
	uint8_t col_data;
	uint8_t row_data;
	kbd_msg_t pressed_key;
	col_data = 0;

	//this->keyBuffer.clear();

	for (uint8_t a = 0; a < KBD_ROWS; a++) {
		row_data = ~(1 << a);
		kbd_io_set_row(self,row_data);
		//delay(2);
		col_data = kbd_io_get_col(self);

		col_data = ~col_data;
		for (uint8_t b = 0; b < KBD_COLS; b++) {
			//if (bitRead(col_data, b) && bitRead(this->previous[a], b)) {
			if( ( col_data & 1<<b ) && (self->previous[a] & 1 << b ) ){

				//bitClear(kbd::active[a],b);
				if (!circular_buf_full(self->buffor)) {
					pressed_key = key_table[a][b];
					circular_buf_put(self->buffor,pressed_key);
				}
			}

			//if (bitRead(col_data, b) && bitRead(this->previous[a], b)) {
				//bitClear(kbd::active[a],b);
			//}
		}
		self->previous[a] = col_data;
	}
}

