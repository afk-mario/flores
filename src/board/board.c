#include "board.h"
#include "block/block-type.h"
#include "block/block.h"
#include "lib/rndm.h"

void
board_ini(struct board *board)
{
	i32 r             = board->rows;
	i32 c             = board->columns;
	board->columns    = BOARD_COLUMNS;
	board->rows       = BOARD_ROWS;
	board->block_size = BLOCK_SIZE;
	i32 block_count   = BOARD_COLUMNS * BOARD_ROWS;

	for(size i = 0; i < block_count; ++i) {
		board->blocks[i].x    = (i % board->columns) * BLOCK_SIZE;
		board->blocks[i].y    = (i / board->columns) * BLOCK_SIZE;
		board->blocks[i].type = rndm_range_i32(NULL, BLOCK_TYPE_NONE + 1, BLOCK_TYPE_C);
	}
}

void
board_upd(struct board *board)
{
}

void
board_drw(struct board *board)
{
	i32 r = board->rows;
	i32 c = board->columns;
	for(size i = 0; i < r * c; ++i) {
		struct block *block = board->blocks + i;
		if(block->type == BLOCK_TYPE_NONE) { continue; }
		block_drw(block, board->block_size);
	}
}
