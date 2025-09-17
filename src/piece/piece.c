#include "piece.h"
#include "base/mathfunc.h"
#include "base/types.h"
#include "base/v2.h"
#include "block/block.h"
#include "board/board.h"
#include "engine/input.h"
#include "globals/g-gfx.h"
#include "piece/piece-data.h"

static inline i32 piece_handle_ody(struct piece *piece, struct board *board, i32 ody, f32 timestamp);
static inline i32 piece_get_btn(struct piece *piece);
static inline void piece_bump(struct piece *piece, i32 direction, f32 timestamp);
static inline void piece_do_btn(struct piece *piece, struct board *board, struct frame_info frame);

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

	piece->btn_buffer = piece_get_btn(piece);

	piece->upd(piece, board, frame);

	b32 is_drop      = piece->fast_drop;
	b32 is_colliding = piece_collides(piece, board, 0, -1);
	if(is_colliding && time_up && piece->o.y >= 0) {
		res = true;
	}

	return res;
}

void
piece_drw(struct piece *piece, struct board *board, enum game_theme theme)
{
	i32 block_size = board->block_size;
	v2_i32 px      = board_coords_to_px(board, piece->p.x, piece->p.y);
	v2_i32 px_o    = v2_add_i32(px, piece->o);
	struct block a = {.type = piece->types[0], .state = 1};
	struct block b = {.type = piece->types[1], .state = 1};

#if DEBUG
	g_pat(gfx_pattern_50());
	block_drw(&a, theme, px.x, px.y, block_size);
	block_drw(&b, theme, px.x + block_size, px.y, block_size);
#endif

	g_pat(gfx_pattern_100());
	block_drw(&a, theme, px_o.x, px_o.y, block_size);
	block_drw(&b, theme, px_o.x + block_size, px_o.y, block_size);
}

str8
piece_to_str(struct piece *piece, struct board *board, struct alloc alloc)
{
	v2_i32 coords = piece->p;
	v2_i32 px     = board_coords_to_px(board, piece->p.x, piece->p.y);
	str8 res      = str8_fmt_push(
        alloc,
        "piece:[%d,%d][%d,%d]btn:%d o.y=%d %s,%s",
        px.x,
        px.y,
        coords.x,
        coords.y,
        piece->btn_buffer,
        piece->o.y,
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
	b32 collides   = piece_collides(piece, board, dx, -1);

	if(has_block && !is_wall) {
		if(piece->o.y < -1 && !piece_collides(piece, board, dx, 1)) {
			has_block  = false;
			dest.y     = dest.y + 1;
			piece->o.y = 0;
		}
	}

	if(has_block || is_wall) {
		piece_bump(piece, dx, timestamp);
		if(collides) {
			piece->timestamp = 0;
		}
	} else {
		if(piece_collides(piece, board, dx, -1)) {
			piece->so.y = 0;
			piece->o.y  = 0;
		} else if(piece->o.y < 0) {
			piece->so.y = 0;
			piece->o.y  = 0;
		}
		res                  = true;
		piece->p             = dest;
		piece->so.x          = -dx * block_size;
		piece->o.x           = piece->so.x;
		piece->ani_timestamp = timestamp;
		piece->ani_duration  = PIECE_ANI_MOVE_DUR;
		piece->upd           = piece_upd_move_x;
		piece->timestamp     = timestamp + PIECE_WAIT;
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
		res      = true;
		piece->p = dest;
	}
	return res;
}

void
piece_upd_inp(struct piece *piece, struct board *board, struct frame_info frame)
{
	f32 timestamp = frame.timestamp;
	b32 time_up   = piece->timestamp < timestamp;

	piece_do_btn(piece, board, frame);
	if(time_up) {
		piece_handle_ody(piece, board, PIECE_GRAVITY, timestamp);
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
	i32 ody        = PIECE_DROP_SPD;
	dy             = piece_handle_ody(piece, board, ody, timestamp);

	if(dy != 0) {
		piece_move_y(piece, board, dy, timestamp);
	}
	if(piece_collides(piece, board, 0, dy)) {
		piece->o.y       = 0;
		piece->upd       = piece_upd_inp;
		piece->timestamp = timestamp + PIECE_WAIT;
	}
}

static inline i32
piece_handle_ody(struct piece *piece, struct board *board, i32 ody, f32 timestamp)
{
	i32 res = 0;
	if(ody == 0) { return res; }
	i32 block_size   = board->block_size;
	piece->o.y       = piece->o.y + ody;
	piece->timestamp = timestamp + PIECE_WAIT;

	if(piece->o.y >= (block_size / 2) - 1) {
		piece->o.y = (-block_size / 2) + 1;
		res        = -1;
	}

	if(res != 0) {
		piece_move_y(piece, board, res, timestamp);
	}
	return res;
}

static inline i32
piece_get_btn(struct piece *piece)
{
	i32 res   = piece->btn_buffer;
	i32 inp[] = {INP_DPAD_U, INP_DPAD_L, INP_DPAD_R, INP_DPAD_D};
	for(size i = 0; i < (size)ARRLEN(inp); ++i) {
		if(inp_just_pressed(inp[i])) {
			res = inp[i];
		}
	}
	return res;
}

static inline void
piece_do_btn(struct piece *piece, struct board *board, struct frame_info frame)
{

	f32 timestamp     = frame.timestamp;
	i32 dx            = 0;
	b32 collides      = piece_collides(piece, board, 0, -1);
	i32 btn           = piece->btn_buffer;
	piece->btn_buffer = 0;

	if(btn == INP_DPAD_U) {
		piece->fast_drop = true;
		piece->upd       = piece_upd_drop;
	} else if(btn == INP_DPAD_L || inp_pressed(INP_DPAD_L)) {
		dx = -1;
	} else if(btn == INP_DPAD_R || inp_pressed(INP_DPAD_R)) {
		dx = 1;
	} else if(btn == INP_DPAD_D || inp_pressed(INP_DPAD_D)) {
		if(collides) {
			piece->o.y = 0;
		} else {
			piece_handle_ody(piece, board, PIECE_GRAVITY, timestamp);
		}
	}

	if(dx != 0) {
		b32 did_move = piece_move_x(piece, board, dx, timestamp);
	}
}

static inline void
piece_bump(struct piece *piece, i32 direction, f32 timestamp)
{
	piece->so.x          = direction * PIECE_BUMP;
	piece->o.x           = piece->so.x;
	piece->ani_timestamp = timestamp;
	piece->ani_duration  = PIECE_ANI_BUMP_DUR;
	piece->upd           = piece_upd_bump;
}
