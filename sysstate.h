/*
 * sysstate.h
 *
 *  Created on: 26 θώνÿ 2017 γ.
 *      Author: alpha-user
 */

#ifndef SYSSTATE_H_
#define SYSSTATE_H_

#include "encoder.h"
#include "interface.h"

typedef enum {
  UPD_BUTTON = 0b1,
  UPD_ENCODER = 0b10,
  UPD_SET_TEMPERATURE = 0b100,
  UPD_TEMPERATURE = 0b1000,
  UPD_PWM = 0b1<<4,
  UPD_SET_PWM = 0b10<<4
} state_updated_t;

typedef struct SystemState {
  uint16_t encoder_cnt;
  encoder_button_state_t encoder_button;
  float temperature;
  uint16_t set_temperature;
  uint8_t pwm;
  interface_t *interface, *return_interface[MAX_INTERFACE_DEPTH];
  uint8_t return_interfaces_top;
  state_updated_t updated;
  mutex_t mutex;
} sys_state_t;


#endif /* SYSSTATE_H_ */
