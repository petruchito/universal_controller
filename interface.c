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

static interface_t adjust_screen = {
                                    .type = OVERVIEW,
                                    .OnPress = HeatingScreen,
                                    .OnLongPress = SettingsScreen,
                                    .OnLoad = AdjustScreenOnLoad,
                                    .encoder_arr = 50,
                                    .Render = RenderFeatures,
                                    .OnEncoder = AdjustOnEncoder,
                                    .items = {
                                              {.features = SHOW_TEMPERATURE|SHOW_SET_VALUE},
                                              {.text = " PRESS TO START"}
                                    }
};

static interface_t heating_screen = {
                                     .type = OVERVIEW,
                                     .OnPress = AdjustScreen,
                                     .Render = RenderFeatures,
                                     .OnLongPress = SettingsScreen,
                                     .items = {
                                               {.features = SHOW_TEMPERATURE|SHOW_SET_VALUE},
                                               {.features = SHOW_PWM,.text = "HEATING"}
                                     }
};

static interface_t settings_screen = {
                                      .type = MENU_INTERFACE,
                                      .OnLoad = SettingsScreenOnLoad,
                                      .OnLongPress = ReturnFromSettingsScreen,
                                      .OnEncoder = SettingsOnEncoder,
                                      .Render = RenderMenuItems,
                                      .encoder_arr = 3, //[menu length - 1]
                                      .encoder_cnt = 0,
                                      .items = {
                                                {.text = "float1.000-0.000",
                                                 .OnPress = AdjustFloat},
                                                {.text = "MENU2"},
                                                {.text = "MENU3"},
                                                {.text = "MENU4"}
                                      }
};

static interface_t adjust_float_screen = {
                                          .OnLongPress = ReturnToMenu,
                                          .Render = RenderAdjustFloatScreen,
                                          .OnLoad = AdjustFloatScreenOnLoad,
                                          .OnEncoder = AdjustFloatScreenOnEncoder,
                                          .encoder_arr = 100,
                                          .items = {{.text = "Float:",
                                                     .OnLongPress = 0}}
};

void AdjustFloatScreenOnEncoder(sys_state_t *state) {
  *(float *)state->interface->parameter_settings.parameter =
      ((float)state->encoder_cnt/100);
}

void AdjustFloatScreenOnLoad(sys_state_t *state){
  state->interface->parameter_settings.max = &(float){0.0f};
  state->interface->parameter_settings.max = &(float){1.0f};
  state->interface->parameter_settings.parameter = &state->f_param;
  EncoderSetup(state->interface->encoder_arr,
               (uint16_t)((*(float *)state->interface->parameter_settings.parameter)*100));
}

void AdjustFloat (sys_state_t *state) {
  EnterSubInterface(state, &adjust_float_screen);
}

void RenderAdjustFloatScreen(sys_state_t *state, uint8_t line) {

  if (!line) {
    LCD_goto(1,0);
    LCD_string(state->interface->items[state->interface->selected].text);
  } else {
    LCD_goto(2,0);
    char tempstr[10];
    sprintf(tempstr, "<%7.2f>", *((float *)state->interface->parameter_settings.parameter) );
    LCD_string(tempstr);
  }
}

void RenderMenuItems(sys_state_t *state, uint8_t line) {
  char mark = ' ';
  if (!line) mark = '>';
  LCD_goto(line+1,0);

  uint8_t item_number = state->interface->selected+line;
  if (item_number>state->interface->encoder_arr
      && state->interface->encoder_arr > 0) item_number = 0;
  if (state->interface->items[item_number].text) {
    char tempstr[16];
    sprintf(tempstr, "%c%-15s", mark,
            state->interface->items[item_number].text);
    LCD_string(tempstr);
  }
}

void HeatingScreen(sys_state_t *state) {
  state->interface = &heating_screen;
  state->interface->redraw = TRUE;
}

void AdjustScreen(sys_state_t *state) {
  state->interface = &adjust_screen;
  state->interface->OnLoad(state);
  state->interface->redraw = TRUE;
}

void AdjustScreenOnLoad(sys_state_t *state) {
  EncoderSetup(state->interface->encoder_arr,state->set_temperature/10);
}

void SettingsScreenOnLoad(sys_state_t *state) {
  EncoderSetup(state->interface->encoder_arr,state->interface->encoder_cnt);
}

void MenuInterfaceOnLoad(sys_state_t *state) {
  EncoderSetup(state->interface->encoder_arr, state->interface->selected);
}

void EnterSubInterface(sys_state_t *state, interface_t *new_interface) {
  state->return_interface[state->return_interfaces_top++] = state->interface;
    state->interface = new_interface;
    if (state->interface->OnLoad) state->interface->OnLoad(state);
    state->interface->redraw = TRUE;
}

void ExitSubInterface(sys_state_t *state) {
  state->interface->selected = 0;
  state->interface = state->return_interface[--state->return_interfaces_top];
  state->return_interface[state->return_interfaces_top] = 0;
  if (state->interface->OnLoad) state->interface->OnLoad(state);
  state->interface->redraw = TRUE;
}

void SettingsScreen(sys_state_t *state) {
  EnterSubInterface(state,&settings_screen);
  //EncoderSetup(state->interface->encoder_arr,state->interface->encoder_cnt);
}

void ReturnToMenu(sys_state_t *state) {
  ExitSubInterface(state);
//  EncoderSetup(state->interface->encoder_arr, state->interface->selected);
}


void ReturnFromSettingsScreen(sys_state_t *state) {
  ExitSubInterface(state);
}

void SettingsOnEncoder(sys_state_t *state) {
  state->interface->selected = state->encoder_cnt;
}

void AdjustOnEncoder(sys_state_t *state) {
  state->set_temperature = state->encoder_cnt*10;
  state->updated |= UPD_SET_TEMPERATURE;
}

void RenderFeatures(sys_state_t* state, uint8_t line) {
  if (state->interface->items[state->interface->selected + line].text
      && (state->interface->redraw || (state->updated & UPD_ENCODER))) {
      LCD_goto(line + 1, 0);
      LCD_string(
          state->interface->items[state->interface->selected + line].text);
  }
  if (state->interface->items[state->interface->selected + line].features
      & SHOW_TEMPERATURE
      && (state->interface->redraw || (state->updated & UPD_TEMPERATURE))) {
    LCD_goto(line + 1, 7);
    char tempstr[10];
    sprintf(tempstr, "%7.2f\337C", state->temperature);
    LCD_string(tempstr);
  }
  if (state->interface->items[state->interface->selected + line].features
      & SHOW_SET_VALUE
      && (state->interface->redraw || (state->updated & UPD_SET_TEMPERATURE))) {
    //TODO: PWM MODE
    LCD_goto(line + 1, 0);
    char tempstr[6];
    sprintf(tempstr, "%3d\337C", state->set_temperature);
    LCD_string(tempstr);
  }
  if (state->interface->items[state->interface->selected + line].features
      & SHOW_PWM && (state->interface->redraw || (state->updated & UPD_PWM))) {
    LCD_goto(line + 1, 12);
    char tempstr[5];
    sprintf(tempstr, "%3d%%", state->pwm);
    LCD_string(tempstr);
  }
}

uint8_t RenderInterface(sys_state_t *state)  {

// startup interface: AdjustScreen
  if (!state->interface) {
    AdjustScreen(state);
  }

//  determine key presses

  if (state->updated & UPD_BUTTON && state->encoder_button == BTN_DOWN) {
    if (state->interface->items[state->interface->selected].OnPress) {
      state->interface->items[state->interface->selected].OnPress(state);
    } else if (state->interface->OnPress) {
      state->interface->OnPress(state);
    }
  }

  if (state->updated & UPD_BUTTON && state->encoder_button == BTN_LONGPRESS) {
    if (state->interface->items[state->interface->selected].OnLongPress) {
      state->interface->items[state->interface->selected].OnLongPress(state);
    } else if(state->interface->OnLongPress) {
      state->interface->OnLongPress(state);
    }
  }

  if (state->updated & UPD_ENCODER) {
    if (state->interface->items[state->interface->selected].OnEncoder) {
      state->interface->items[state->interface->selected].OnEncoder(state);
    } else if (state->interface->OnEncoder) {
      state->interface->OnEncoder(state);
    }
  }

//  draw interface
  if (state->interface->redraw) LCD_clear();

  uint8_t line;
  for (line=0;line<LCD_ROW_SIZE;line++) {

    if (state->interface->items[state->interface->selected+line].RenderItem) {
      state->interface->items[state->interface->selected+line].RenderItem(state, line);
    } else if (state->interface->Render) {
      state->interface->Render(state, line);
    }

  }

  state->interface->redraw = FALSE;
  return 0;
}
