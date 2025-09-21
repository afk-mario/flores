#pragma once

#include "base/marena.h"
#include "base/types.h"
#include "block/block-type.h"
#include "engine/gfx/gfx.h"
#include "lib/rndm.h"

struct flower_state {
	v2_i32 p;
	f32 angle;
	rot2 rot;
};

struct flower_bud {
	i16 x;
	i16 y;
	u8 water;
};

struct flower_rules {
	str8 axiom;
	str8 rules[127];
	f32 angle_delta;
	f32 length;
	i32 iterations_max;
};

struct flower {
	struct rndm rndm;
	b32 is_full;
	v2_i32 p;
	i32 water;
	u8 iterations;
	f32 length;
	f32 angle_delta;
	f32 start_angle_og;
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
