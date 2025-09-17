#include "board.h"
#include "base/log.h"
#include "base/rec.h"
#include "base/types.h"
#include "base/v2.h"
#include "block/block-type.h"
#include "block/block.h"
#include "globals/g-gfx.h"
#include "piece/piece-defs.h"
#include "scrns/scrn-game/scrn-game-data.h"

void
board_ini(struct board *board, f32 timestamp)
{
	i32 r                 = board->rows;
	i32 c                 = board->columns;
	board->columns        = BOARD_COLUMNS;
	board->rows           = BOARD_ROWS;
	board->block_size     = BLOCK_SIZE;
	board->block_size_inv = 1.0f / (f32)BLOCK_SIZE;

	mclr_array(board->blocks);
}

void
board_upd(struct board *board, struct frame_info frame)
{
	f32 timestamp        = frame.timestamp;
	struct alloc scratch = frame.alloc;

	for(size i = 0; i < (size)ARRLEN(board->blocks); ++i) {
		struct block *block = board->blocks + i;
		if(block->type == BLOCK_TYPE_NONE) { continue; }
		block_upd(block, board->block_size);
	}
}

void
board_drw(struct board *board, enum game_theme theme)
{
	i32 r          = board->rows;
	i32 c          = board->columns;
	i32 block_size = board->block_size;

	{
		enum prim_mode bg = GAME_BOARD_BG[theme];
		g_color(bg);
		g_rec_fill(
			0,
			-1 * board->block_size,
			board->columns * board->block_size,
			(board->rows + 1) * board->block_size);
	}

	g_color(PRIM_MODE_BLACK);
	for(size i = 0; i < (size)ARRLEN(board->blocks); ++i) {
		struct block *block = board->blocks + i;
		if(block->type == BLOCK_TYPE_NONE) { continue; }
		v2_i32 p      = board_idx_to_px(board, i);
		v2_i32 coords = board_idx_to_coords(board, i);

		for(size j = 0; j < (size)ARRLEN(PIECE_ROTATIONS); ++j) {
			v2_i32 rotation     = PIECE_ROTATIONS[j];
			v2_i32 other_coords = v2_add_i32(coords, rotation);
			i32 other_id        = board_coords_to_idx(board, other_coords.x, other_coords.y);
			if(other_id > -1) {
				struct block *other_block = board->blocks + other_id;
				if(block->type == other_block->type) {
					v2_i32 un_offset = {rotation.x * (block_size * 0.5f), -rotation.y * (block_size * 0.5f)};
					v2_i32 cir_p     = v2_add_i32(p, (v2_i32){block_size * 0.5f, block_size * 0.5f});
					cir_p            = v2_add_i32(cir_p, un_offset);
					g_cir_fill(cir_p.x, cir_p.y, 10);
				}
			}
		}

		// block_drw(block, theme, p.x, p.y, board->block_size);
	}

	for(size i = 0; i < (size)ARRLEN(board->blocks); ++i) {
		struct block *block = board->blocks + i;
		if(block->type == BLOCK_TYPE_NONE) { continue; }
		v2_i32 p = board_idx_to_px(board, i);
		block_drw(block, theme, p.x, p.y, board->block_size);
	}
}

void
board_block_set(struct board *board, struct block block, i32 x, i32 y)
{
	i16 idx = board_coords_to_idx(board, x, y);
	log_info("board", "set: %d,%d[%d]=%d", x, y, idx, block.type);
	if(idx < 0 || idx > (size)ARRLEN(board->blocks)) {
		log_warn("board", "tried setting at invalid: %d,%d[%d]=%d", x, y, idx, block.type);
		return;
	}
	board->blocks[idx]    = block;
	board->blocks[idx].id = idx;
}

void
board_block_clr(struct board *board, i32 x, i32 y)
{
	i16 idx = board_coords_to_idx(board, x, y);
	if(idx < 0 || idx > (size)ARRLEN(board->blocks)) { return; }
	board->blocks[idx].id   = 0;
	board->blocks[idx].type = BLOCK_TYPE_NONE;
}

b32
board_block_has(struct board *board, i16 x, i16 y)
{
	b32 res = false;
	i16 idx = board_coords_to_idx(board, x, y);
	if(idx < 0 || idx > (size)ARRLEN(board->blocks)) { return res; }
	res = board->blocks[idx].type != BLOCK_TYPE_NONE;
	return res;
}

struct block *
board_block_get(struct board *board, i32 x, i32 y)
{
	struct block *res = NULL;
	i16 idx           = board_coords_to_idx(board, x, y);
	if(idx < 0 || idx > (size)ARRLEN(board->blocks)) { return res; }
	res = board->blocks + idx;
	return res;
}

v2_i32
board_idx_to_coords(struct board *board, i16 idx)
{
	v2_i32 res = {
		.x = idx % board->columns,
		.y = idx / board->columns,
	};
	return res;
}

v2_i32
board_idx_to_px(struct board *board, i16 idx)
{
	v2_i32 coords = board_idx_to_coords(board, idx);
	v2_i32 res    = board_coords_to_px(board, coords.x, coords.y);
	return res;
}

i16
board_coords_to_idx(struct board *board, i32 x, i32 y)
{
	i16 res = -1;
	if(y < 0 || x < 0) { return res; }
	res = (y * board->columns) + x;
	if(res > (size)ARRLEN(board->blocks)) { return res = -1; }
	return res;
}

v2_i32
board_coords_to_px(struct board *board, i32 x, i32 y)
{
	// top-left pixel of the tile at (x, y), with y=0 = bottom row
	v2_i32 res = {
		x * board->block_size,
		(board->rows - 1 - y) * board->block_size};
	return res;
}

v2_i32
board_px_to_coords(struct board *board, i32 px, i32 py)
{
	i32 cx     = px * board->block_size_inv;
	i32 tile_y = div_i32_floor(py, board->block_size);
	i32 cy     = (board->rows - 1) - tile_y;
	v2_i32 res = {cx, cy};
	return res;
}

void
board_drw_dbg(struct board *board)
{
	i32 r          = board->rows;
	i32 c          = board->columns;
	rec_i32 rec    = {0, 0, board->columns, board->rows};
	rec_i32 border = rec_i32_expand(rec, 1);
	g_color(PRIM_MODE_BLACK);
	g_rec_fill(REC_UNPACK(rec));
	g_color(PRIM_MODE_WHITE);
	g_rec(REC_UNPACK(border));

	g_color(PRIM_MODE_WHITE);
	for(size i = 0; i < (size)ARRLEN(board->blocks); ++i) {
		struct block *block = board->blocks + i;
		if(block->type == BLOCK_TYPE_NONE) { continue; }
		v2_i32 p = board_idx_to_coords(board, i);
		g_px(p.x, p.y);
	}
}
