#pragma once

#include "block/block-type.h"

struct falling_handle {
	i16 id;
};

struct falling {
	i16 id;
	v2_i32 o;
	v2_i32 p;
	enum block_type type;
};
