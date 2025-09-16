#pragma once

#include "piece-defs.h"

void piece_ini(struct piece *piece, f32 timestamp);
b32 piece_upd(struct piece *piece, struct board *board, struct frame_info frame);
void piece_drw(struct piece *piece, struct board *board);
str8 piece_to_str(struct piece *piece, struct board *board, struct alloc alloc);

b32 piece_move_x(struct piece *piece, struct board *board, i32 dx, f32 timestamp);
b32 piece_move_y(struct piece *piece, struct board *board, i32 dy, f32 timestamp);
b32 piece_collides(struct piece *piece, struct board *board, i32 dx, i32 dy);

void piece_upd_inp(struct piece *piece, struct board *board, struct frame_info frame);
void piece_upd_move_x(struct piece *piece, struct board *board, struct frame_info frame);
void piece_upd_move_y(struct piece *piece, struct board *board, struct frame_info frame);
void piece_upd_drop(struct piece *piece, struct board *board, struct frame_info frame);
void piece_upd_bump(struct piece *piece, struct board *board, struct frame_info frame);
