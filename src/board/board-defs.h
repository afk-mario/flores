#pragma once

#include "base/types.h"

#include "block/block-defs.h"
#include "falling/falling-defs.h"

#define BOARD_COLUMNS 6
#define BOARD_ROWS    8

struct board {
	i16 columns;
	i16 rows;
	i16 block_size;
	f32 block_size_inv;
	struct block blocks[(BOARD_COLUMNS * (BOARD_ROWS + 1)) + 1];
	struct falling fallings[(BOARD_COLUMNS * (BOARD_ROWS + 1)) + 1];
};
