#include "app/app-defs.h"

void scrn_loading_ini(struct app *app);
void scrn_loading_upd(struct app *app, f32 dt);
void scrn_loading_drw(struct app *app);

b32 scrn_loading_start(struct loading_state *state, f32 time_start, f32 time_max, void *args);
b32 scrn_loading_end(struct loading_state *state, f32 time_start, f32 time_max, void *args);

b32
scrn_loading_db_init(struct loading_state *state, f32 time_start, f32 time_max, void *args);
b32 scrn_loading_push_paths(struct loading_state *state, f32 time_start, f32 time_max, void *args);

b32 scrn_loading_load(struct loading_state *state, f32 time_start, f32 time_max, void *args);
