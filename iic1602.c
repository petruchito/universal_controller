/*
 * iic1602.c
 *
 *  Created on: 09 мая 2017 г.
 *      Author: alpha-user
 */

#include "iic1602.h"


systime_t timeout = MS2ST(4);

static const I2CConfig i2cfg = {
    OPMODE_I2C,
    100000,
    FAST_DUTY_CYCLE_2,
};

void I2C_transmit_4bit(uint8_t value) {

  uint8_t pin_mapped_value = 0;
  pin_mapped_value |= (((value>>0) & 1)<<LCD_I2C_D4);
  pin_mapped_value |= (((value>>1) & 1)<<LCD_I2C_D5);
  pin_mapped_value |= (((value>>2) & 1)<<LCD_I2C_D6);
  pin_mapped_value |= (((value>>3) & 1)<<LCD_I2C_D7);

  pin_mapped_value |= (((value>>5) & 1)<<LCD_I2C_BL);
  pin_mapped_value |= (((value>>6) & 1)<<LCD_I2C_RS);
  pin_mapped_value |= (((value>>7) & 1)<<LCD_I2C_RW);

  i2cAcquireBus(&I2CD1);

  pin_mapped_value |= (1<<LCD_I2C_E);
  i2cMasterTransmitTimeout(&I2CD1,LCD_I2C_ADDR,&pin_mapped_value,1,NULL,0,timeout);

  pin_mapped_value &= ~(1<<LCD_I2C_E);
  i2cMasterTransmitTimeout(&I2CD1,LCD_I2C_ADDR,&pin_mapped_value,1,NULL,0,timeout);

  i2cReleaseBus(&I2CD1);
  chThdSleepMicroseconds(1);
}


void LCD_init(void) {

  palSetPadMode(GPIOB, 6, PAL_MODE_STM32_ALTERNATE_OPENDRAIN); /* SCL */
  palSetPadMode(GPIOB, 7, PAL_MODE_STM32_ALTERNATE_OPENDRAIN); /* SDA */

  i2cStart(&I2CD1, &i2cfg);
  uint8_t tmp = 0;

  i2cAcquireBus(&I2CD1);
  i2cMasterTransmitTimeout(&I2CD1,LCD_I2C_ADDR,&tmp,1,NULL,0,timeout);
  i2cReleaseBus(&I2CD1);

  //I2C_transmit_4bit(0);
  chThdSleepMilliseconds(100);
  I2C_transmit_4bit(LCD_FUNCTIONSET_DATALENGTH_8BIT>>4);

  chThdSleepMilliseconds(50);
  I2C_transmit_4bit(LCD_FUNCTIONSET_DATALENGTH_8BIT>>4);

  chThdSleepMicroseconds(120);
  I2C_transmit_4bit(LCD_FUNCTIONSET_DATALENGTH_8BIT>>4);
  I2C_transmit_4bit(LCD_FUNCTIONSET_DATALENGTH_4BIT>>4);
  I2C_transmit_4bit(LCD_FUNCTIONSET_DATALENGTH_4BIT>>4);
  I2C_transmit_4bit(LCD_OPERATION_DISPLAY_OFF);
  LCD_cmd(LCD_FUNCTIONSET_2LINES|LCD_FUNCTIONSET_5X8FONT);
  LCD_cmd(LCD_RETURNHOME);
  LCD_cmd(LCD_OPERATION_DISPLAY_ON|LCD_OPERATION_CURSOR_ON);

/*  LCD_cmd(LCD_CLEARDISPLAY);
  LCD_cmd(LCD_ENTRYMODE_RIGHT);
  LCD_transmit(0, LCD_PIN_BL);
  LCD_transmit(LCD_RETURNHOME, LCD_PIN_BL);*/
  LCD_transmit('a', LCD_PIN_RS|LCD_PIN_BL);
  LCD_transmit('b', LCD_PIN_RS|LCD_PIN_BL);

//  LCD_cmd(LCD_FUNCTIONSET_2LINES|LCD_FUNCTIONSET_5X8FONT);
//  chThdSleepMilliseconds(5);
//
//
//  LCD_cmd(LCD_OPERATION_CURSOR_ON);
//  LCD_cmd(LCD_OPERATION_CURSORBLINK_ON);
//  LCD_cmd(LCD_RETURNHOME);

}


// Sends command not checking BF flag.
void LCD_transmit(uint8_t value, uint8_t control) { //value [D7..D0]
  I2C_transmit_4bit((value >> 4)|control);
  I2C_transmit_4bit((value & 0x0F)|control);
  }

void LCD_cmd(uint8_t value) {
  //TODO:add busy flag check here
  LCD_transmit(value,0);
}
