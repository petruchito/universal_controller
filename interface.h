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
extern sys_state_t system_state;

typedef enum {
  SHOW_TEMPERATURE=0b1,
  SHOW_PWM=0b10,
  SHOW_SET_VALUE=0b100
} interface_features_t;

typedef struct {
  interface_features_t features;
  void (*RenderItem)(sys_state_t *state, uint8_t item_number, uint8_t line);
  void (*OnPress)(sys_state_t *state);
  void (*OnLongPress)(sys_state_t *state);
  void (*OnEncoder)(sys_state_t *state);
  void *parameter;
  char *parameter_template;
  char *text;
} menu_item_t;

typedef struct {
  void *parameter;
  void *max;
  void *min;
} parameter_struct_t;

typedef struct {
  uint8_t redraw;
  uint8_t selected;
  parameter_struct_t parameter_settings;
  uint16_t encoder_cnt,encoder_arr, items_count;
  void (*Render)(sys_state_t *state, uint8_t item_number, uint8_t line);
  void (*OnLoad)(sys_state_t *state);
  void (*OnUnload)(sys_state_t *state);
  void (*OnPress)(sys_state_t *state);
  void (*OnLongPress)(sys_state_t *state);
  void (*OnEncoder)(sys_state_t *state);
  menu_item_t items[];
} interface_t;

/*
 *
 *  Heating screen functions
 *
 */
void HeatingScreen(sys_state_t *state);
void HeatingScreenOnUnload(sys_state_t *state);
void RenderFeatures(sys_state_t* state, uint8_t item_number, uint8_t line);

/*
 *
 * Adjust screen functions
 *
 */
void AdjustScreen(sys_state_t *state);
void AdjustOnEncoder(sys_state_t *state);
void AdjustScreenOnLoad(sys_state_t *state);

/*
 *
 * Settings screen functions
 *
 */
void SettingsScreen(sys_state_t *state);
void SettingsOnEncoder(sys_state_t *state);
void SettingsScreenOnLoad(sys_state_t *state);
void RenderMenuItemText(sys_state_t *state, uint8_t item_number, uint8_t line);
void RenderMenuItemFloat(sys_state_t *state, uint8_t item_number, uint8_t line);

/*
 *
 * Adjust float functions
 *
 */
void AdjustFloat (sys_state_t *state);
void AdjustFloatScreenOnEncoder(sys_state_t *state);
void AdjustFloatScreenOnLoad(sys_state_t *state);
void RenderAdjustFloatScreen(sys_state_t *state, uint8_t item_number, uint8_t line);

/*
 *
 * Generic functions
 *
 */
void EnterSubInterface(sys_state_t *state, interface_t *new_interface);
void ExitSubInterface(sys_state_t *state);
void RenderInterface(sys_state_t *state);

#endif /* INTERFACE_H_ */
