/*
 * encoder.c
 *
 *  Created on: 22 θώνÿ 2017 γ.
 *      Author: alpha-user
 */
#include <hal.h>
#include "encoder.h"


void EncoderInit() {

  qeiInit();
  palSetPadMode(GPIOA, BTN_PIN, PAL_MODE_INPUT_PULLUP);
  palSetPadMode(GPIOA, 6, PAL_MODE_INPUT_PULLUP);
  palSetPadMode(GPIOA, 7, PAL_MODE_INPUT_PULLUP);
  QEIConfig conf = {.mode = QEI_MODE_QUADRATURE, .dirinv = QEI_DIRINV_FALSE,
                    .resolution = QEI_SINGLE_EDGE};
  qeiStart(&QEID3, &conf);
  qeiEnable(&QEID3);
  qeiSetMax(&QEID3, 2);
}


encoder_button_state_t EncoderBtnStatus() {
  static uint8_t last_pin_state = 0;
  static encoder_button_state_t button_state = BTN_UNINIT;
  static encoder_button_state_t last_button_state = BTN_UNINIT;
  static uint8_t debounce_counter = DEBOUNCE_COUNTER_DEFAULT;
  static uint8_t longpress_counter = LONGPRESS_COUNTER_DEFAULT;

  uint8_t current_pin_state = palReadPad(GPIOA, BTN_PIN);
  if (last_pin_state ^ current_pin_state) {
    last_pin_state = current_pin_state;
    button_state = BTN_DEBOUNCE;
    debounce_counter = DEBOUNCE_COUNTER_DEFAULT;
    longpress_counter = LONGPRESS_COUNTER_DEFAULT;
    return BTN_DEBOUNCE;
  }

//  if ( (button_state == BTN_DEBOUNCE) && debounce_counter ) {
//    if (--debounce_counter) return BTN_DEBOUNCE;
//  }


  // current_pin_state is debounced from this point

  button_state = current_pin_state ? BTN_UP : BTN_DOWN;

  if (button_state == BTN_DOWN) {

    if (longpress_counter == LONGPRESS_COUNTER_DEFAULT)
      last_button_state = BTN_DOWN;

    if (longpress_counter) {
      if (!(--longpress_counter)) {
        last_button_state = BTN_UP;
        return BTN_LONGPRESS;
      }
    }

  } else {
    if (last_button_state == BTN_DOWN) {
      last_button_state = BTN_UP;
      return BTN_DOWN;
    }
  }

  return BTN_UP;

}

uint8_t EncoderUpdated() {
  return qeiUpdate(&QEID3) ? 1 : 0;
}

uint16_t EncoderValue() {
  return (uint16_t) qeiGetCount(&QEID3);
}

void EncoderSetMax(uint16_t value) {
  qeiSetMax(&QEID3, value);
}

