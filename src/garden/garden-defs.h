#pragma once

#include "base/marena.h"
#include "base/mem.h"
#include "base/types.h"

struct garden_state {
	v2_i32 p;
	f32 angle;
};

struct garden {
	u8 iterations;
	u8 length;
	f32 angle_delta;

	str8 axiom;
	str8 next;

	struct alloc alloc;
	struct marena marena;
};
