/*
 * gui_text_box.c
 *
 *  Created on: May 15, 2024
 *      Author: marcin
 */

#include "inttypes.h"
#include "circular_buffer.h"
#include "u8g2.h"
#include "stm32f4xx_hal.h"
#include "stdlib.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "gui.h"



extern u8g2_t u8g2;

void gui_textbox_init(	gui_textbox_t *self,char *str, uint8_t n,
						int8_t pos_x, int8_t pos_y,uint8_t size_x,uint8_t size_y ){
	assert(self);
	assert(n);
	assert(size_x);
	assert(size_y);

	self->str = str;
	self->cursor = str;
	self->str_size = n;
	self->pos_x = pos_x;
	self->pos_y = pos_y;
	self->size_x = size_x;
	self->size_y = size_y;
	self->radius = 0;
	self->align_mode = 0;
	self->frame_type = 0;

}

void inline gui_textbox_setFont(gui_textbox_t *self, uint8_t *font){
	self->font = font;
}

void gui_textbox_deleteChar(gui_textbox_t *self){

}

void gui_textbox_addChar(gui_textbox_t *self,char c){
	assert(c);
	if(self->cursor > self->str - 1){
		*self->cursor = c;
		self->cursor++;
	}
}

void gui_textbox_deleteText(gui_textbox_t *self){
	memset(self->str,0,self->str_size);
	self->cursor = self->str;
}

void gui_textbox_addText(gui_textbox_t *self,const char * str){
	uint32_t size = self->str + self->str_size - self->cursor;
	memcpy(self->cursor,str,size);
	self->cursor += size;
}

void gui_textbox_setCursorPos(gui_textbox_t *self,gui_textbox_cursor_pos_t mode ,uint8_t pos){
	if ((mode & 0x0F) == CURSOR_BEGIN){
		self->cursor = self->str;
	}
	if ((mode & 0x0F) == CURSOR_END){
			self->cursor = self->str + self->str_size - 1;
			for(uint8_t a = 0; a < self->str_size; a++){
				if (self->cursor != 0){
					break;
				}else{
					self->cursor --;
				}
			}

		}
	if ((mode & 0x0F) == CURSOR_POSSITION){
		if (pos < self->str_size - 1){
			self->cursor = self->str + pos;
		}
	}
}

void gui_textbox_setFrameType(gui_textbox_t *self,gui_textbox_frame_type_t ftype,uint8_t r)
{
	assert(self);
	self->frame_type = ftype;
	self->radius = r;
}

void inline gui_textbox_setAllignHV(gui_textbox_t *self, gui_textbox_Halign_t H, gui_textbox_Valign_t V){
	self->align_mode = (V & 0xF0) | (H & 0x0F);
}

void gui_textbox_display(gui_textbox_t *self){
	assert(self);

	uint8_t x,y;
	u8g2_SetFont(&u8g2, self->font);
	uint8_t str_lenght = u8g2_GetStrWidth(&u8g2,self->str);

	u8g2_SetFontPosBaseline(&u8g2);
	u8g2_SetDrawColor(&u8g2, 1);
	//u8g2_DrawUTF8(&u8g2, type.x + (type.box_x - width) / 2 , type.y + type.box_y - (type.box_y - height)/2 - 1, buffer);

	u8g2_SetClipWindow(&u8g2,self->pos_x,self->pos_y,self->pos_x + self->size_x,self->pos_y + self->size_y);

	switch (self->frame_type) {
	case NO_FRAME:

		break;
	case RECT:
		u8g2_DrawFrame(&u8g2, self->pos_x, self->pos_y, self->size_x,
				self->size_y);
		u8g2_SetDrawColor(&u8g2,1);
		break;
	case FILLED_RECT:
		u8g2_DrawBox(&u8g2, self->pos_x, self->pos_y, self->size_x,
				self->size_y);
		u8g2_SetDrawColor(&u8g2,0);
		break;
	case ROUNDED_RECT:
		u8g2_DrawRFrame(&u8g2, self->pos_x, self->pos_y, self->size_x,
				self->size_y, self->radius);
		u8g2_SetDrawColor(&u8g2,1);
		break;
	case ROUNDED_FILLED_RECT:
		u8g2_DrawRBox(&u8g2, self->pos_x, self->pos_y, self->size_x,
				self->size_y, self->radius);
		u8g2_SetDrawColor(&u8g2,0);
		break;
	default:
		break;
	}
	// Consider if all calculation should be done on uin16_t to prevent over
	switch (self->align_mode & 0x0F) {
	case H_ALIGN_LEFT:
		x = self->pos_x + 1;
		break;
	case H_ALIGN_CENTER:
		x = self->pos_x + (self->size_x - str_lenght) / 2;
		break;
	case H_ALIGN_RIGHT:
		x = self->pos_x + self->size_x - str_lenght - 1;
		break;
	default:
		break;
	}

	y = self->pos_y + self->size_y;

	switch (self->align_mode & 0xF0) {
	case V_ALIGN_UP:
		u8g2_SetFontPosBaseline(&u8g2);
		y = self->pos_y + u8g2_GetAscent(&u8g2) + 1;
		break;
	case V_ALIGN_CENTER:
		u8g2_SetFontPosBaseline(&u8g2);
		y = self->pos_y + (self->size_y / 2) + u8g2_GetAscent(&u8g2) / 2;
		break;
	case V_ALIGN_DOWN:
		u8g2_SetFontPosBaseline(&u8g2);
		y = self->pos_y + self->size_y - 1;
		break;
	case V_ALIGN_CENTER_BASELINE:
		u8g2_SetFontPosBottom(&u8g2);
		y = self->pos_y + (self->size_y / 2) + u8g2_GetAscent(&u8g2) - u8g2_GetDescent(&u8g2) / 2;
		break;
	case V_ALIGN_DOWN_BASELINE:
		u8g2_SetFontPosBottom(&u8g2);
		y = self->pos_y + self->size_y - 1;
		break;
	default:
		break;
	}

	u8g2_SetClipWindow(&u8g2,self->pos_x,self->pos_y,self->pos_x + self->size_x,self->pos_y + self->size_y - 1);

	//static char cursor_temp_char;
	if (self->cursor_blink_mode == BLINK_BAR){
		//cursor_temp_char =


		switch (*self->cursor) {
		case 0:
			*self->cursor = ' ';
			break;
		case ' ':
			*self->cursor = '_';
			break;

		case '_':
			*self->cursor = ' ';
			break;
		default:
			*self->cursor = '#';
						break;
		}
	}

	u8g2_DrawUTF8(&u8g2,x,y,self->str);

}


