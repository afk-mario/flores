#pragma once

#include "board/board-defs.h"
#include "scrns/scrn-game/scrn-game-defs.h"

void board_ini(struct board *board, f32 timestamp);
void board_upd(struct board *board, struct frame_info frame);
void board_drw(struct board *board);
void board_drw_dbg(struct board *board);

void board_block_set(struct board *board, struct block block, i32 x, i32 y);
void board_block_clr(struct board *board, i32 x, i32 y);
b32 board_block_has(struct board *board, i16 x, i16 y);
b32 board_block_has_px(struct board *board, i32 x, i32 y);
struct block *board_block_get(struct board *board, i32 x, i32 y);

v2_i32 board_idx_to_coords(struct board *board, i16 idx);
v2_i32 board_idx_to_px(struct board *board, i16 idx);
i16 board_coords_to_idx(struct board *board, i32 x, i32 y);
v2_i32 board_coords_to_px(struct board *board, i32 x, i32 y);
v2_i32 board_px_to_coords(struct board *board, i32 x, i32 y);
// v2_i32 board_px_to_idx(struct board *board, i32 x, i32 y);
