#include "hal.h"
#include "ch.h"
#include "encoder.h"
#include "iic1602.h"
#include "MAX6675.h"
#include "interface.h"
#include "sysstate.h"
#include "PWMout.h"
#include "pid.h"
#include <string.h>

static thread_t *update_display_thd;

static sys_state_t system_state = {
                                   .f_param = .5f,
                                   .pwm = 0,
                                   .pid_params = {
                                                  //todo: load this from eeprom
                                                  .out_max = 100,
                                                  .out_min = 0,
                                                  .kc = 1,
                                                  .ti = 10,
                                                  .td = 1
                                   }
};

static THD_WORKING_AREA(PIDWA, 256);

static THD_FUNCTION(PIDThread, arg) {
  (void) arg;
  PWMInit();
  system_state.pid_params.enabled = TRUE;
  while (TRUE) {

    if (system_state.pid_params.enabled) {
      system_state.pwm = PidTick(&system_state.pid_params,
              system_state.set_temperature,
              system_state.temperature);
      system_state.updated |= UPD_PWM;
    }
    PWMSetDutyCycle(2550000/system_state.pwm);
    chThdSleepMilliseconds(1000);
  }
}
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



static THD_WORKING_AREA(UpdateDisplayWA, 2500);

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

    system_state.f_param = current_sate.f_param;
    //TODO: add consistency mutex!

    system_state.return_interfaces_top = current_sate.return_interfaces_top;
    system_state.interface = current_sate.interface;
    memcpy(system_state.return_interface,
           current_sate.return_interface,
           sizeof(current_sate.return_interface));

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

  chThdCreateStatic(PIDWA,
                    sizeof(PIDWA),
                    NORMALPRIO,
                    PIDThread,
                    NULL);

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
