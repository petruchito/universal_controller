/*
 * iic1602.h
 *
 *  Created on: 09 мая 2017 г.
 *      Author: alpha-user
 */



#ifndef IIC1602_H_
#define IIC1602_H_
#include "ch.h"
#include "hal.h"

/*
 *
 * P0=RS
 * P1=RW
 * P2=E
 *
 * P3=BL
 *
 * P4=D4
 * P5=D5
 * P6=D6
 * P7=D7
 *
 */

#define LCD_I2C_RS 0
#define LCD_I2C_RW 1
#define LCD_I2C_E  2
#define LCD_I2C_BL 3
#define LCD_I2C_D4 4
#define LCD_I2C_D5 5
#define LCD_I2C_D6 6
#define LCD_I2C_D7 7


#define LCD_PIN_BL (1<<5)
#define LCD_PIN_RS (1<<6)
#define LCD_PIN_RW (1<<7)

#define LCD_I2C_ADDR 0x27


#define LCD_CLEARDISPLAY            0b0001


#define LCD_RETURNHOME              0b0010


#define LCD_ENTRYMODE_RIGHT             0b110
#define LCD_ENTRYMODE_LEFT              0b100

#define LCD_ENTRYMODE_SHIFT_DISPLAY     0b101
#define LCD_ENTRYMODE_SHIFT_CURSOR      0b100


#define LCD_OPERATION_DISPLAY_ON        0b1100
#define LCD_OPERATION_DISPLAY_OFF       0b1000

#define LCD_OPERATION_CURSOR_ON         0b1010
#define LCD_OPERATION_CURSOR_OFF        0b1000

#define LCD_OPERATION_CURSORBLINK_ON    0b1001
#define LCD_OPERATION_CURSORBLINK_OFF   0b1000


#define LCD_SHIFT_CURSOR_LEFT           0b10000
#define LCD_SHIFT_CURSOR_RIGHT          0b11000

#define LCD_SHIFT_DISPLAY_LEFT          0b10000
#define LCD_SHIFT_DISPLAY_RIGHT         0b10100


#define LCD_FUNCTIONSET_DATALENGTH_8BIT 0b110000
#define LCD_FUNCTIONSET_DATALENGTH_4BIT 0b100000

#define LCD_FUNCTIONSET_1LINE           0b100000
#define LCD_FUNCTIONSET_2LINES          0b101000

#define LCD_FUNCTIONSET_5X8FONT         0b100000
#define LCD_FUNCTIONSET_5X11FONT        0b100100

#define LCD_CGRAM_CMD                   0b1000000
#define LCD_CGRAM_ADDR_MASK             0b111111
#define LCD_DDRAM_CMD                   0b10000000
#define LCD_DDRAM_ADDR_MASK             0b1111111

#define LCD_BUSY_FLAG_MASK              0b10000000

#define LCD_ADDR_MASK                   0b1111111



void I2C_get_pins(uint8_t *value);
void I2C_set_pins(uint8_t *value, uint8_t length);
void I2C_transmit_4bit(uint8_t value);

uint8_t LCD_get_bf(void);

void LCD_transmit(uint8_t value, uint8_t control);

void LCD_init(void);

void LCD_cmd(uint8_t value);

void LCD_clear(void);

void LCD_home(void);

uint8_t LCD_read(void);

#endif /* IIC1602_H_ */
