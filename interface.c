/*
 * interface.c
 *
 *  Created on: 26 θώνÿ 2017 γ.
 *      Author: alpha-user
 */

#include <string.h>
#include <stdio.h>

#include "hal.h"
#include "sysstate.h"
#include "interface.h"
#include "iic1602.h"

//static interface_t *current_interface, *previous_interface;

static interface_t adjust_screen = {
                                    .type = OVERVIEW,
                                    .OnPress = HeatingScreen,
                                    .OnLongPress = SettingsScreen,
                                    .items = {
                                              {.features = SHOW_TEMPERATURE},
                                              {.text = "PRESS TO START"}
                                    }
};

static interface_t heating_screen = {
                                     .type = OVERVIEW,
                                     .OnPress = AdjustScreen,
                                     .OnLongPress = SettingsScreen,
                                     .items = {
                                               {.features = SHOW_TEMPERATURE|SHOW_SET_VALUE},
                                               {.features = SHOW_PWM,.text = "HEATING"}
                                     }
};

static interface_t settings_screen = {
                                      .type = MENU_INTERFACE,
                                      .OnLongPress = ReturnFromSettingsScreen,
                                      .OnEncoder = SettingsOnEncoder,
                                      .encoder_arr = 3, //menu length - 1
                                      .items = {
                                                {.text = "MENU1"},
                                                {.text = "MENU2"},
                                                {.text = "MENU3"},
                                                {.text = "MENU4"}
                                      }
};

void HeatingScreen(sys_state_t *state) {
  state->interface = &heating_screen;
}

void AdjustScreen(sys_state_t *state) {
  state->interface = &adjust_screen;
}

void SettingsScreen(sys_state_t *state) {
  state->updated |= UPD_ARR;
  state->return_interface = state->interface;
  state->interface = &settings_screen;

}

void ReturnFromSettingsScreen(sys_state_t *state) {
  state->interface->selected = 0;
  state->interface = state->return_interface;
  state->return_interface = 0;
}

void SettingsOnEncoder(sys_state_t *state) {
      state->interface->selected = state->encoder_cnt;
}


uint8_t RenderInterface(sys_state_t *state)  {

  uint8_t redraw = 0;

  if (!state->interface) {
    state->interface = &adjust_screen;
    redraw = TRUE;
  }
//  determine key presses
//  selected_item++ or change current screen

  if (state->updated & UPD_BUTTON && state->encoder_button == BTN_DOWN) {
    if(state->interface->OnPress) state->interface->OnPress(state);
    redraw = TRUE;
  }

  if (state->updated & UPD_BUTTON && state->encoder_button == BTN_LONGPRESS) {
    if(state->interface->OnLongPress) state->interface->OnLongPress(state);
    redraw = TRUE;
  }

  if (state->updated & UPD_ENCODER) {
    if (state->interface->OnEncoder) {
      state->interface->OnEncoder(state);
      redraw = TRUE;
    }

  }

//  draw interface
  if (redraw) LCD_clear();

  uint8_t i;
  for (i=0;i<LCD_ROW_SIZE;i++) {
    if (state->interface->items[state->interface->selected+i].text
        && ( redraw || (state->updated & UPD_ENCODER) )) {
      LCD_goto(i+1,0);
      LCD_string(state->interface->items[state->interface->selected+i].text);
    }

    if (state->interface->items[state->interface->selected+i].features & SHOW_TEMPERATURE
        && ( redraw || (state->updated & UPD_TEMPERATURE) )) {
      LCD_goto(i+1,7);
      char tempstr[10];
      sprintf(tempstr, "%7.2f\337C", state->temperature);
      LCD_string(tempstr);
    }

    if (state->interface->items[state->interface->selected+i].features & SHOW_SET_VALUE
        && ( redraw || (state->updated & UPD_SET_TEMPERATURE) )) {
      //TODO: PWM MODE
      LCD_goto(i+1,0);
      char tempstr[6];
      sprintf(tempstr, "%3d\337C", state->set_temperature);
      LCD_string(tempstr);
    }
    if (state->interface->items[state->interface->selected+i].features & SHOW_PWM
            && ( redraw || (state->updated & UPD_PWM) )) {
          LCD_goto(i+1,12);
          char tempstr[5];
          sprintf(tempstr, "%3d%%", state->pwm);
          LCD_string(tempstr);
    }
  } //for(;i<LCD_ROW_SIZE...
  redraw = FALSE;
  return 0;
}
