#pragma once
#include "base/marena.h"
#include "base/mem.h"
#include "base/types.h"

struct frame_info {
	f32 dt;
	f32 dt_inv;
	f32 timestamp;
	struct alloc alloc;
	struct marena marena;
};

struct block {
	u8 x;
	u8 y;
	u8 type;
};

struct scrn_game {
	struct frame_info frame;
};
