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
#include "debug.h"
#include "keyboard.h"
#include "psu.h"



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

matrix_keyboard_t kbd;

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


//******************************************
// Timer for keyboard function
//******************************************
#define KBD_TIMER_TIME 50

void kbd_timer (void){
	uint8_t static timer = 0;
	 timer++;
	 if (timer == KBD_TIMER_TIME){
		 timer = 0;
		 kbd_proc(&kbd);
	 }
}

//******************************************
// Timer for display function
//******************************************

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

//******************************************
// publisher for time dependent event
//******************************************
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


//******************************************
// Main application functions
//******************************************
typedef enum{
	DISP_PSU_NORMAL = 0,
	DISP_PSU_SET_V,
	DISP_PSU_SET_V_KBD,
	DISP_PSU_SET_I,
	DISP_PSU_SET_I_KBD,
}disp_psu_mode_t;

typedef struct {
	display_window_t window;
	disp_psu_mode_t		psu_mode;
	char backup_buffor[10];
	uint16_t backup_value;
}state_t;

state_t state;
PSU_control_t psu;

#define MESAGE_ENC_BUFFOR_SIZE 5

// Returns ASCII code for pressed key

char kbd_return_key_char(kbd_msg_t key){
	char out;
	switch (key) {
	case KBD_MSG_SW_0:	out = '0'; break;
	case KBD_MSG_SW_1:	out = '1'; break;
	case KBD_MSG_SW_2:	out = '2'; break;
	case KBD_MSG_SW_3:	out = '3'; break;
	case KBD_MSG_SW_4:	out = '4'; break;
	case KBD_MSG_SW_5:	out = '5'; break;
	case KBD_MSG_SW_6:	out = '6'; break;
	case KBD_MSG_SW_7:	out = '7'; break;
	case KBD_MSG_SW_8:	out = '8'; break;
	case KBD_MSG_SW_9:	out = '9'; break;
	case KBD_MSG_DOT:	out = '.'; break;
	default:   			out = 0; break;
	}
	return out;
}

cbuf_handle_t kbd_enc_queue;
uint8_t kbd_enc_queue_buffor[MESAGE_ENC_BUFFOR_SIZE];

void state_machine_init(){
	state.psu_mode = DISP_PSU_NORMAL;
}

void state_machine(void){

	kbd_msg_t msg = 0;
	if (circular_buf_empty(kbd_enc_queue)) {
		return;
	}
	circular_buf_get(kbd_enc_queue,(uint8_t*)&msg);

	if (msg == KBD_MSG_SW_MENU){
		// changing window
	}
	if (msg == KBD_MSG_SW_ENABLE) {
		// changing window enable disable output

	}


	switch (state.psu_mode) {
	case DISP_PSU_NORMAL:
		if (msg == KBD_MSG_SW_SET_VOLTAGE) {
			state.psu_mode = DISP_PSU_SET_V;
			gui_textbox_setFrameType(&gui_TB_voltage, ROUNDED_FILLED_RECT, 2);
			// changing window
		}
		if (msg == KBD_MSG_SW_SET_CURRENT_LIMIT) {
			state.psu_mode = DISP_PSU_SET_I;
			gui_textbox_setFrameType(&gui_TB_current_limit, ROUNDED_FILLED_RECT, 2);
			// changing window
		}
		break;
	case DISP_PSU_SET_V:


		if (msg == ENCODER_RIGHT) {
			//Increase output voltage
		}
		if (msg == ENCODER_LEFT) {
			// Decrees output voltage
		}
		if (msg == ENCODER_SW) {
			// Toggle fine coarse adjustments
		}
		if(msg == KBD_MSG_SW_ESC){
			state.psu_mode = DISP_PSU_NORMAL;
			gui_textbox_setFrameType(&gui_TB_current_limit, ROUNDED_RECT, 2);
		}
		if (msg == KBD_MSG_SW_ENTER) {
			state.psu_mode = DISP_PSU_NORMAL;
			gui_textbox_setFrameType(&gui_TB_current_limit, ROUNDED_RECT, 2);
		}
		if (kbd_return_key_char(msg)){

			memset(state.backup_buffor,0,10);
			strncpy(state.backup_buffor,gui_TB_voltage.str,10);


			gui_textbox_deleteText(&gui_TB_voltage);
			if (kbd_return_key_char(msg) == '.'){
				gui_textbox_addChar(&gui_TB_voltage, '0');
			}
			gui_textbox_addChar(&gui_TB_voltage, kbd_return_key_char(msg));
			gui_textbox_addChar(&gui_TB_voltage, ' ');
			gui_textbox_addChar(&gui_TB_voltage, 'V');
			gui_textbox_setCursorPos(&gui_TB_voltage, CURSOR_POSSITION, kbd_return_key_char(msg)=='.'? 2 : 1);
			gui_TB_voltage.cursor_blink_mode = BLINK_BAR; // a function needed
			//gui_textbox_
			state.psu_mode = DISP_PSU_SET_V_KBD;
		}
		break;
	case DISP_PSU_SET_V_KBD:
		if (msg == ENCODER_RIGHT) {
			//Increase output voltage
		}
		if (msg == ENCODER_LEFT) {
			// Decrees output voltage
		}
		if (msg == ENCODER_SW) {
			// Toggle fine coarse adjustments
		}
		if (msg == KBD_MSG_SW_ESC) {
			state.psu_mode = DISP_PSU_NORMAL;
			gui_textbox_setFrameType(&gui_TB_current_limit, ROUNDED_RECT, 2);
		}
		if (msg == KBD_MSG_SW_ENTER) {
			state.psu_mode = DISP_PSU_NORMAL;
			gui_textbox_setFrameType(&gui_TB_current_limit, ROUNDED_RECT, 2);
		}
		if (kbd_return_key_char(msg)){

		}
		break;
	case DISP_PSU_SET_I:
		break;
	default:
		break;
	}
}

extern u8g2_t u8g2;



void display_proc(void) {

	switch (state.window) {
	case DISPLAY_WINDOW_WELCOME:
		//display_current_limit(mode,1000);

		u8g2_ClearBuffer(&u8g2);
		u8g2_DrawFrame(&u8g2, 0, 0, 128, 64);
		u8g2_SetFont(&u8g2, u8g2_font_inr16_mr);
		u8g2_DrawUTF8(&u8g2, 15, 40, "WELCOME");
		u8g2_SendBuffer(&u8g2);

		HAL_Delay(1000);

		state.window = DISPLAY_WINDOW_MENU;

		break;
	case DISPLAY_WINDOW_MENU:

		u8g2_ClearBuffer(&u8g2);
		gui_textbox_display(&gui_TB_voltage);
		gui_textbox_display(&gui_TB_current_limit);
		gui_textbox_display(&gui_TB_current_measured);
		gui_textbox_display(&gui_TB_voltage_measured);
		gui_textbox_display(&gui_TB_CC);
		u8g2_SendBuffer(&u8g2);

		debug("Are we here?");

		break;
	case DISPLAY_WINDOW_PSU:

		break;
	case DISPLAY_WINDOW_CHARGER:

		break;
	default:

		break;
	}

}

void app_run(void) {

	gui_textbox_init(&gui_TB_voltage, gui_TB_voltage_array,
			sizeof(gui_TB_voltage_array), 0, 0, 30, 16);
	gui_textbox_setFrameType(&gui_TB_voltage, ROUNDED_RECT, 2);
	gui_textbox_addText(&gui_TB_voltage, "SET\0");
	gui_textbox_setAllignHV(&gui_TB_voltage, H_ALIGN_CENTER, V_ALIGN_CENTER);
	gui_textbox_setFont(&gui_TB_voltage, (uint8_t*)u8g2_font_6x12_mf);
	gui_textbox_setCursorPos(&gui_TB_voltage, CURSOR_POSSITION, 4);
	gui_TB_voltage.cursor_blink_mode = BLINK_NO;

	gui_textbox_init(&gui_TB_current_limit, gui_TB_current_limit_array,
			sizeof(gui_TB_current_limit_array), 31, 0, 97, 16);
	gui_textbox_setFrameType(&gui_TB_current_limit, ROUNDED_RECT, 2);
	gui_textbox_addText(&gui_TB_current_limit, "12.4V 1.00A\0");
	gui_textbox_setAllignHV(&gui_TB_current_limit, H_ALIGN_CENTER,
			V_ALIGN_CENTER);
	gui_textbox_setFont(&gui_TB_current_limit, (uint8_t*)u8g2_font_6x12_mf);
	gui_TB_current_limit.cursor_blink_mode = BLINK_NO;

	gui_textbox_init(&gui_TB_current_measured, gui_TB_current_measured_array,
			sizeof(gui_TB_current_measured_array), 48, 17, 80, 22);
	gui_textbox_setFrameType(&gui_TB_current_measured, ROUNDED_RECT, 2);
	gui_textbox_addText(&gui_TB_current_measured, "1.001A\0");
	gui_textbox_setAllignHV(&gui_TB_current_measured, H_ALIGN_CENTER,
			V_ALIGN_CENTER);
	gui_textbox_setFont(&gui_TB_current_measured, (uint8_t*)u8g2_font_10x20_mf);

	gui_textbox_init(&gui_TB_voltage_measured, gui_TB_voltage_measured_array,
			sizeof(gui_TB_voltage_measured_array), 48, 41, 80, 22);
	gui_textbox_setFrameType(&gui_TB_voltage_measured, ROUNDED_RECT, 2);
	gui_textbox_addText(&gui_TB_voltage_measured, "12.6V\0");
	gui_textbox_setAllignHV(&gui_TB_voltage_measured, H_ALIGN_CENTER,
			V_ALIGN_CENTER);
	gui_textbox_setFont(&gui_TB_voltage_measured, (uint8_t*)u8g2_font_10x20_mf);

	gui_textbox_init(&gui_TB_CC, gui_TB_CC_array, sizeof(gui_TB_CC_array), 0,
			20, 46, 40);
	gui_textbox_setFrameType(&gui_TB_CC, ROUNDED_FILLED_RECT, 5);
	gui_textbox_addText(&gui_TB_CC, "CC\0");
	gui_textbox_setAllignHV(&gui_TB_CC, H_ALIGN_CENTER, V_ALIGN_CENTER);
	gui_textbox_setFont(&gui_TB_CC, (uint8_t*)u8g2_font_10x20_mf);


	app_tick_init();
	app_register_tick_callback(app_display_timer_proc);
	app_register_tick_callback(kbd_timer);

	psu_init(&psu);

	kbd_enc_queue = circular_buf_init(kbd_enc_queue_buffor,MESAGE_ENC_BUFFOR_SIZE);

	kbd_init(&kbd,kbd_enc_queue);

	state.window = DISPLAY_WINDOW_WELCOME;

	while (1) {
		//circular_buf_put(queue,current_limit.x);

		state_machine();

		if (app_is_display_timeout()) {
			display_proc();
		}
	}
}

//WINDOW_WELCOME
void window_welcome_start(void){

}

void window_welcome_event_handler(kbd_msg_t event){

}

void window_welcome_exit(void){

}

void window_welcome_display(void){

}

//WINDOW_MENU
void window_menu_start(void){

}

void window_menu_event_handler(kbd_msg_t event){

}

void window_menu_exit(void){

}

void window_menu_display(void){

}

	//WINDOW_PSU
void window_PSU_start(void){

}

void window_PSU_event_handler(kbd_msg_t event){

}

void window_PSU_exit(void){

}

void window_PSU_display(void){

}

//WINDOW_CHARGER
void window_charger_start(void){

}

void window_charger_event_handler(kbd_msg_t event){

}

void window_charger_exit(void){

}

void window_charger_display(void){

}

typedef struct{
	display_window_t window;
	void (*on_start)(void);
	void (*on_event)(kbd_msg_t event);
	void (*on_exit)(void);
	void (*on_display)(void);
}window_t;
#define DISPLAY_WINDOW_NUMBER 4
window_t window_table[DISPLAY_WINDOW_NUMBER] = {
		{DISPLAY_WINDOW_WELCOME,window_welcome_start,	window_welcome_event_handler,	window_welcome_exit,	window_welcome_display},
		{DISPLAY_WINDOW_MENU,	window_menu_start,		window_welcome_event_handler,	window_menu_exit,		window_menu_display},
		{DISPLAY_WINDOW_PSU,	window_PSU_start,		window_PSU_event_handler,		window_PSU_exit,		window_PSU_display},
		{DISPLAY_WINDOW_CHARGER,window_charger_start,	window_charger_event_handler,	window_charger_exit,	window_charger_display}
};

window_t window_active;

void executor_set(display_window_t set_window) {

	for (uint8_t a = 0; a < DISPLAY_WINDOW_NUMBER; a++) {
		if (window_table[a].window == set_window) {
			window_active = window_table[a];
		}
	}
}

extern UART_HandleTypeDef huart3;

PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART1 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}
