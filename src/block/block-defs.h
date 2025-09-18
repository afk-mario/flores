#pragma once

#include "base/types.h"

#define BLOCK_SIZE   28
#define BLOCK_LINK_D 15

struct block {
	i16 type;
	i16 state;
};
