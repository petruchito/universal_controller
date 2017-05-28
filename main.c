#include <string.h>
#include <stdio.h>
#include "hal.h"
#include "ch.h"
#include "qei.h"
#include "iic1602.h"
qeicnt_t cnt=0;
uint8_t updated = 0;
QEIDriver qeip;

static THD_WORKING_AREA(EncoderWA, 256);

static THD_FUNCTION(EncoderThread, arg) {
  (void) arg;

  while(TRUE) {
    if(qeiUpdate(&qeip)){
      cnt = qeiGetCount(&qeip);
      updated = 1;
    }

    chThdSleepMilliseconds(50);
  }
};

static THD_WORKING_AREA(UpdateDisplayWA, 400);

static THD_FUNCTION(UpdateDisplay, arg) {
  (void) arg;
  chRegSetThreadName("UpdateDisplay");
  LCD_init();
  LCD_string("Running...");
  int i = 0;
  while (1) {
    LCD_goto(2,0);
    LCD_char('0'+i);
    i++;
    if (i>9) i = 0;
    if(updated) {
      LCD_goto(1,6);
        char value[10];
        sprintf(value,"%9d",cnt);
        LCD_string(value);
        updated = 0;
     }
    chThdSleepMilliseconds(100);
  }
}

int main(void) {
   halInit();
    chSysInit();
    qeiInit();


    palSetPadMode(GPIOA, 6, PAL_MODE_INPUT_PULLUP); /* encoder A */
    palSetPadMode(GPIOA, 7, PAL_MODE_INPUT_PULLUP); /* encoder B */

    QEIConfig conf = { .mode = QEI_MODE_QUADRATURE,
                       .dirinv = QEI_DIRINV_FALSE,
                       .resolution = QEI_SINGLE_EDGE };

    qeip.tim = STM32_TIM3;
    qeiObjectInit(&qeip);
    qeiStart(&qeip, &conf);
    qeiEnable(&qeip);
    qeip.tim->ARR=200;



    chThdCreateStatic(EncoderWA,
                      sizeof(EncoderWA),
                      NORMALPRIO,
                      EncoderThread,
                      NULL);



//    chThdCreateStatic(PollTemperatureThreadWA,
//                      sizeof(PollTemperatureThreadWA),
//                      NORMALPRIO,PollTemperatureThread,
//                      NULL);

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
