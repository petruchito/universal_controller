#include <string.h>
#include "hal.h"
#include "ch.h"
#include "iic1602.h"

static THD_WORKING_AREA(PollTemperatureThreadWA, 256);

static THD_FUNCTION(PollTemperatureThread, arg) {
  (void) arg;
  chRegSetThreadName("PollTemp");
//  while(TRUE) {
//    osalThreadSleepMilliseconds(50);
//    //read temperature
//  }
};

static THD_WORKING_AREA(UpdateDisplayWA, 256);

static THD_FUNCTION(UpdateDisplay, arg) {
  (void) arg;
  chRegSetThreadName("UpdateDisplay");
  LCD_init();
}

int main(void) {
  halInit();
    chSysInit();


    chThdCreateStatic(PollTemperatureThreadWA,
                      sizeof(PollTemperatureThreadWA),
                      NORMALPRIO,PollTemperatureThread,
                      NULL);

    chThdCreateStatic(UpdateDisplayWA,
                       sizeof(UpdateDisplayWA),
                       NORMALPRIO,
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
