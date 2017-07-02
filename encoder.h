/*
 * encoder.h
 *
 *  Created on: 22 θώνÿ 2017 γ.
 *      Author: alpha-user
 */

#ifndef ENCODER_H_
#define ENCODER_H_

#include "qei.h"

#define BTN_PIN 5
#define DEBOUNCE_COUNTER_DEFAULT 2
#define LONGPRESS_COUNTER_DEFAULT 50

typedef enum {
BTN_UNINIT = 0,
BTN_DEBOUNCE = 0b1,
BTN_UP = 0b1000,
BTN_DOWN = 0b0100,
BTN_LONGPRESS = 0b0010
} encoder_button_state_t;

encoder_button_state_t EncoderBtnStatus(void);
void EncoderInit(void);
uint8_t EncoderUpdated(void);
uint16_t EncoderValue(void);
void EncoderSetMax(uint16_t value);
#endif /* ENCODER_H_ */
