/*
 * pid.c
 *
 *  Created on: 18 июля 2017 г.
 *      Author: alpha-user
 */

#include "pid.h"

static float i = 0, last_pv = 0;

uint8_t PidTick(pid_params_t *params, uint16_t sv, float pv) {
  float e = sv-pv;
  float p = e * params->kc;
  i += (1/params->ti) * e;
  float d = params->td * (pv - last_pv);
  last_pv = pv;

  float out = (p+d+i);
  if (out > params->out_max) {
    return 255;
  } else if (out < params->out_min) {
    return 0;
  } else {
    return (uint8_t) 255*(out / (params->out_max-params->out_min) );
  }

}

void PidResetIntegral() {
  i = 0;
}
