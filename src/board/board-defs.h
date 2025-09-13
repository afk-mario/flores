#pragma once

#include "base/types.h"

#include "block/block-defs.h"
#include "block/block-type.h"
#include "board/board-state.h"

#define BOARD_COLUMNS 6
#define BOARD_ROWS    8

struct piece {
	i16 x;
	i16 y;
	f32 timestamp;
	enum block_type types[2];
};

struct board {
	u8 columns;
	u8 rows;
	u8 block_size;
	enum board_state state;
	struct piece piece;
	struct block blocks[(BOARD_COLUMNS * (BOARD_ROWS + 1)) + 1];
};
