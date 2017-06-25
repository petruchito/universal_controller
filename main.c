#include <string.h>
#include <stdio.h>
#include "hal.h"
#include "ch.h"
#include "encoder.h"
#include "iic1602.h"
#include "MAX6675.h"

static thread_t *update_display_thd;

#define SCR_ADJUST 0
#define SCR_HEATING 1
#define SCR_MENU 2

typedef enum {
  UPD_BUTTON = 0b1,
  UPD_ENCODER = 0b10,
  UPD_SET_TEMPERATURE = 0b100,
  UPD_TEMPERATURE = 0b1000,
  UPD_PWM = 0b1<<4,
  UPD_SET_PWM = 0b10<<4
} state_updated_t;

typedef struct {
  uint16_t encoder_cnt;
  encoder_button_state_t encoder_button;
  float temperature;
  uint16_t set_temperature;
  uint8_t pwm;
  state_updated_t updated;
  mutex_t mutex;
} sys_state_t;

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

    if(EncoderUpdated()) {
      chMtxLock(&system_state.mutex);
      system_state.updated |= UPD_ENCODER;
      system_state.encoder_cnt = EncoderValue();
      chMtxUnlock(&system_state.mutex);
    }

    chThdSleepMilliseconds(10);
  }
}

void RenderTemperature(float temperature) {
  LCD_goto(1, 7);
  char tempstr[10];
  sprintf(tempstr, "%7.2f\337C", temperature);
  LCD_string(tempstr);
}

void RenderSetTemperature(uint16_t set_temperature) {
  LCD_goto(1, 0);
  char tempstr[6];
  sprintf(tempstr, "%3d\337C", set_temperature);
  LCD_string(tempstr);
}

void RenderAdjustScreen(sys_state_t *state,
                       uint8_t *current_screen,
                       uint8_t *redraw) {

  if (state->updated & UPD_BUTTON) {
    if (state->encoder_button == BTN_DOWN) {
      //TODO: START HEATING
      *current_screen = SCR_HEATING;
      *redraw = TRUE;
      return;
    }
    if (state->encoder_button == BTN_LONGPRESS) {
      *current_screen = SCR_MENU;
      *redraw = TRUE;
      return;
    }
  }

  if (state->updated & UPD_TEMPERATURE) {
    RenderTemperature(state->temperature);
  }

  if (state->updated & UPD_SET_TEMPERATURE) {
    RenderSetTemperature(state->set_temperature);
  }

  if (*redraw == TRUE) {
    LCD_goto(2,1);
    LCD_string("PRESS TO START");
    *redraw = FALSE;
  }
}

void RenderHeatingScreen(sys_state_t *state,
                       uint8_t *current_screen,
                       uint8_t *redraw) {

  if (state->updated & UPD_BUTTON) {
    if (state->encoder_button == BTN_DOWN) {
      //TODO: STOP HEATING
      *current_screen = SCR_ADJUST;
      *redraw = TRUE;
      return;
    }
    if (state->encoder_button == BTN_LONGPRESS) {
      //TODO: STOP HEATING
      *current_screen = SCR_MENU;
      *redraw = TRUE;
      return;
    }
  }

  if (state->updated & UPD_TEMPERATURE) {
    RenderTemperature(state->temperature);
  }

  if (state->updated & UPD_SET_TEMPERATURE) {
    RenderSetTemperature(state->set_temperature);
  }

  if (*redraw == TRUE) {
    LCD_goto(2,0);
    LCD_string("HEATING");
    *redraw = FALSE;
  }
}

void RenderMenu(sys_state_t *state,
                uint8_t *current_screen,
                uint8_t *redraw) {

  if (state->updated & UPD_BUTTON) {
    if (state->encoder_button == BTN_LONGPRESS) {
      //TODO: STOP HEATING
      *current_screen = SCR_ADJUST;
      *redraw = TRUE;
      return;
    }
  }

  if (*redraw == TRUE) {
    LCD_goto(1,5);
    LCD_string("MENU");
    *redraw = FALSE;
  }
}

static THD_WORKING_AREA(UpdateDisplayWA, 1000);

static THD_FUNCTION(UpdateDisplay, arg) {
  (void) arg;
  chRegSetThreadName("UpdateDisplay");
  LCD_init();

  uint8_t current_screen = SCR_ADJUST;
  static uint8_t redraw = TRUE;

  while(1) {

    chMtxLock(&system_state.mutex);
    sys_state_t current_sate = system_state;
    system_state.updated = 0;
    chMtxUnlock(&system_state.mutex);

    switch (current_screen) {
    case SCR_ADJUST:
      RenderAdjustScreen(&current_sate, &current_screen, &redraw);
      break;
    case SCR_HEATING:
      RenderHeatingScreen(&current_sate, &current_screen, &redraw);
      break;
    case SCR_MENU:
      RenderMenu(&current_sate, &current_screen, &redraw);
      break;
    }

    if (redraw) {
      LCD_clear();
      continue;
    }

    while (!system_state.updated) chThdSleepMilliseconds(100);
  }

//  LCD_string("Running...");
//  int i,j = 0;
//  while (1) {
//    LCD_goto(1,14);
//    LCD_string("\337C");
//
//    LCD_goto(1,7);
//    char tempstr[8];
//    sprintf(tempstr, "%+7.2f", temp);
//    LCD_string(tempstr);
//
//    LCD_goto(2,0);
//    LCD_char('0'+i);
//    i++;
//    if (i>9) i = 0;
//    if(updated & UPD_STATUS_POSITION) {
//      LCD_goto(1,6);
//      sprintf(tempstr,"%9d",encoder_cnt);
//      LCD_string(tempstr);
//      updated &= ~UPD_STATUS_POSITION;
//    }
//    if (updated & UPD_STATUS_BUTTON) {
//      LCD_goto(2,10);
//      LCD_string(" ");
//      LCD_goto (2,6);
//      if (encoder_button) {
//        LCD_string("   ");
//      } else {
//        LCD_string("BTN");
//        LCD_goto(2,2);
//        LCD_char('0'+j);
//        if (j>8) j = 0;
//        j++;
//
//
//
//      }
//      updated &= ~UPD_STATUS_BUTTON;
//    }
//    if (updated & UPD_STATUS_BTN_LONG) {
//      LCD_goto(2,10);
//      LCD_string("L");
//      updated &= ~UPD_STATUS_BTN_LONG;
//    }
//    chThdSleepMilliseconds(100);
//  }
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
