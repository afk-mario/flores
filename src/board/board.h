#pragma once

#include "board/board-defs.h"
#include "scrns/scrn-game/scrn-game-defs.h"

void board_ini(struct board *board, f32 timestamp);
void board_upd(struct board *board, struct frame_info frame);
void board_drw(struct board *board);
