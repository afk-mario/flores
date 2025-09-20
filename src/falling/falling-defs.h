#pragma once

#include "block/block-type.h"

#define FALLING_SPD 100

struct falling_handle {
	i16 id;
};

struct falling {
	i16 id;
	f32 dy;
	v2_i32 o;
	v2_i32 p;
	f32 timestamp;
	enum block_type type;
};
