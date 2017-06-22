/*
 * MAX6675.c
 *
 *  Created on: 23 θώνÿ 2017 γ.
 *      Author: alpha-user
 */

#include "MAX6675.h"

void MAX6675Init() {
  static const SPIConfig ls_spicfg = {NULL,
  GPIOB,
                                      12,
                                      SPI_CR1_BR_2 | SPI_CR1_BR_1};
  palSetPadMode(GPIOB, 13, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
  palSetPadMode(GPIOB, 14, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
  palSetPadMode(GPIOB, 12, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPad(GPIOB, 12);
  spiAcquireBus(&SPID2); /* Acquire ownership of the bus.    */
  spiStart(&SPID2, &ls_spicfg); /* Setup transfer parameters.       */
  spiReleaseBus(&SPID2); /* Ownership release.               */
}

float MAX6675Read() {

  uint8_t rxbuf[2] = {0,0};

  spiAcquireBus(&SPID2); /* Acquire ownership of the bus.    */
  spiSelect(&SPID2); /* Slave Select assertion.          */
  spiReceive(&SPID2, 2, rxbuf);
  spiUnselect(&SPID2); /* Slave Select de-assertion.       */
  spiReleaseBus(&SPID2); /* Ownership release.               */

  uint16_t i = (rxbuf[0] << 8)|(rxbuf[1]);
  i = i >> 3; // *0.25degC
  return (float) i*0.25;

}
