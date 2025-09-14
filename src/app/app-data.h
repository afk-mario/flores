#pragma once

#include "sys/sys.h"

#define APP_LOAD_MAX_TIME 0.01f // 1/80
#define APP_CNTR_X        (SYS_DISPLAY_W * 0.5f)
#define APP_CNTR_Y        (SYS_DISPLAY_H * 0.5f)
const rec_i32 APP_SCRN_REC = {0, 0, SYS_DISPLAY_W, SYS_DISPLAY_H};
