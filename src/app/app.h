#pragma once

#include "app-defs.h"
#include "base/types.h"
#include "scrns/scrn-type.h"

void app_init(usize mem_size);
void app_tick(f32 dt);
void app_draw(void);
void app_close(void);
void app_resume(void);
void app_pause(void);

void app_load_assets(void);
void app_set_scrn(struct app *app, enum scrn_type scrn);
