/*
 * PWM.h
 *
 *  Created on: 20 июля 2017 г.
 *      Author: alpha-user
 */

#ifndef PWMOUT_H_
#define PWMOUT_H_

#define PWM_PIN 8

void PWMInit(void);

void PWMSetDutyCycle (uint8_t duty);

#endif /* PWMOUT_H_ */
