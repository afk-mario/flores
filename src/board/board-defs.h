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
	i32 falling_count;
	struct block blocks[(BOARD_COLUMNS * (BOARD_ROWS + 1)) + 1];
	struct falling fallings[(BOARD_COLUMNS * (BOARD_ROWS + 1)) + 1];
	u8 matches[BOARD_COLUMNS * BOARD_ROWS];
	u8 visited[BOARD_COLUMNS * BOARD_ROWS];
};
