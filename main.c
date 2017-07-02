#include "hal.h"
#include "ch.h"
#include "encoder.h"
#include "iic1602.h"
#include "MAX6675.h"
#include "interface.h"
#include "sysstate.h"

static thread_t *update_display_thd;

static sys_state_t system_state;

static THD_WORKING_AREA(MAX6675WA, 256);


static THD_FUNCTION(MAX6675Thread, arg) {
  (void) arg;

  MAX6675Init();
  while(TRUE) {
    chMtxLock(&system_state.mutex);
    system_state.temperature = MAX6675Read();
    system_state.updated = UPD_TEMPERATURE;
    chMtxUnlock(&system_state.mutex);
    chThdSleepMilliseconds(500);
  }
}


static THD_WORKING_AREA(EncoderWA, 256);

static THD_FUNCTION(EncoderThread, arg) {
  (void) arg;

  while(TRUE) {

    encoder_button_state_t btn = EncoderBtnStatus();
    if ((btn == BTN_DOWN) || (btn == BTN_LONGPRESS)) {
      chMtxLock(&system_state.mutex);
      system_state.encoder_button = btn;
      system_state.updated |= UPD_BUTTON;
      chMtxUnlock(&system_state.mutex);
    }

    uint16_t cnt = EncoderValue();
    if(cnt != system_state.encoder_cnt) {
      chMtxLock(&system_state.mutex);
      system_state.updated |= UPD_ENCODER;
      system_state.encoder_cnt = cnt;
      chMtxUnlock(&system_state.mutex);
    }

    chThdSleepMilliseconds(10);
  }
}



static THD_WORKING_AREA(UpdateDisplayWA, 1500);

static THD_FUNCTION(UpdateDisplay, arg) {
  (void) arg;
  chRegSetThreadName("UpdateDisplay");
  LCD_init();

  while(1) {
    chMtxLock(&system_state.mutex);
    sys_state_t current_sate = system_state;
    system_state.updated = 0;
    chMtxUnlock(&system_state.mutex);

    RenderInterface(&current_sate);

    system_state.interface = current_sate.interface;
    system_state.return_interface = current_sate.return_interface;

    if(current_sate.updated & UPD_SET_TEMPERATURE) {
      chMtxLock(&system_state.mutex);
      system_state.updated|=current_sate.updated & UPD_SET_TEMPERATURE;
      system_state.set_temperature = current_sate.set_temperature;
      chMtxUnlock(&system_state.mutex);
    }

    while (!system_state.updated) chThdSleepMilliseconds(100);
  }
}



int main(void) {
  halInit();
  chSysInit();
  chMtxObjectInit(&system_state.mutex);
  EncoderInit();
  chThdCreateStatic(EncoderWA,
                    sizeof(EncoderWA),
                    NORMALPRIO,
                    EncoderThread,
                    NULL);


  chThdCreateStatic(MAX6675WA,
                    sizeof(MAX6675WA),
                    NORMALPRIO,
                    MAX6675Thread,
                    NULL);


  update_display_thd = chThdCreateStatic(UpdateDisplayWA,
                    sizeof(UpdateDisplayWA),
                    LOWPRIO,
                    UpdateDisplay,
                    NULL);


  while(1) {
    palSetPad(GPIOC, GPIOC_LED);
    chThdSleepMilliseconds(1000);
    palClearPad(GPIOC, GPIOC_LED);
    chThdSleepMilliseconds(1000);

  }
  return 0;
}
