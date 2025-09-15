#pragma once

#include "base/types.h"
#include "block/block-type.h"

struct piece {
	b16 fast_drop;
	i16 x;
	i16 y;
	f32 timestamp;
	enum block_type types[2];
};
