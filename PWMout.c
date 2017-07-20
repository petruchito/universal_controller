/*
 * PWM.c
 *
 *  Created on: 20 июля 2017 г.
 *      Author: alpha-user
 */
#include "hal.h"
#include "PWMout.h"

void PWMInit() {

  palSetPadMode(GPIOB, PWM_PIN , PAL_MODE_STM32_ALTERNATE_PUSHPULL);
  static const PWMConfig pwm_config1 = {
                                       .frequency = 10000,
                                       .period = 10000,
                                       .callback = NULL,
                                       .channels = {
                                                    {PWM_OUTPUT_DISABLED, NULL},
                                                    {PWM_OUTPUT_DISABLED, NULL},
                                                    {PWM_OUTPUT_ACTIVE_HIGH, NULL},
                                                    {PWM_OUTPUT_DISABLED, NULL}
                                       },
                                       .cr2 = 0,
                                       .dier = 0
  };

  pwmStart(&PWMD4,&pwm_config1);

}

void PWMSetDutyCycle (uint16_t duty) {
  pwmEnableChannel(&PWMD4, 2, duty);
}
