#include <string.h>
#include <stdio.h>
#include "hal.h"
#include "ch.h"
#include "qei.h"
#include "iic1602.h"
qeicnt_t encoder_cnt=0;
uint8_t encoder_button = 0;
uint8_t updated = 0;
static uint8_t rxbuf[2];

#define UPD_STATUS_BUTTON 0b1
#define UPD_STATUS_POSITION 0b10
#define UPD_STATUS_BTN_DEBOUNCE 0b100

#define BTN_PIN 5

static THD_WORKING_AREA(MAX6675WA, 256);

static THD_FUNCTION(MAX6675Thread, arg) {
  (void) arg;

  static const SPIConfig ls_spicfg = {
                                      NULL,
                                      GPIOB,
                                      12,
                                      SPI_CR1_BR_2 | SPI_CR1_BR_1  };



  palSetPadMode(GPIOB, 13, PAL_MODE_STM32_ALTERNATE_PUSHPULL);     /* SCK. */
  palSetPadMode(GPIOB, 14, PAL_MODE_STM32_ALTERNATE_PUSHPULL);     /* MISO.*/
  palSetPadMode(GPIOB, 12, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(GPIOB, 12);
  spiAcquireBus(&SPID2);              /* Acquire ownership of the bus.    */
  spiStart(&SPID2, &ls_spicfg);       /* Setup transfer parameters.       */
  spiReleaseBus(&SPID2);              /* Ownership release.               */


  while(TRUE) {
    spiAcquireBus(&SPID2);              /* Acquire ownership of the bus.    */
    spiSelect(&SPID2);                  /* Slave Select assertion.          */
    spiReceive(&SPID2, 2, rxbuf);
    spiUnselect(&SPID2);                /* Slave Select de-assertion.       */
    spiReleaseBus(&SPID2);              /* Ownership release.               */
    uint16_t i = 0;
    i = (rxbuf[0]<<8);
    i|=(rxbuf[1]);
    i=i>>3; // *0.25degC
    chThdSleepMilliseconds(500);
  }
}


static THD_WORKING_AREA(EncoderWA, 256);

static THD_FUNCTION(EncoderThread, arg) {
  (void) arg;
  uint8_t current_button_status = 0;


  while(TRUE) {
    current_button_status = palReadPad(GPIOA, BTN_PIN);
      if (current_button_status ^ encoder_button) {
        encoder_button = current_button_status;
        updated |= UPD_STATUS_BTN_DEBOUNCE;
      } else {
        if (updated & UPD_STATUS_BTN_DEBOUNCE) {
          updated |= UPD_STATUS_BUTTON;
          updated &= ~UPD_STATUS_BTN_DEBOUNCE;
        }
      }

    if(qeiUpdate(&QEID3)){
      encoder_cnt = qeiGetCount(&QEID3);
      updated |= UPD_STATUS_POSITION;
    }

    chThdSleepMilliseconds(50);
  }
}

static THD_WORKING_AREA(UpdateDisplayWA, 400);

static THD_FUNCTION(UpdateDisplay, arg) {
  (void) arg;
  chRegSetThreadName("UpdateDisplay");
  LCD_init();
  LCD_string("Running...");
  int i,j = 0;
  while (1) {
    LCD_goto(2,0);
    LCD_char('0'+i);
    i++;
    if (i>9) i = 0;
    if(updated & UPD_STATUS_POSITION) {
      LCD_goto(1,6);
        char value[10];
        sprintf(value,"%9d",encoder_cnt);
        LCD_string(value);
        updated &= ~UPD_STATUS_POSITION;
     }
    if (updated & UPD_STATUS_BUTTON) {
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
    chThdSleepMilliseconds(100);
  }
}

int main(void) {
   halInit();
    chSysInit();
    qeiInit();

    palSetPadMode(GPIOA, BTN_PIN, PAL_MODE_INPUT_PULLUP); /* encoder button */
    palSetPadMode(GPIOA, 6, PAL_MODE_INPUT_PULLUP); /* encoder A */
    palSetPadMode(GPIOA, 7, PAL_MODE_INPUT_PULLUP); /* encoder B */

    QEIConfig conf = { .mode = QEI_MODE_QUADRATURE,
                       .dirinv = QEI_DIRINV_FALSE,
                       .resolution = QEI_SINGLE_EDGE };

    qeiStart(&QEID3, &conf);
    qeiEnable(&QEID3);
    qeiSetMax(&QEID3,200);



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
