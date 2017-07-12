# Universal controller

## WORK IN PROGRESS

## Description

Universal PID controller, triac driving with zero-crossing optotriac.

Based on STM32F103C8T6 aka "BluePill" board.

Pin connections are in the *pid.ioc* file (STM32CubeMX format).

Project is based on "ChibiOS blink with stable 16.1.5" (c)InvalidExcepti0n boilerplate.

## what's working and what's not:

- [x] MAX6675 thermocouple polling 
- [x] quadrature encoder
- [x] ywrobot lcm1602 iic v1 display
- [x] interface and menu 
- [ ] EEPROM settings storage
- [ ] PWM output
- [ ] direct PWM adjustment without regulation
- [ ] PID algorithm
- [ ] PID settings in menu
- [ ] PID coefficients autotuning (probably)
