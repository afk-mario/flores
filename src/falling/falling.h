#pragma once

#include "board/board-defs.h"
#include "falling/falling-defs.h"
#include "scrns/scrn-game/scrn-game-theme.h"

b32 falling_upd(struct falling *falling, struct board *board, f32 timestamp);
void falling_drw(struct falling *falling, struct board *board, enum game_theme theme);
