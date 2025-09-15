#pragma once

#include "base/types.h"

#define BLOCK_SIZE 28

struct block_handle {
	i16 id;
};

struct block {
	i16 id;
	i16 type;
	i16 state;
};
