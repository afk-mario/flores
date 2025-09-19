#include "app/app-defs.h"

void scrn_game_ini(struct app *app);
void scrn_game_upd(struct app *app, f32 dt);
void scrn_game_drw(struct app *app);
void scrn_game_state_set(struct scrn_game *scrn, enum scrn_game_state value);
void scrn_game_state_pop(struct scrn_game *scrn);
