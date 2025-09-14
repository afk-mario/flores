#include "board.h"
#include "base/dbg.h"
#include "base/mathfunc.h"
#include "base/types.h"
#include "block/block-type.h"
#include "block/block.h"
#include "engine/input.h"
#include "lib/rndm.h"

static inline void board_spawn_rndm_piece(struct board *board, f32 timestamp);
static inline struct block_handle board_spawn_block(struct board *board, struct block block);
static inline void board_remove_block(struct board *board, struct block_handle handle);
static inline b32 board_has_block(struct board *board, u8 x, u8 y);

#define BOARD_FULL
#define PIECE_SPEED 0.8f

void
board_ini(struct board *board, f32 timestamp)
{
	i32 r             = board->rows;
	i32 c             = board->columns;
	board->columns    = BOARD_COLUMNS;
	board->rows       = BOARD_ROWS;
	board->block_size = BLOCK_SIZE;
	i32 block_count   = BOARD_COLUMNS * BOARD_ROWS;
#if !defined(BOARD_FULL)
	board_spawn_rndm_piece(board, timestamp);
#endif

	mclr_array(board->blocks);

#if defined(BOARD_FULL)
	for(size i = 0; i < block_count; ++i) {
		struct block block = {
			.x    = (i % board->columns),
			.y    = (i / board->columns),
			.type = rndm_range_i32(NULL, BLOCK_TYPE_NONE + 1, BLOCK_TYPE_C),
		};
		board_spawn_block(board, block);
	}
#endif
}

void
board_upd(struct board *board, struct frame_info frame)
{
	f32 timestamp = frame.timestamp;

	switch(board->state) {
	case BOARD_STATE_PIECE: {
		struct piece *piece = &board->piece;
		i32 x               = piece->x;
		i32 y               = piece->y;
		if(inp_just_pressed(INP_DPAD_L)) {
			x = max_i32(x - 1, 0);
		} else if(inp_just_pressed(INP_DPAD_R)) {
			x = min_i32(x + 1, board->columns - 2);
		} else if(inp_just_pressed(INP_DPAD_D)) {
			y                = min_i32(y + 1, board->rows);
			piece->timestamp = timestamp + PIECE_SPEED;
		} else if(piece->timestamp < timestamp) {
			y                = min_i32(y + 1, board->rows);
			piece->timestamp = timestamp + PIECE_SPEED;
		}

		// TODO: Fix this
		if(inp_just_pressed(INP_DPAD_U)) {
			for(size i = piece->y; i < board->rows; ++i) {
				if(
					!board_has_block(board, x, i) &&
					!board_has_block(board, x + 1, i)) {
					y = i;
				}
			}
		}

		if(
			y == board->rows ||
			board_has_block(board, x, y) ||
			board_has_block(board, x + 1, y)) {
			if(piece->y == -1) {
				board->state = BOARD_STATE_OVER;
			} else {
				board_spawn_block(board, (struct block){.x = piece->x, .y = piece->y, .type = piece->types[0]});
				board_spawn_block(board, (struct block){.x = piece->x + 1, .y = piece->y, .type = piece->types[1]});
				board_spawn_rndm_piece(board, timestamp);
			}
		} else {
			piece->x = x;
			piece->y = y;
		}
	} break;
	default: {
	} break;
	}
}

void
board_drw(struct board *board)
{
	i32 r = board->rows;
	i32 c = board->columns;
	switch(board->state) {
	case BOARD_STATE_PIECE: {
		struct piece *piece = &board->piece;
		i32 block_size      = board->block_size;
		struct block a      = {.x = piece->x, .y = piece->y, .type = piece->types[0]};
		struct block b      = {.x = (piece->x + 1), .y = piece->y, .type = piece->types[1]};
		block_drw(&a, board->block_size);
		block_drw(&b, board->block_size);

	} break;
	default: {
	} break;
	}

	for(size i = 0; i < (size)ARRLEN(board->blocks); ++i) {
		struct block *block = board->blocks + i;
		if(block->type == BLOCK_TYPE_NONE) { continue; }
		block_drw(block, board->block_size);
	}
}

static inline struct block_handle
board_spawn_block(struct board *board, struct block block)
{
	dbg_assert(!board_has_block(board, block.x, block.y));
	struct block_handle res = {0};
	for(size i = 1; i < (size)ARRLEN(board->blocks); ++i) {
		if(board->blocks[i].id == 0) {
			board->blocks[i]    = block;
			board->blocks[i].id = i;
			res.id              = i;
			break;
		}
	}
	return res;
}

static inline void
board_remove_block(struct board *board, struct block_handle handle)
{
	dbg_assert(handle.id > 0 && handle.id < ARRLEN(board->blocks));
	board->blocks[handle.id].id = 0;
}

static inline void
board_spawn_rndm_piece(struct board *board, f32 timestamp)
{
	board->piece.types[0]  = rndm_range_i32(NULL, BLOCK_TYPE_NONE + 1, BLOCK_TYPE_C);
	board->piece.types[1]  = rndm_range_i32(NULL, BLOCK_TYPE_NONE + 1, BLOCK_TYPE_C);
	board->piece.x         = board->columns * 0.5f;
	board->piece.y         = -1;
	board->piece.timestamp = timestamp + PIECE_SPEED;
	board->state           = BOARD_STATE_PIECE;
}

static inline b32
board_has_block(struct board *board, u8 x, u8 y)
{
	b32 res = false;
	for(size i = 0; i < (size)ARRLEN(board->blocks); ++i) {
		if(board->blocks[i].id != 0 && board->blocks[i].x == x && board->blocks[i].y == y) {
			return true;
			break;
		}
	}
	return res;
}
