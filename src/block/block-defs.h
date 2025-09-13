#pragma once

#include "base/types.h"

#define BLOCK_SIZE 28

struct block {
	u8 x;
	u8 y;
	u8 type;
};
