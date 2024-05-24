/*
 * application.c
 *
 *  Created on: May 5, 2024
 *      Author: marcin
 */

#include "inttypes.h"
#include "circular_buffer.h"
#include "u8g2.h"
#include "stm32f4xx_hal.h"
#include "stdlib.h"
#include <string.h>
#include <stdio.h>
#include "gui.h"

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

typedef enum{
	DISPLAY_WINDOW_WELCOME = 0,
	DISPLAY_WINDOW_MENU,
	DISPLAY_WINDOW_PSU,
	DISPLAY_WINDOW_CHARGER
}display_window_t;

typedef enum{
	DISPLAY_CURENT_LIMIT_NORMAL_MODE = 0,
	DISPLAY_CURENT_LIMIT_INSERTION_MODE,
	DISPLAY_CURENT_LIMIT_INSERTION_IN_PROGRESS,	// find better name 1.2_
}display_current_limit_mode_t;



extern u8g2_t u8g2;

//void gui_numberBoxDisplay(gui_number_box_t type) {
//	uint8_t width;
//	uint8_t height = u8g2_GetAscent(&u8g2) - u8g2_GetDescent(&u8g2);
//	char buffer[10];
//	display_current_limit_mode_t mode = DISPLAY_CURENT_LIMIT_NORMAL_MODE;
//
//	uint16_dec_to_string(type.value, buffer, sizeof(buffer),
//			type.decimal_places);
//	strncpy(&buffer[strlen(buffer)], type.sufix, 2);
//
//	width = u8g2_GetStrWidth(&u8g2, buffer);
//
//	switch (mode) {
//
//	case DISPLAY_CURENT_LIMIT_NORMAL_MODE:
//
//		u8g2_SetFont(&u8g2, u8g2_font_inr16_mr);
//		u8g2_SetFontPosBaseline(&u8g2);
//		u8g2_DrawUTF8(&u8g2,
//				type.x + ((int8_t) type.box_x - (int8_t) width) / 2,
//				type.y + type.box_y - (type.box_y - height) / 2 - 1, buffer);
//		//u8g2_DrawRFrame(&u8g2, type.x, type.y, type.box_x, type.box_y,5);
//
//		break;
//	case DISPLAY_CURENT_LIMIT_INSERTION_MODE:
//
//		u8g2_SetFont(&u8g2, u8g2_font_inr16_mr);
//		u8g2_SetFontPosBaseline(&u8g2);
//		u8g2_SetDrawColor(&u8g2, 2);
//		u8g2_DrawUTF8(&u8g2, type.x + (type.box_x - width) / 2,
//				type.y + type.box_y - (type.box_y - height) / 2 - 1, buffer);
//		u8g2_DrawRBox(&u8g2, type.x, type.y, type.box_x, type.box_y, 5);
//
//		break;
//	case DISPLAY_CURENT_LIMIT_INSERTION_IN_PROGRESS:
//
//		u8g2_SetFont(&u8g2, u8g2_font_inr16_mr);
//		u8g2_SetFontPosBaseline(&u8g2);
//		u8g2_SetDrawColor(&u8g2, 2);
//		u8g2_DrawUTF8(&u8g2, type.x + (type.box_x - width) / 2,
//				type.y + type.box_y - (type.box_y - height) / 2 - 1, buffer);
//		u8g2_DrawRBox(&u8g2, type.x, type.y, type.box_x, type.box_y, 5);
//
//		break;
//	default:
//		break;
//	}
//
//}



gui_textbox_t gui_TB_voltage;
gui_textbox_t gui_TB_current_limit;
gui_textbox_t gui_TB_current_measured;
gui_textbox_t gui_TB_voltage_measured;
gui_textbox_t gui_TB_CC;

char gui_TB_voltage_array[10];
char gui_TB_current_limit_array[15];
char gui_TB_current_measured_array[10];
char gui_TB_voltage_measured_array[10];
char gui_TB_CC_array[3];

void display_proc(cbuf_handle_t q) {
	static display_window_t window = DISPLAY_WINDOW_WELCOME;

	static display_current_limit_mode_t mode = DISPLAY_CURENT_LIMIT_NORMAL_MODE;


	switch (window) {
	case DISPLAY_WINDOW_WELCOME:
		//display_current_limit(mode,1000);

		u8g2_ClearBuffer(&u8g2);
		u8g2_DrawFrame(&u8g2, 0, 0, 128, 64);
		u8g2_SetFont(&u8g2, u8g2_font_inr16_mr);
		u8g2_DrawUTF8(&u8g2, 15, 40, "WELCOME");
		u8g2_SendBuffer(&u8g2);

		break;
	case DISPLAY_WINDOW_MENU:

		u8g2_ClearBuffer(&u8g2);
		gui_textbox_display(&gui_TB_voltage);
		gui_textbox_display(&gui_TB_current_limit);
		gui_textbox_display(&gui_TB_current_measured);
		gui_textbox_display(&gui_TB_voltage_measured);
		gui_textbox_display(&gui_TB_CC);
		u8g2_SendBuffer(&u8g2);

		break;
	case DISPLAY_WINDOW_PSU:

		break;
	case DISPLAY_WINDOW_CHARGER:

		break;
	default:

		break;
	}


	if(mode == DISPLAY_CURENT_LIMIT_NORMAL_MODE && window == DISPLAY_WINDOW_MENU){
		mode = DISPLAY_CURENT_LIMIT_INSERTION_MODE;
	}
	else{
		mode = DISPLAY_CURENT_LIMIT_NORMAL_MODE;
	}

	if (window == DISPLAY_WINDOW_WELCOME){

			window = DISPLAY_WINDOW_MENU;
	}
}


cbuf_handle_t queue;
#define APP_DISPLAY_TIMER_TIME 20

uint16_t volatile app_display_timer = 0;
uint16_t volatile app_display_timer_flag = 0;

uint8_t app_is_display_timeout(void) {
	if (app_display_timer_flag == 1) {
		app_display_timer_flag = 0;
		return 1;
	}
	return 0;
}

void app_display_timer_proc(void){
	app_display_timer++;
	if (app_display_timer == APP_DISPLAY_TIMER_TIME){
		app_display_timer = 0;
		app_display_timer_flag = 1;
	}


}


typedef void (*app_tick_callback)(void);

#define APP_TICK_CALLBACK_TABLE_SIZE 10

typedef struct{
	app_tick_callback table[APP_TICK_CALLBACK_TABLE_SIZE];
	uint8_t index;
}app_tick_calback_s;

app_tick_calback_s publisher;

void app_tick_init(void){	//find better name for this
	publisher.index = 0;
}

uint8_t app_register_tick_callback(app_tick_callback callback) {

	if (publisher.index < APP_TICK_CALLBACK_TABLE_SIZE) {
		publisher.table[publisher.index] = callback;
		publisher.index++;
		return 1;
	} else {
		return 0; //ERROR
	}
}

void app_ticks (void){
	if (publisher.index != 0) {
		for (uint8_t i = 0; i <= publisher.index; i++) {
			if (publisher.table[i] != 0) {
				publisher.table[i]();
			}
		}
	}
}

void app_run(void) {

	gui_textbox_init(&gui_TB_voltage, gui_TB_voltage_array,
			sizeof(gui_TB_voltage_array), 0, 0, 30, 16);
	gui_textbox_setFrameType(&gui_TB_voltage, ROUNDED_RECT, 2);
	gui_textbox_addText(&gui_TB_voltage, "SET\0");
	gui_textbox_setAllignHV(&gui_TB_voltage, H_ALIGN_CENTER, V_ALIGN_CENTER);
	gui_textbox_setFont(&gui_TB_voltage, u8g2_font_6x12_mf);
	gui_textbox_setCursorPos(&gui_TB_voltage, CURSOR_POSSITION, 4);
	gui_TB_voltage.cursor_blink_mode = BLINK_NO;

	gui_textbox_init(&gui_TB_current_limit, gui_TB_current_limit_array,
			sizeof(gui_TB_current_limit_array), 31, 0, 97, 16);
	gui_textbox_setFrameType(&gui_TB_current_limit, ROUNDED_RECT, 2);
	gui_textbox_addText(&gui_TB_current_limit, "12.4V 1.00A\0");
	gui_textbox_setAllignHV(&gui_TB_current_limit, H_ALIGN_CENTER,
			V_ALIGN_CENTER);
	gui_textbox_setFont(&gui_TB_current_limit, u8g2_font_6x12_mf);
	gui_TB_current_limit.cursor_blink_mode = BLINK_NO;

	gui_textbox_init(&gui_TB_current_measured, gui_TB_current_measured_array,
			sizeof(gui_TB_current_measured_array), 48, 17, 80, 22);
	gui_textbox_setFrameType(&gui_TB_current_measured, ROUNDED_RECT, 2);
	gui_textbox_addText(&gui_TB_current_measured, "1.001A\0");
	gui_textbox_setAllignHV(&gui_TB_current_measured, H_ALIGN_CENTER,
			V_ALIGN_CENTER);
	gui_textbox_setFont(&gui_TB_current_measured, u8g2_font_10x20_mf);

	gui_textbox_init(&gui_TB_voltage_measured, gui_TB_voltage_measured_array,
			sizeof(gui_TB_voltage_measured_array), 48, 41, 80, 22);
	gui_textbox_setFrameType(&gui_TB_voltage_measured, ROUNDED_RECT, 2);
	gui_textbox_addText(&gui_TB_voltage_measured, "12.6V\0");
	gui_textbox_setAllignHV(&gui_TB_voltage_measured, H_ALIGN_CENTER,
			V_ALIGN_CENTER);
	gui_textbox_setFont(&gui_TB_voltage_measured, u8g2_font_10x20_mf);

	gui_textbox_init(&gui_TB_CC, gui_TB_CC_array, sizeof(gui_TB_CC_array), 0,
			20, 46, 40);
	gui_textbox_setFrameType(&gui_TB_CC, ROUNDED_FILLED_RECT, 5);
	gui_textbox_addText(&gui_TB_CC, "CC\0");
	gui_textbox_setAllignHV(&gui_TB_CC, H_ALIGN_CENTER, V_ALIGN_CENTER);
	gui_textbox_setFont(&gui_TB_CC, u8g2_font_inb24_mf);


	app_tick_init();
	app_register_tick_callback(app_display_timer_proc);


	while (1) {
		//circular_buf_put(queue,current_limit.x);
		if (app_is_display_timeout()) {
			display_proc(queue);
		}

	}

}


