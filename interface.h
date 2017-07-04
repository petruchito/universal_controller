/*
 * interface.h
 *
 *  Created on: 26 θώνÿ 2017 γ.
 *      Author: alpha-user
 */

#ifndef INTERFACE_H_
#define INTERFACE_H_

#define SCR_ADJUST 0
#define SCR_HEATING 1
#define SCR_MENU 2
#define MAX_INTERFACE_DEPTH 3
typedef struct SystemState sys_state_t;

typedef enum {
  SHOW_TEMPERATURE=0b1,
  SHOW_PWM=0b10,
  SHOW_SET_VALUE=0b100
} interface_features_t;

typedef enum {
  OVERVIEW,
  MENU_INTERFACE
  } interface_item_type_t;

typedef struct {
  interface_features_t features;
  char *text;
} menu_item_t;

typedef struct {
  uint8_t redraw;
  uint8_t selected;
  uint16_t encoder_cnt,encoder_arr;
  interface_item_type_t type;
  void (*RenderText)(sys_state_t *state, uint8_t i);
  void (*OnPress)(sys_state_t *state);
  void (*OnLongPress)(sys_state_t *state);
  void (*OnEncoder)(sys_state_t *state);
  menu_item_t items[];
} interface_t;

uint8_t RenderInterface(sys_state_t *state);

void RenderSettingText(sys_state_t *state, uint8_t i);
void HeatingScreen(sys_state_t *state);
void AdjustScreen(sys_state_t *state);
void SettingsScreen(sys_state_t *state);
void ReturnFromSettingsScreen(sys_state_t *state);
void SettingsOnEncoder(sys_state_t *state);
void AdjustOnEncoder(sys_state_t *state);

#endif /* INTERFACE_H_ */
