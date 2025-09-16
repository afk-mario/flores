#pragma once

#include "base/types.h"

#include "block/block-defs.h"

#define BOARD_COLUMNS    6
#define BOARD_ROWS       8
#define BOARD_USE_SHAPES false

struct board {
	i16 columns;
	i16 rows;
	i16 block_size;
	f32 block_size_inv;
	struct block blocks[(BOARD_COLUMNS * (BOARD_ROWS + 1)) + 1];
};
