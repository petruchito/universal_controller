/*
 * pid.h
 *
 *  Created on: 18 июля 2017 г.
 *      Author: alpha-user
 */

#ifndef PID_H_
#define PID_H_
#include "hal.h"

typedef struct {
  float kc,ti,td,out_max,out_min;
  uint8_t enabled;
} pid_params_t;


uint8_t PidTick(pid_params_t *params, uint16_t sv, float pv);
void PidResetIntegral(void);

#endif /* PID_H_ */
