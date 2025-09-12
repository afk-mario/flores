#pragma once

// dirx=split"-1,1,0,0,1,1,-1,-1"
// diry=split"0,0,-1,1,-1,1,1,-1"

#include "base/types.h"
#include "engine/input.h"
#include "globals/g-ui-actions.h"

static u32 G_UI_MAP_DEFAULT[G_UI_NUM_COUNT] = {
    [G_UI_ACCEPT] = INP_A | INP_MOUSE_RIGHT,
    [G_UI_BACK] = INP_B | INP_MOUSE_LEFT,
    [G_UI_LEFT] = INP_DPAD_L,
    [G_UI_RIGHT] = INP_DPAD_R,
    [G_UI_UP] = INP_DPAD_U,
    [G_UI_DOWN] = INP_DPAD_D,
};
static u32 G_UI_MAP[G_UI_NUM_COUNT] = {0};

void g_ui_init(void) { mcpy_array(G_UI_MAP, G_UI_MAP_DEFAULT); }

u8 g_ui_btn(enum g_ui act) { return G_UI_MAP[act]; }

b32 g_ui_pressed(enum g_ui act) {
  b32 res = inp_pressed(G_UI_MAP[act]);
  return res;
}

b32 g_ui_pressed_any(void) {
  i32 btn = G_UI_MAP[G_UI_ACCEPT] | G_UI_MAP[G_UI_BACK] | G_UI_MAP[G_UI_UP] |
            G_UI_MAP[G_UI_DOWN] | G_UI_MAP[G_UI_RIGHT] | G_UI_MAP[G_UI_LEFT];
  b32 res = inp_pressed(btn);
  return res;
}

b32 g_ui_was_pressed(enum g_ui act) {
  b32 res = inp_was_pressed(G_UI_MAP[act]);
  return res;
}

b32 g_ui_was_pressed_any(void) {
  i32 btn = G_UI_MAP[G_UI_ACCEPT] | G_UI_MAP[G_UI_BACK] | G_UI_MAP[G_UI_UP] |
            G_UI_MAP[G_UI_DOWN] | G_UI_MAP[G_UI_RIGHT] | G_UI_MAP[G_UI_LEFT];
  b32 res = inp_was_pressed(btn);
  return res;
}

b32 g_ui_just_pressed(enum g_ui act) {
  b32 res = inp_just_pressed(G_UI_MAP[act]);
  return res;
}

b32 g_ui_just_pressed_any(void) {
  i32 btn = G_UI_MAP[G_UI_ACCEPT] | G_UI_MAP[G_UI_BACK] | G_UI_MAP[G_UI_UP] |
            G_UI_MAP[G_UI_DOWN] | G_UI_MAP[G_UI_RIGHT] | G_UI_MAP[G_UI_LEFT];
  b32 res = inp_just_pressed(btn);
  return res;
}
