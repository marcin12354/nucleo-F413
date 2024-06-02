/*
 * gui.h
 *
 *  Created on: May 15, 2024
 *      Author: marcin
 */

#ifndef INC_GUI_H_
#define INC_GUI_H_

typedef enum{
	NO_FRAME = 0,
	RECT,
	FILLED_RECT,
	ROUNDED_RECT,
	ROUNDED_FILLED_RECT,
}gui_textbox_frame_type_t;



typedef enum{
	H_ALIGN_LEFT = 0,
	H_ALIGN_CENTER,
	H_ALIGN_RIGHT
}gui_textbox_Halign_t;

typedef enum{
	V_ALIGN_UP = 0x10,
	V_ALIGN_CENTER = 0x20,
	V_ALIGN_DOWN = 0x30,
	V_ALIGN_CENTER_BASELINE = 0x40,
	V_ALIGN_DOWN_BASELINE = 0x50,
}gui_textbox_Valign_t;

typedef enum{
	CURSOR_BEGIN = 0,
	CURSOR_END,
	CURSOR_POSSITION
}gui_textbox_cursor_pos_t;

typedef enum{
	BLINK_NO = 0x10,
	BLINK_BAR = 0x20,
	BLINK_RECTANGLE = 0x30,
}gui_textbox_cursor_blink_t;

typedef struct gui_textbox_t gui_textbox_t;

struct gui_textbox_t{
	int8_t pos_x;
	int8_t pos_y;
	uint8_t size_x;
	uint8_t size_y;
	const char *str;
	char *cursor;
	gui_textbox_cursor_blink_t cursor_blink_mode;
	uint8_t str_size;
	uint8_t radius;
	uint8_t align_mode;
	gui_textbox_frame_type_t frame_type;
	uint8_t *font;
	void (*onKey)(gui_textbox_t *self, uint8_t pressed_key);
	void (*onChangeCallback)(gui_textbox_t *self, uint8_t value);

};

// Initialization of the GUI Text Box
// Have to be called before any other functions related to the Text Box

void gui_textbox_init(gui_textbox_t *self,char *str, uint8_t n,
						int8_t pos_x, int8_t pos_y,uint8_t size_x,uint8_t size_y );

void gui_textbox_setFont(gui_textbox_t *self, uint8_t *font);

void gui_textbox_deleteChar(gui_textbox_t *self);

void gui_textbox_addChar(gui_textbox_t *self,char c);

void gui_textbox_swapChar(gui_textbox_t *self,char c);

void gui_textbox_deleteText(gui_textbox_t *self);

void gui_textbox_addText(gui_textbox_t *self,const char * str);

void gui_textbox_setCursorPos(gui_textbox_t *self,gui_textbox_cursor_pos_t mode ,uint8_t pos);

void gui_textbox_setFrameType(gui_textbox_t *self,gui_textbox_frame_type_t ftype,uint8_t r);

void gui_textbox_setAllignHV(gui_textbox_t *self, gui_textbox_Halign_t H, gui_textbox_Valign_t V);

void gui_textbox_display(gui_textbox_t *self);



#endif /* INC_GUI_H_ */
