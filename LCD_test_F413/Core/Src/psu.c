/*
 * psu.c
 *
 *  Created on: May 22, 2024
 *      Author: marcin
 */

#include "psu.h"

void psu_read_ADC(PSU_control_s *self){

}

void psu_write_DAC(PSU_control_s *self){

}

void psu_IO_proces(PSU_control_s *self){
	psu_read_ADC(self);
	// add filtering here
	psu_write_DAC(self);
}

void psu_set_output_voltage(PSU_control_s *self, uint16_t voltage){
	if (voltage <= PSU_MAX_VOLTAGE_mV) {
		self->set_voltage_mV = voltage;
		//psu_updateDAC(self);
	} else {
		errorHandler();
	}
}


uint16_t psu_get_output_voltage(PSU_control_s *self){
	return self->set_voltage_mV;
}

void psu_set_current_limit(PSU_control_s *self, uint16_t current_limit){
	if (current_limit <= PSU_MAX_CURRENT_mA) {
		self->set_current_limit_mA = current_limit;
		//psu_UpdateDAC(self);
	} else {
		errorHandler();
	}
}

inline uint16_t psu_get_current_limit(PSU_control_s *self){
	return self->set_current_limit_mA;
}

inline uint16_t psu_get_measured_voltage(PSU_control_s *self){
	return self->measured_voltage_mV;
}

inline uint16_t psu_get_measured_current(PSU_control_s *self){
	return self->measured_current_mA;
}

inline uint8_t psu_is_current_limitActive(PSU_control_s *self){
	return self->current_limit_active_flag;
}

inline void psu_set_relay(PSU_control_s *self,uint8_t state){
	self->output_relay_flag = state;
}


