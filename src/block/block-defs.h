#pragma once

#include "base/types.h"

#define BLOCK_SIZE 28

struct block_handle {
	u16 id;
};

struct block {
	u16 id;
	u8 bx;
	u8 by;
	i16 x;
	i16 y;
	u8 type;
	u8 state;
};
