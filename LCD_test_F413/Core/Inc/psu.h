/*
 * psu.h
 *
 *  Created on: May 22, 2024
 *      Author: marci
 */

#ifndef INC_PSU_H_
#define INC_PSU_H_

#include "inttypes.h"


#define PSU_MAX_VOLTAGE_mV 15000
#define PSU_MAX_CURRENT_mA 4000

typedef struct {
	uint16_t set_voltage_mV;
	uint16_t set_current_limit_mA;
	uint16_t measured_voltage_mV;
	uint16_t measured_current_mA;
	uint8_t current_limit_active_flag;
	uint8_t output_relay_flag;
}PSU_control_t;

void psu_init(PSU_control_t *self);

void psu_set_output_voltage(PSU_control_t *self, uint16_t voltage);

uint16_t psu_get_output_voltage(PSU_control_t *self);

void psu_set_current_limit(PSU_control_t *self, uint16_t current_limit);

uint16_t psu_get_current_limit(PSU_control_t *self);

uint16_t psu_get_measured_voltage(PSU_control_t *self);

uint16_t psu_get_measured_current(PSU_control_t *self);

uint8_t psu_is_current_limitActive(PSU_control_t *self);

void psu_set_relay(PSU_control_t *self,uint8_t state);

void psu_IO_proces(PSU_control_t *self);


#endif /* INC_PSU_H_ */
