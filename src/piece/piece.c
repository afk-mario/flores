#include "piece.h"
#include "base/mathfunc.h"
#include "block/block.h"
#include "board/board.h"
#include "engine/input.h"
#include "piece/piece-data.h"

void
piece_ini(struct piece *piece, f32 timestamp)
{
	piece->fast_drop = false;
	piece->timestamp = timestamp + PIECE_WAIT;
}

b32
piece_upd(struct piece *piece, struct board *board, struct frame_info frame)
{
	b32 res        = false;
	f32 timestamp  = frame.timestamp;
	v2_i32 coords  = board_px_to_coords(board, piece->x, piece->y);
	i32 x          = piece->x;
	i32 y          = piece->y;
	i32 block_size = board->block_size;
	i32 c          = board->columns;
	i32 r          = board->rows;
	i32 miny       = I16_MIN;
	i32 maxy       = I16_MAX;

	if(piece->fast_drop) {
		y                = clamp_i32(y + PIECE_SPEED * 3, miny, maxy);
		piece->timestamp = timestamp + PIECE_WAIT;
	} else {
		if(inp_just_pressed(INP_DPAD_L)) {
			x = x - block_size;
		} else if(inp_just_pressed(INP_DPAD_R)) {
			x = x + block_size;
		} else if(inp_pressed(INP_DPAD_D) || piece->timestamp < timestamp) {
			y                = y + PIECE_SPEED;
			piece->timestamp = timestamp + PIECE_WAIT;
		} else if(inp_just_pressed(INP_DPAD_U)) {
			piece->fast_drop = true;
			y                = y + PIECE_SPEED;
			piece->timestamp = timestamp + PIECE_WAIT;
		}
	}

	b32 collides_x = piece_collides_x(piece, board, x, y);
	res            = piece_collides_y(piece, board, x, y);
	if(!collides_x) {
		piece->x = x;
	}
	piece->y = y;
	return res;
}

void
piece_drw(struct piece *piece, struct board *board)
{
	i32 block_size = board->block_size;
	i32 x          = piece->x;
	i32 y          = piece->y;
	struct block a = {.type = piece->types[0], .state = 1};
	struct block b = {.type = piece->types[1], .state = 1};
	block_drw(&a, x, y, block_size);
	block_drw(&b, x + block_size, y, block_size);
}

str8
piece_to_str(struct piece *piece, struct board *board, struct alloc alloc)
{
	v2_i32 coords = board_px_to_coords(board, piece->x, piece->y);
	str8 res      = str8_fmt_push(
        alloc,
        "piece:%d,%d %d,%d %s,%s",
        piece->x,
        piece->y,
        coords.x,
        coords.y,
        BLOCK_TYPE_LABELS[piece->types[0]].str,
        BLOCK_TYPE_LABELS[piece->types[1]].str);
	return res;
}

b32
piece_collides_x(struct piece *piece, struct board *board, i32 x, i32 y)
{
	b32 res        = false;
	i32 block_size = board->block_size;
	i32 c          = board->columns;
	i32 minx       = 0;
	i32 maxx       = (c - 2) * block_size;
	v2_i32 coords  = board_px_to_coords(board, x, y);
	res            = x > maxx || x < minx;

	res = res || board_block_has(board, coords.x, coords.y);
	res = res || board_block_has(board, coords.x + 1, coords.y);

	return res;
}

b32
piece_collides_y(struct piece *piece, struct board *board, i32 x, i32 y)
{
	b32 res        = false;
	i32 block_size = board->block_size;
	v2_i32 coords  = board_px_to_coords(board, x, y + block_size);
	if(coords.y < 0) {
		res = true;
	} else if(board_block_has(board, coords.x, coords.y)) {
		res = true;
	} else if(board_block_has(board, coords.x + 1, coords.y)) {
		res = true;
	}

	return res;
}
