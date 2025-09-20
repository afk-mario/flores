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

struct garden {
	b32 is_loaded;
	struct tex *textures;
	i32 flower_idx;
	struct flower *flowers;
	struct garden_cell cells[BOARD_COLUMNS];
};
