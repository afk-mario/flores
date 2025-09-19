#pragma once

#include "base/types.h"
#include "engine/gfx/gfx.h"
#include "globals/g-gfx.h"

struct vfx_handle {
	u16 id;
};

enum vfx_type {
	VFX_TYPE_NONE,

	VFX_TYPE_SHAPE,
	VFX_TYPE_SPR,
	VFX_TYPE_TXT,

	VFX_TYPE_NUM_COUNT,

};

// TODO: Maybe this is a gfx shape
enum vfx_shape_type {
	VFX_SHAPE_TYPE_NONE,

	VFX_SHAPE_TYPE_CIR,
	VFX_SHAPE_TYPE_CIR_FILL,
	VFX_SHAPE_TYPE_REC,
	VFX_SHAPE_TYPE_REC_FILL,

	VFX_SHAPE_TYPE_NUM_COUNT,
};

enum vfx_blink_type {
	VFX_BLINK_NONE,

	VFX_BLINK_TRANSPARENT,
	VFX_BLINK_COLOR,
};

struct vfx_shape {
	enum vfx_shape_type type;
	enum prim_mode color;

	i32 d;
	rec_i32 rec;
};

struct vfx_blink {
	enum vfx_blink_type type;
	f32 period_min;
	f32 period_max;
};

struct vfx_txt {
	char buf[120];
	str8 str;
	v2 pivot;
	enum g_txt_style style;
};

struct vfx {
	u16 id;
	f32 timestamp;
	f32 duration;

	v2 p;
	v2 p_target;
	enum vfx_type type;
	struct vfx_blink blink;
	struct vfx_shape shape;
	struct vfx_txt txt;
};

struct vfxs {
	i16 free_list_cursor;
	u16 count;
	struct vfx *items;
	u8 *generations;
	u8 *free_list;
};
