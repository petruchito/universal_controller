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
 * Physical connections:
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

//
//   internal format: [7:RW] [6:RS] [5:NOT USED] [4:E]
//                    [3:D7/D3] [2:D6/D2] [1:D5/D1] [0:D4/D0]

#define _LCD_D4_0 0
#define _LCD_D5_1 1
#define _LCD_D6_2 2
#define _LCD_D7_3 3
#define _LCD_E    4
#define _LCD_RS   6
#define _LCD_RW   7

#define LCD_PIN_D4_0 (1<<0)
#define LCD_PIN_D5_1 (1<<1)
#define LCD_PIN_D6_2 (1<<2)
#define LCD_PIN_D7_3 (1<<3)
#define LCD_PIN_E  (1<<4)
#define LCD_PIN_RS (1<<6)
#define LCD_PIN_RW (1<<7)

//
// IIC address
//

#define LCD_I2C_ADDR 0x27


#define LCD_STRING_SIZE 16
#define LCD_ROW_SIZE    2


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
#define LCD_SHIFT_CURSOR_RIGHT          0b10100

#define LCD_SHIFT_DISPLAY_LEFT          0b11000
#define LCD_SHIFT_DISPLAY_RIGHT         0b11100


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

uint8_t I2C_map_pins(uint8_t value);
//uint8_t I2C_read_bf_ac(void);
void I2C_set_pins(uint8_t *value, uint8_t length);
void I2C_transmit_4bit(uint8_t value);


void LCD_transmit(uint8_t value, uint8_t control);

void LCD_init(void);

void LCD_cmd(uint8_t value);

void LCD_test(void);

void LCD_shift_cmd(int8_t value, uint8_t command_l, uint8_t command_r);

void LCD_shift_cursor(int8_t value);

void LCD_shift_display(int8_t value);

void LCD_clear(void);

void LCD_home(void);

void LCD_char(char value);

void LCD_goto(uint8_t x, uint8_t y);

uint8_t LCD_read(void);

void LCD_string(char* value);

#endif /* IIC1602_H_ */
