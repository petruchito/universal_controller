#include <string.h>
#include <stdio.h>
#include "hal.h"
#include "ch.h"
#include "encoder.h"
#include "iic1602.h"
#include "MAX6675.h"
uint16_t encoder_cnt=0;
uint8_t encoder_button = 0;
uint8_t updated = 0;
float temp = 0;

#define UPD_STATUS_BUTTON 0b1
#define UPD_STATUS_POSITION 0b10
#define UPD_STATUS_BTN_LONG 0b100




static THD_WORKING_AREA(MAX6675WA, 256);


static THD_FUNCTION(MAX6675Thread, arg) {
  (void) arg;

  MAX6675Init();
  while(TRUE) {
    temp = MAX6675Read();
    chThdSleepMilliseconds(500);
  }
}


static THD_WORKING_AREA(EncoderWA, 256);

static THD_FUNCTION(EncoderThread, arg) {
  (void) arg;

  while(TRUE) {

    encoder_button_state_t current_button_status = EncoderBtnStatus();
    if (current_button_status == BTN_DOWN) {
      updated |= UPD_STATUS_BUTTON;
    }
    if (current_button_status == BTN_LONGPRESS) {
      updated |= UPD_STATUS_BTN_LONG;
    }



    if(EncoderUpdated()) {
      updated |= UPD_STATUS_POSITION;
      encoder_cnt = EncoderValue();
    }

    chThdSleepMilliseconds(30);
  }
}

static THD_WORKING_AREA(UpdateDisplayWA, 1000);

static THD_FUNCTION(UpdateDisplay, arg) {
  (void) arg;
  chRegSetThreadName("UpdateDisplay");
  LCD_init();
  //LCD_string("Running...");
  int i,j = 0;
  while (1) {
    LCD_goto(1,0);
    char tempstr[6];
    sprintf(tempstr, "%3.2f", temp);
    LCD_string(tempstr);
    LCD_goto(2,0);
    LCD_char('0'+i);
    i++;
    if (i>9) i = 0;
    if(updated & UPD_STATUS_POSITION) {
      LCD_goto(1,6);
      sprintf(tempstr,"%9d",encoder_cnt);
      LCD_string(tempstr);
      updated &= ~UPD_STATUS_POSITION;
    }
    if (updated & UPD_STATUS_BUTTON) {
      LCD_goto(2,10);
      LCD_string(" ");
      LCD_goto (2,6);
      if (encoder_button) {
        LCD_string("   ");
      } else {
        LCD_string("BTN");
        LCD_goto(2,2);
        LCD_char('0'+j);
        if (j>8) j = 0;
        j++;



      }
      updated &= ~UPD_STATUS_BUTTON;
    }
    if (updated & UPD_STATUS_BTN_LONG) {
      LCD_goto(2,10);
      LCD_string("L");
      updated &= ~UPD_STATUS_BTN_LONG;
    }
    chThdSleepMilliseconds(100);
  }
}



int main(void) {
  halInit();
  chSysInit();
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


  chThdCreateStatic(UpdateDisplayWA,
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
