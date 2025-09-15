#pragma once

#include "piece-defs.h"
#include "scrns/scrn-game/scrn-game-defs.h"

void piece_ini(struct piece *piece, f32 timestamp);
b32 piece_upd(struct piece *piece, struct board *board, struct frame_info frame);
void piece_drw(struct piece *piece, struct board *board);
str8 piece_to_str(struct piece *piece, struct board *board, struct alloc alloc);
b32 piece_collides_y(struct piece *piece, struct board *board, i32 x, i32 y);
b32 piece_collides_x(struct piece *piece, struct board *board, i32 x, i32 y);
