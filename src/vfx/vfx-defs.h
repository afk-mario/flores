#pragma once

#include "base/types.h"

struct vfx_handle {
	u16 id;
};

struct vfx {
	u16 id;
	f32 timestamp;
	f32 duration;
	i16 x;
	i16 y;
	i16 w;
	i16 h;
};

struct vfxs {
	i16 free_list_cursor;
	u16 count;
	struct vfx *items;
	u8 *generations;
	u8 *free_list;
};
