#include "piece.h"
#include "base/types.h"
#include "base/v2.h"
#include "block/block.h"
#include "board/board.h"
#include "engine/input.h"
#include "globals/g-gfx.h"
#include "piece/piece-data.h"

void
piece_ini(struct piece *piece, f32 timestamp)
{
	piece->timestamp = timestamp + PIECE_WAIT;
	piece->upd       = piece_upd_inp;
}

b32
piece_upd(struct piece *piece, struct board *board, struct frame_info frame)
{
	b32 res           = false;
	f32 timestamp     = frame.timestamp;
	i32 block_size    = board->block_size;
	i32 c             = board->columns;
	i32 r             = board->rows;
	b32 was_colliding = piece_collides(piece, board, 0, -1);
	b32 time_up       = piece->timestamp < timestamp;

	piece->upd(piece, board, frame);

	b32 is_drop      = piece->fast_drop;
	b32 is_colliding = piece_collides(piece, board, 0, -1);
	if(
		(is_colliding && time_up) ||
		(is_colliding && time_up)) {
		res = true;
	}

	return res;
}

void
piece_drw(struct piece *piece, struct board *board)
{
	i32 block_size = board->block_size;
	v2_i32 px      = board_coords_to_px(board, piece->p.x, piece->p.y);
	v2_i32 px_o    = v2_add_i32(px, piece->o);
	struct block a = {.type = piece->types[0], .state = 1};
	struct block b = {.type = piece->types[1], .state = 1};

	g_pat(gfx_pattern_50());
	block_drw(&a, px.x, px.y, block_size);
	block_drw(&b, px.x + block_size, px.y, block_size);

	g_pat(gfx_pattern_100());
	block_drw(&a, px_o.x, px_o.y, block_size);
	block_drw(&b, px_o.x + block_size, px_o.y, block_size);
}

str8
piece_to_str(struct piece *piece, struct board *board, struct alloc alloc)
{
	v2_i32 coords = piece->p;
	v2_i32 px     = board_coords_to_px(board, piece->p.x, piece->p.y);
	str8 res      = str8_fmt_push(
        alloc,
        "piece:%d,%d %d,%d %s,%s",
        piece->p.x,
        piece->p.y,
        coords.x,
        coords.y,
        BLOCK_TYPE_LABELS[piece->types[0]].str,
        BLOCK_TYPE_LABELS[piece->types[1]].str);
	return res;
}

b32
piece_collides(struct piece *piece, struct board *board, i32 dx, i32 dy)
{
	b32 res        = false;
	i32 block_size = board->block_size;
	v2_i32 dest    = {piece->p.x + dx, piece->p.y + dy};
	b32 has_block  = board_block_has(board, dest.x, dest.y) || board_block_has(board, dest.x + 1, dest.y);
	b32 is_wall    = dest.x < 0 || dest.x > board->columns - 2;
	is_wall        = is_wall || dest.y < 0;
	return has_block || is_wall;
}

b32
piece_move_x(
	struct piece *piece,
	struct board *board,
	i32 dx,
	f32 timestamp)
{
	b32 res        = false;
	i32 block_size = board->block_size;
	v2_i32 dest    = {piece->p.x + dx, piece->p.y};
	b32 has_block  = board_block_has(board, dest.x, dest.y) || board_block_has(board, dest.x + 1, dest.y);
	b32 is_wall    = dest.x < 0 || dest.x > board->columns - 2;
	if(is_wall) {
		piece->so.x          = dx * (block_size * 0.5f);
		piece->o.x           = piece->so.x;
		piece->ani_timestamp = timestamp;
		piece->ani_duration  = PIECE_ANI_DUR * 0.5f;
		piece->upd           = piece_upd_bump;
	} else if(!has_block) {
		res                  = true;
		piece->p             = dest;
		piece->so.x          = -dx * block_size;
		piece->o.x           = piece->so.x;
		piece->ani_timestamp = timestamp;
		piece->ani_duration  = PIECE_ANI_DUR;
		piece->upd           = piece_upd_move_x;
	}
	return res;
}

b32
piece_move_y(struct piece *piece, struct board *board, i32 dy, f32 timestamp)
{
	b32 res        = false;
	i32 block_size = board->block_size;
	v2_i32 dest    = {piece->p.x, piece->p.y + dy};
	b32 has_block  = board_block_has(board, dest.x, dest.y) || board_block_has(board, dest.x + 1, dest.y);
	b32 is_wall    = dest.y < 0;
	if(is_wall) {
	} else if(!has_block) {
		res       = true;
		piece->p  = dest;
		piece->o  = (v2_i32){0};
		piece->so = (v2_i32){0};
	}
	return res;
}

void
piece_upd_inp(struct piece *piece, struct board *board, struct frame_info frame)
{
	f32 timestamp  = frame.timestamp;
	i32 dx         = 0;
	i32 dy         = 0;
	i32 ody        = 0;
	i32 block_size = board->block_size;
	b32 time_up    = piece->timestamp < timestamp;
	b32 collides   = piece_collides(piece, board, 0, -1);

	if(inp_just_pressed(INP_DPAD_L)) {
		dx = -1;
	} else if(inp_just_pressed(INP_DPAD_R)) {
		dx = 1;
	} else if(inp_just_pressed(INP_DPAD_D)) {
		if(!collides) {
			ody = block_size;
		}
	} else if(inp_just_pressed(INP_DPAD_U)) {
		if(!collides) {
			piece->fast_drop = true;
			piece->upd       = piece_upd_drop;
			return;
		}
	} else if(time_up) {
		if(!collides) {
			ody = PIECE_GRAVITY;
		}
	}

	if(ody > 0) {
		piece->o.y       = piece->o.y + ody;
		piece->timestamp = timestamp + PIECE_WAIT;
		if(piece->o.y >= block_size) { dy = -1; }
	}

	if(dy != 0) {
		piece_move_y(piece, board, dy, timestamp);
		piece->timestamp = timestamp + PIECE_WAIT;
	} else if(dx != 0) {
		b32 did_move = piece_move_x(piece, board, dx, timestamp);
	}
}

void
piece_upd_move_x(struct piece *piece, struct board *board, struct frame_info frame)
{
	f32 timestamp = frame.timestamp;
	f32 dur       = timestamp - piece->ani_timestamp;
	f32 t         = clamp_f32(dur / piece->ani_duration, 0, 1.0f);
	piece->o.x    = piece->so.x * (1.0f - t);
	if(t == 1.0f) {
		piece->upd = piece_upd_inp;
	}
}

void
piece_upd_move_y(struct piece *piece, struct board *board, struct frame_info frame)
{
	f32 timestamp = frame.timestamp;
	f32 dur       = timestamp - piece->ani_timestamp;
	f32 t         = clamp_f32(dur / piece->ani_duration, 0, 1.0f);
	piece->o.y    = piece->so.y * (1.0f - t);
	if(t == 1.0f) {
		piece->upd = piece_upd_inp;
	}
}

void
piece_upd_bump(struct piece *piece, struct board *board, struct frame_info frame)
{
	f32 timestamp = frame.timestamp;
	f32 dur       = timestamp - piece->ani_timestamp;
	f32 t         = clamp_f32(dur / piece->ani_duration, 0, 1.0f);
	f32 t2        = t;
	if(t > 0.5f) {
		t2 = 1.0f - t;
	}
	piece->o.x = piece->so.x * t2;

	if(t == 1.0f) {
		piece->upd = piece_upd_inp;
	}
}

void
piece_upd_drop(struct piece *piece, struct board *board, struct frame_info frame)
{
	f32 timestamp  = frame.timestamp;
	i32 block_size = board->block_size;
	i32 dy         = 0;
	piece->o.y     = piece->o.y + (PIECE_GRAVITY * 2);
	if(piece->o.y >= block_size) { dy = -1; }
	if(dy != 0) {
		piece_move_y(piece, board, dy, timestamp);
	}
	if(piece_collides(piece, board, 0, dy)) {
		piece->upd       = piece_upd_inp;
		piece->timestamp = timestamp + PIECE_WAIT;
	}
}
