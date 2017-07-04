/*
 * interface.c
 *
 *  Created on: 26 ���� 2017 �.
 *      Author: alpha-user
 */

#include <string.h>
#include <stdio.h>

#include "hal.h"
#include "sysstate.h"
#include "interface.h"
#include "iic1602.h"

static interface_t adjust_screen = {
                                    .type = OVERVIEW,
                                    .OnPress = HeatingScreen,
                                    .OnLongPress = SettingsScreen,
                                    .encoder_arr = 50,
                                    .OnEncoder = AdjustOnEncoder,
                                    .items = {
                                              {.features = SHOW_TEMPERATURE|SHOW_SET_VALUE},
                                              {.text = " PRESS TO START"}
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
                                      .RenderText = RenderSettingText,
                                      .encoder_arr = 3, //menu length - 1
                                      .encoder_cnt = 0,
                                      .items = {
                                                {.text = "MENU1"},
                                                {.text = "MENU2"},
                                                {.text = "MENU3"},
                                                {.text = "MENU4"}
                                      }
};
void RenderSettingText(sys_state_t *state, uint8_t line) {
  char *mark = " ";
  if (!line) mark = ">";
  LCD_goto(line+1,0);
  LCD_string(mark);
  LCD_string(state->interface->items[state->interface->selected+line].text);

  if (line == 0
      && state->interface->selected == state->interface->encoder_arr) {
    LCD_goto(2,0);
    LCD_string(" ");
    LCD_string(state->interface->items[0].text);
  }
}

void HeatingScreen(sys_state_t *state) {
  state->interface = &heating_screen;
  state->interface->redraw = TRUE;
}

void AdjustScreen(sys_state_t *state) {
  state->interface = &adjust_screen;
  state->interface->redraw = TRUE;
  EncoderSetup(state->interface->encoder_arr,state->set_temperature/10);

}

void SettingsScreen(sys_state_t *state) {
  state->return_interface[state->return_interfaces_top++] = state->interface;
  state->interface = &settings_screen;
  state->interface->redraw = TRUE;
  EncoderSetup(state->interface->encoder_arr,state->interface->encoder_cnt);

}

void ReturnFromSettingsScreen(sys_state_t *state) {
  state->interface->selected = 0;
  state->interface = state->return_interface[--state->return_interfaces_top];
  state->return_interface[state->return_interfaces_top] = 0;
  state->interface->redraw = TRUE;
  if (state->interface == &adjust_screen) {
    EncoderSetup(state->interface->encoder_arr,state->set_temperature/10);

  }

}

void SettingsOnEncoder(sys_state_t *state) {
  state->interface->selected = state->encoder_cnt;
}

void AdjustOnEncoder(sys_state_t *state) {
  state->set_temperature = state->encoder_cnt*10;
  state->updated |= UPD_SET_TEMPERATURE;
}

uint8_t RenderInterface(sys_state_t *state)  {


  if (!state->interface) {
    AdjustScreen(state);
  }

  //  determine key presses

  if (state->updated & UPD_BUTTON && state->encoder_button == BTN_DOWN) {
    if(state->interface->OnPress) state->interface->OnPress(state);
  }

  if (state->updated & UPD_BUTTON && state->encoder_button == BTN_LONGPRESS) {
    if(state->interface->OnLongPress) state->interface->OnLongPress(state);
  }

  if (state->updated & UPD_ENCODER) {
    if (state->interface->OnEncoder) {
      state->interface->OnEncoder(state);
    }

  }

  //  draw interface
  if (state->interface->redraw) LCD_clear();

  uint8_t i;
  for (i=0;i<LCD_ROW_SIZE;i++) {
    if (state->interface->items[state->interface->selected+i].text
        && ( state->interface->redraw || (state->updated & UPD_ENCODER) )) {
      if (state->interface->RenderText) {
        RenderSettingText(state, i);
      } else {
        LCD_goto(i+1,0);
        LCD_string(state->interface->items[state->interface->selected+i].text);
      }
    }

    if (state->interface->items[state->interface->selected+i].features & SHOW_TEMPERATURE
        && ( state->interface->redraw || (state->updated & UPD_TEMPERATURE) )) {
      LCD_goto(i+1,7);
      char tempstr[10];
      sprintf(tempstr, "%7.2f\337C", state->temperature);
      LCD_string(tempstr);
    }

    if (state->interface->items[state->interface->selected+i].features & SHOW_SET_VALUE
        && ( state->interface->redraw || (state->updated & UPD_SET_TEMPERATURE) )) {
      //TODO: PWM MODE
      LCD_goto(i+1,0);
      char tempstr[6];
      sprintf(tempstr, "%3d\337C", state->set_temperature);
      LCD_string(tempstr);
    }

    if (state->interface->items[state->interface->selected+i].features & SHOW_PWM
        && ( state->interface->redraw || (state->updated & UPD_PWM) )) {
      LCD_goto(i+1,12);
      char tempstr[5];
      sprintf(tempstr, "%3d%%", state->pwm);
      LCD_string(tempstr);
    }
  } //for(;i<LCD_ROW_SIZE...
  state->interface->redraw = FALSE;
  return 0;
}