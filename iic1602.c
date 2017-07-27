/*
 * iic1602.c
 *
 *  Created on: 09 мая 2017 г.
 *      Author: alpha-user
 */
#include <stdlib.h>
#include <string.h>
#include "iic1602.h"

uint8_t backlight = 1;

static const I2CConfig i2cfg = {
    OPMODE_I2C,
    100000,
    STD_DUTY_CYCLE
};

uint8_t I2C_map_pins(uint8_t value) {

//   internal format: [7:RW] [6:RS] [5:NOT USED] [4:E]
//                    [3:D7/D3] [2:D6/D2] [1:D5/D1] [0:D4/D0]
//   to physical connections of PCF8574

  uint8_t mapped = 0;
  mapped |= (((value>>0) & 1) << LCD_I2C_D4);
  mapped |= (((value>>1) & 1) << LCD_I2C_D5);
  mapped |= (((value>>2) & 1) << LCD_I2C_D6);
  mapped |= (((value>>3) & 1) << LCD_I2C_D7);
  mapped |= (((value>>4) & 1) << LCD_I2C_E);
  mapped |= (backlight << LCD_I2C_BL);
  mapped |= (((value>>6) & 1) << LCD_I2C_RS);
  mapped |= (((value>>7) & 1) << LCD_I2C_RW);
  return mapped;
}

uint8_t I2C_unmap_pins(uint8_t value) {
  uint8_t unmapped = 0;
  unmapped |= (((value>>LCD_I2C_RW) & 1) << _LCD_RW);
  unmapped |= (((value>>LCD_I2C_RS) & 1) << _LCD_RS);
  unmapped |= (((value>>LCD_I2C_E ) & 1) << _LCD_E);
  unmapped |= (((value>>LCD_I2C_D4) & 1) << _LCD_D4_0);
  unmapped |= (((value>>LCD_I2C_D5) & 1) << _LCD_D5_1);
  unmapped |= (((value>>LCD_I2C_D6) & 1) << _LCD_D6_2);
  unmapped |= (((value>>LCD_I2C_D7) & 1) << _LCD_D7_3);
  return unmapped;
}


void I2C_set_pins(uint8_t *value, uint8_t length) {
  systime_t timeout = MS2ST(40);
  i2cAcquireBus(&I2CD1);
  i2cMasterTransmitTimeout(&I2CD1,LCD_I2C_ADDR,value,length,NULL,0,timeout);
  i2cReleaseBus(&I2CD1);
  chThdSleepMicroseconds(50);
}

 /*
  uint8_t I2C_read_bf_ac() {
  systime_t timeout = MS2ST(4);
  uint8_t rxbuffer[4] = {0,0,0,0};

  uint8_t command[3] = { I2C_map_pins(LCD_PIN_RW|LCD_PIN_E|0xF),
                         I2C_map_pins(LCD_PIN_RW),
                         I2C_map_pins(0)};

  i2cAcquireBus(&I2CD1);

  i2cMasterTransmitTimeout(&I2CD1,LCD_I2C_ADDR,command,1,NULL,0,timeout);
  i2cMasterReceiveTimeout(&I2CD1, LCD_I2C_ADDR, rxbuffer, 2, timeout);
  i2cMasterTransmitTimeout(&I2CD1,LCD_I2C_ADDR,command+1,1,NULL,0,timeout);

  i2cMasterTransmitTimeout(&I2CD1,LCD_I2C_ADDR,command,1,NULL,0,timeout);
  i2cMasterReceive(&I2CD1, LCD_I2C_ADDR, rxbuffer+2, 2);
  i2cMasterTransmitTimeout(&I2CD1,LCD_I2C_ADDR,command+2,1,NULL,0,timeout);
  //i2cMasterTransmitTimeout(&I2CD1,LCD_I2C_ADDR,command,2,NULL,0,timeout);
  i2cReleaseBus(&I2CD1);
  chThdSleepMicroseconds(1);
  rxbuffer[1] = I2C_unmap_pins(rxbuffer[1]);
  rxbuffer[3] = I2C_unmap_pins(rxbuffer[3]);
  rxbuffer[0] = ((rxbuffer[1]<<4)|(rxbuffer[3] & 0xF));
  return ((rxbuffer[1]<<4)|(rxbuffer[3] & 0xF));
}
*/

void I2C_transmit_4bit(uint8_t value) {
  uint8_t pin_mapped_value[2] = {I2C_map_pins(value|LCD_PIN_E),
                                 I2C_map_pins(value)};
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

void LCD_shift_cmd(int8_t value, uint8_t command_l, uint8_t command_r) {
  uint8_t displacement = abs(value);
  uint8_t command;
  if (value > 0) {
    command = command_r;
  } else {
    command = command_l;
  }

  uint8_t i;
  for (i=1;i<displacement;i++) {
    LCD_cmd(command);
  }
}

void LCD_shift_cursor(int8_t value) {
  LCD_shift_cmd(value, LCD_SHIFT_CURSOR_LEFT, LCD_SHIFT_CURSOR_RIGHT);
}
void LCD_shift_display(int8_t value) {
  LCD_shift_cmd(value, LCD_SHIFT_DISPLAY_LEFT, LCD_SHIFT_DISPLAY_RIGHT);
}


void LCD_char(char value) {
  LCD_transmit(value, LCD_PIN_RS);
}

void LCD_string(char* value) {
  size_t length = strlen(value);
  uint8_t i;
  for(i=0;i<length;i++) {
    LCD_char(value[i]);
  }
}

void LCD_goto(uint8_t row, uint8_t column) {
  uint8_t position = column;
  row++;
  if (row == 2) position += 0x40;
  position &= LCD_DDRAM_ADDR_MASK;
  LCD_cmd(LCD_DDRAM_CMD | position);
}

void LCD_test() {
  LCD_string("Testing...");
  chThdSleepMilliseconds(1000);
  LCD_string("Clear...");
  chThdSleepMilliseconds(1000);
  LCD_clear();
  int i;
  for (i = 0; i < 0x28; i++) {
    LCD_transmit('a' + i, LCD_PIN_RS);
    if (i % 2)
      chThdSleepMilliseconds(100);
  }
  for (i = 0x40; i < 0x68; i++) {
      LCD_transmit('A' + i - 0x40, LCD_PIN_RS);
      if (i % 2)
        chThdSleepMilliseconds(100);
  }
  chThdSleepMilliseconds(1000);
  LCD_shift_display(25);
  chThdSleepMilliseconds(500);
  LCD_shift_display(-25);
  chThdSleepMilliseconds(500);
  LCD_shift_cursor(5);
  chThdSleepMilliseconds(500);
  LCD_goto(2, 5);
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
  LCD_cmd(LCD_OPERATION_DISPLAY_ON);
  //LCD_test();
}


void LCD_transmit(uint8_t value, uint8_t control) { //value [D7..D0]
  I2C_transmit_4bit((value >> 4)|control);
  I2C_transmit_4bit((value & 0x0F)|control);
}

void LCD_cmd(uint8_t value) {
  LCD_transmit(value,0);
}

