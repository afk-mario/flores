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
