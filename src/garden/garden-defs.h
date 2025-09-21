#pragma once

#include "base/marena.h"
#include "base/mem.h"
#include "base/types.h"
#include "block/block-type.h"
#include "board/board-defs.h"
#include "engine/gfx/gfx.h"
#include "garden/flower-defs.h"

#define GARDEN_FLOWERS_MAX          12
#define GARDEN_FLOWERS_MAX_PER_CELL 1

struct garden_cell {
	u16 count;
	u16 handles[GARDEN_FLOWERS_MAX_PER_CELL];
};

struct garden_slicing {
	i32 index;
	f32 timestamp;
};

struct garden_time {
	i32 hour;
	i32 day;
	i32 is_day;
	f32 timestamp;
};

struct garden {
	struct garden_time time;
	b32 is_loaded;
	struct tex *textures;
	i32 flower_idx;
	struct garden_slicing slicing;
	struct flower *flowers;
	struct garden_cell cells[BOARD_COLUMNS];
};
