#pragma once

#include "base/types.h"

#include "block/block-defs.h"

#define BOARD_COLUMNS 6
#define BOARD_ROWS    8

struct board {
	u8 columns;
	u8 rows;
	u8 block_size;
	struct block blocks[BOARD_COLUMNS * BOARD_ROWS];
};
