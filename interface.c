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
                                    .OnPress = HeatingScreen,
                                    .OnLongPress = SettingsScreen,
                                    .OnLoad = AdjustScreenOnLoad,
                                    .OnEncoder = AdjustOnEncoder,
                                    .encoder_arr = 50,
                                    .Render = RenderFeatures,
                                    .items_count = 2,
                                    .items = {
                                              {.features = SHOW_TEMPERATURE|SHOW_SET_VALUE},
                                              {.text = " PRESS TO START"}
                                    }
};

static interface_t heating_screen = {
                                     .OnPress = AdjustScreen,
                                     .OnUnload = HeatingScreenOnUnload,
                                     .OnLongPress = SettingsScreen,
                                     .Render = RenderFeatures,
                                     .items_count = 2,
                                     .items = {
                                               {.features = SHOW_TEMPERATURE|SHOW_SET_VALUE},
                                               {.features = SHOW_PWM,.text = "HEATING"}
                                     }
};

static interface_t settings_screen = {
                                      .OnLoad = SettingsScreenOnLoad,
                                      .OnLongPress = ExitSubInterface,
                                      .OnEncoder = SettingsOnEncoder,
                                      .Render = RenderMenuItemText,
                                      .encoder_cnt = 0,
                                      .items_count = 8,
                                      .items = {
                                                {.text = "Kc",
                                                 .parameter = &system_state.pid_params.kc,
                                                 .RenderItem = RenderMenuItemFloat,
                                                 .OnPress = AdjustFloat
                                                },
                                                {.text = "Ti"},
                                                {.text = "Td"},
                                                {.text = "Gain"},
                                                {.text = "PWM Mode"},
                                                {.text = "Save"},
                                                {.text = "Cancel"},
                                                {.text = "Load defaults"}
                                      }
};

static interface_t adjust_float_screen = {
                                          .OnLongPress = ExitSubInterface,
                                          .OnLoad = AdjustFloatScreenOnLoad,
                                          .OnEncoder = AdjustFloatScreenOnEncoder,
                                          .Render = RenderAdjustFloatScreen,
                                          .encoder_arr = 100,
                                          .items_count = 1,
                                          .items = {{.text = "Float:",
                                                     .OnLongPress = 0}}
};

/*
 *
 * Adjust float functions
 *
 */

void AdjustFloatScreenOnEncoder(sys_state_t *state) {
  *(float *)state->interface->parameter_settings.parameter =
      ((float)state->encoder_cnt/100);
}

void AdjustFloatScreenOnLoad(sys_state_t *state){
  state->interface->parameter_settings.max = &(float){0.0f};
  state->interface->parameter_settings.max = &(float){1.0f};
  state->interface->parameter_settings.parameter = &state->pid_params.kc;
  EncoderSetup(state->interface->encoder_arr,
               (uint16_t)((*(float *)state->interface->parameter_settings.parameter)*100));
}

void AdjustFloat (sys_state_t *state) {
  EnterSubInterface(state, &adjust_float_screen);
}

void RenderAdjustFloatScreen(sys_state_t *state, uint8_t item_number, uint8_t line) {

  if (!line) {
    LCD_goto(1,0);
    LCD_string(state->interface->items[item_number].text);
  } else {
    LCD_goto(2,0);
    char tempstr[10];
    sprintf(tempstr, "      <%3.2f>", *((float *)state->interface->parameter_settings.parameter) );
    LCD_string(tempstr);
  }
}

/*
 *
 *  Heating screen functions
 *
 */

void HeatingScreen(sys_state_t *state) {
  state->interface = &heating_screen;
  state->pid_params.enabled = TRUE;
  state->interface->redraw = TRUE;
}

void HeatingScreenOnUnload(sys_state_t *state) {
  state->pid_params.enabled = FALSE;
}

void RenderFeatures(sys_state_t* state, uint8_t item_number, uint8_t line) {


  if (state->interface->items[item_number].text
      && (state->interface->redraw || (state->updated & UPD_ENCODER))) {
      LCD_goto(line, 0);
      LCD_string(
          state->interface->items[item_number].text);
  }

  if ((state->interface->items[item_number].features
      & SHOW_TEMPERATURE )
      && (state->interface->redraw || (state->updated & UPD_TEMPERATURE))) {
    LCD_goto(line, 7);
    char tempstr[10];
    sprintf(tempstr, "%7.2f\337C", state->temperature);
    LCD_string(tempstr);
  }
  if ((state->interface->items[item_number].features
      & SHOW_SET_VALUE)
      && (state->interface->redraw || (state->updated & UPD_SET_TEMPERATURE))) {
    //TODO: PWM MODE
    LCD_goto(line, 0);
    char tempstr[6];
    sprintf(tempstr, "%3d\337C", state->set_temperature);
    LCD_string(tempstr);
  }
  if ((state->interface->items[item_number].features
      & SHOW_PWM) && (state->interface->redraw || (state->updated & UPD_PWM))) {
    LCD_goto(line, 12);
    char tempstr[5];
    sprintf(tempstr, "%d", state->pwm);
    LCD_string(tempstr);
  }
}


/*
 *
 * Adjust screen functions
 *
 */

void AdjustScreen(sys_state_t *state) {
  if (state->interface && state->interface->OnUnload) state->interface->OnUnload(state);
  state->interface = &adjust_screen;
  state->interface->OnLoad(state);
  state->interface->redraw = TRUE;
}

void AdjustOnEncoder(sys_state_t *state) {
  state->set_temperature = state->encoder_cnt*10;
  state->updated |= UPD_SET_TEMPERATURE;
}


void AdjustScreenOnLoad(sys_state_t *state) {
  EncoderSetup(state->interface->encoder_arr,state->set_temperature/10);
}


/*
 *
 * Settings screen functions
 *
 */

void SettingsScreen(sys_state_t *state) {
  EnterSubInterface(state,&settings_screen);
}

void SettingsOnEncoder(sys_state_t *state) {
  state->interface->selected = state->encoder_cnt;
  state->interface->redraw = TRUE;
}

char Mark(uint8_t line) {
  return line ?' ':'>';
}

void RenderMenuItemFloat(sys_state_t *state, uint8_t item_number, uint8_t line) {
  if (!state->interface->redraw) return;
  LCD_goto(line,0);
  if (state->interface->items[item_number].text) {
    char tempstr[16];
    sprintf(tempstr, "%c%-11s %3.2f", Mark(line),
            state->interface->items[item_number].text,
            *(float *)(state->interface->items[item_number].parameter));
    LCD_string(tempstr);
  }
}

void RenderMenuItemText(sys_state_t *state, uint8_t item_number, uint8_t line) {
  if (!state->interface->redraw) return;
  LCD_goto(line,0);

  if (state->interface->items[item_number].text) {
    char tempstr[16];
    sprintf(tempstr, "%c%s", Mark(line),
            state->interface->items[item_number].text);
    LCD_string(tempstr);
  }
}

void SettingsScreenOnLoad(sys_state_t *state) {
  EncoderSetup(state->interface->items_count-1,state->interface->encoder_cnt);
}


/*
 *
 * Generic functions
 *
 */

void EnterSubInterface(sys_state_t *state, interface_t *new_interface) {
  if (state->interface->OnUnload) state->interface->OnUnload(state);
  state->return_interface[state->return_interfaces_top++] = state->interface;
  state->interface = new_interface;
  if (state->interface->OnLoad) state->interface->OnLoad(state);
  state->interface->redraw = TRUE;
}

void ExitSubInterface(sys_state_t *state) {
  if (state->interface->OnUnload) state->interface->OnUnload(state);
  state->interface->selected = 0;
  state->interface = state->return_interface[--state->return_interfaces_top];
  state->return_interface[state->return_interfaces_top] = 0;
  if (state->interface->OnLoad) state->interface->OnLoad(state);
  state->interface->redraw = TRUE;
}


void RenderInterface(sys_state_t *state)  {

// startup interface: AdjustScreen
  if (!state->interface) {
    AdjustScreen(state);
  }

//  determine key presses

  if (( state->updated & UPD_BUTTON ) && state->encoder_button == BTN_DOWN) {
    if (state->interface->items[state->interface->selected].OnPress) {
      state->interface->items[state->interface->selected].OnPress(state);
    } else if (state->interface->OnPress) {
      state->interface->OnPress(state);
    }
  }

  if (( state->updated & UPD_BUTTON ) && state->encoder_button == BTN_LONGPRESS) {
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
  uint8_t item_number = state->interface->selected;

  for (line=0;line<LCD_ROW_SIZE;line++) {
    if (item_number == state->interface->items_count) item_number = 0;

    if (state->interface->items[item_number].RenderItem) {
      state->interface->items[item_number].RenderItem(state, item_number, line);
    } else if (state->interface->Render) {
      state->interface->Render(state, item_number, line);
    }

    item_number++;
  }

  state->interface->redraw = FALSE;
}
