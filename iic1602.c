/*
 * iic1602.c
 *
 *  Created on: 09 мая 2017 г.
 *      Author: alpha-user
 */

#include "iic1602.h"
uint8_t backlight = 1;



static const I2CConfig i2cfg = {
    OPMODE_I2C,
    100000,
    FAST_DUTY_CYCLE_2,
};

void I2C_set_pins(uint8_t *value, uint8_t length) {
  systime_t timeout = MS2ST(4);

  i2cAcquireBus(&I2CD1);
  i2cMasterTransmitTimeout(&I2CD1,LCD_I2C_ADDR,value,length,NULL,0,timeout);
  i2cReleaseBus(&I2CD1);
  chThdSleepMicroseconds(1);
}

void I2C_get_pins(uint8_t *value) {
  systime_t timeout = MS2ST(4);
  uint8_t command[2] = { (1<<LCD_I2C_RW)|(backlight<<LCD_I2C_BL)|(1<<LCD_I2C_E),
                         (1<<LCD_I2C_RW)|(backlight<<LCD_I2C_BL) };

  i2cAcquireBus(&I2CD1);
  i2cMasterTransmitTimeout(&I2CD1,LCD_I2C_ADDR,command,2,value,2,timeout);
  i2cReleaseBus(&I2CD1);
}

void I2C_transmit_4bit(uint8_t value) {
  uint8_t pin_mapped_value[2] = {0, 0};
  pin_mapped_value[0] |= (((value>>0) & 1)<<LCD_I2C_D4);
  pin_mapped_value[0] |= (((value>>1) & 1)<<LCD_I2C_D5);
  pin_mapped_value[0] |= (((value>>2) & 1)<<LCD_I2C_D6);
  pin_mapped_value[0] |= (((value>>3) & 1)<<LCD_I2C_D7);

//  pin_mapped_value[0] |= (((value>>5) & 1)<<LCD_I2C_BL);
  pin_mapped_value[0] |= (backlight << LCD_I2C_BL);
  pin_mapped_value[0] |= (((value>>6) & 1)<<LCD_I2C_RS);
  pin_mapped_value[0] |= (((value>>7) & 1)<<LCD_I2C_RW);

  pin_mapped_value[1] = pin_mapped_value[0];
  pin_mapped_value[0] |= (1<<LCD_I2C_E);

  I2C_set_pins(pin_mapped_value, 2);

}

void LCD_clear() {
  LCD_cmd(LCD_CLEARDISPLAY);
  chThdSleepMilliseconds(2);
}

void LCD_home() {
  LCD_cmd(LCD_RETURNHOME);
  chThdSleepMilliseconds(2);
}

uint8_t LCD_get_bf() {
  uint8_t read_value[2] = {0,0};
  I2C_get_pins(read_value);
  return read_value[0] & LCD_BUSY_FLAG_MASK;
}

void LCD_init(void) {

  palSetPadMode(GPIOB, 6, PAL_MODE_STM32_ALTERNATE_OPENDRAIN); /* SCL */
  palSetPadMode(GPIOB, 7, PAL_MODE_STM32_ALTERNATE_OPENDRAIN); /* SDA */

  i2cStart(&I2CD1, &i2cfg);
  I2C_set_pins(0,1);
  chThdSleepMilliseconds(20);

  I2C_transmit_4bit(LCD_FUNCTIONSET_DATALENGTH_8BIT>>4);
  chThdSleepMilliseconds(5);

  I2C_transmit_4bit(LCD_FUNCTIONSET_DATALENGTH_8BIT>>4);
  chThdSleepMicroseconds(120);
  
  I2C_transmit_4bit(LCD_FUNCTIONSET_DATALENGTH_8BIT>>4);
  I2C_transmit_4bit(LCD_FUNCTIONSET_DATALENGTH_4BIT>>4);
  I2C_transmit_4bit(LCD_FUNCTIONSET_DATALENGTH_4BIT>>4);
  I2C_transmit_4bit(LCD_OPERATION_DISPLAY_OFF);
  
  LCD_clear();
  LCD_cmd(LCD_FUNCTIONSET_2LINES|LCD_FUNCTIONSET_5X8FONT);

  LCD_home();
  LCD_cmd(LCD_OPERATION_DISPLAY_ON|
          LCD_OPERATION_CURSOR_ON|
          LCD_OPERATION_CURSORBLINK_ON);

  LCD_transmit('a', LCD_PIN_RS|LCD_PIN_BL);
  LCD_transmit('b', LCD_PIN_RS|LCD_PIN_BL);





}


// Sends command not checking BF flag.
void LCD_transmit(uint8_t value, uint8_t control) { //value [D7..D0]
  I2C_transmit_4bit((value >> 4)|control);
  I2C_transmit_4bit((value & 0x0F)|control);
  }

void LCD_cmd(uint8_t value) {
//  int i=0;
//  while (LCD_get_bf())
//  {
//    i++;
//    chThdSleepMicroseconds(100);
//  }
//  i++;
  //LCD_get_bf();
  LCD_transmit(value,0);
}
