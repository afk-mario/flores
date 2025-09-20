#pragma once

#include "base/marena.h"
#include "base/mem.h"
#include "base/types.h"
#include "block/block-type.h"
#include "engine/gfx/gfx.h"

struct garden_state {
	v2_i32 p;
	f32 angle;
	rot2 rot;
};

struct garden_rules {
	str8 axiom;
	str8 rules[127];
	f32 angle_delta;
	f32 length;
	i32 iterations_max;
};

struct garden_cell {
	u8 iterations;
	u8 length;
	f32 angle_delta;
	f32 start_angle;
	str8 axiom;
	str8 next;
	v2_i32 offset;
	f32 timestamp;
	struct gfx_ctx ctx;
	struct alloc alloc;
	struct marena marena;
	enum block_type type;
};

struct garden {
	struct garden_cell cells[6];
};
