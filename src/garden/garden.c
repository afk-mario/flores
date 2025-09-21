#include "garden.h"
#include "base/arr.h"
#include "base/dbg.h"
#include "base/marena.h"
#include "base/mathfunc.h"
#include "base/mem.h"
#include "base/rec.h"
#include "base/str.h"
#include "base/types.h"
#include "base/utils.h"
#include "engine/debug-draw/debug-draw.h"
#include "engine/gfx/gfx-defs.h"
#include "engine/gfx/gfx.h"
#include "engine/input.h"
#include "garden/flower-data.h"
#include "garden/flower.h"
#include "garden/garden-data.h"
#include "garden/garden-defs.h"
#include "globals/g-gfx.h"
#include "lib/rndm.h"
#include "lib/tex/tex.h"
#include "sys/sys.h"

void
garden_load(struct garden *garden, v2_i32 dims, struct alloc alloc)
{
	if(garden->is_loaded) { return; }
	garden->textures = arr_new(garden->textures, GARDEN_FLOWERS_MAX, alloc);
	for(size i = 0; i < (size)arr_cap(garden->textures); ++i) {
		struct tex t = tex_create(dims.x, dims.y, alloc);
		tex_clr(t, GFX_COL_CLEAR);
		arr_push(garden->textures, t);
	}
	garden->is_loaded = true;
}

void
garden_ini(struct garden *garden, struct alloc alloc, struct frame_info frame)
{
	dbg_assert(garden->is_loaded);
	garden->flower_idx = 0;
	mclr_array(garden->cells);
	garden->flowers = arr_new(garden->flowers, GARDEN_FLOWERS_MAX + 1, alloc);
	arr_push(garden->flowers, (struct flower){0});
	for(size i = 1; i < (size)arr_cap(garden->flowers); ++i) {
		arr_push(garden->flowers, (struct flower){0});
	}
	for(size i = 1; i < (size)arr_cap(garden->flowers); ++i) {
		struct flower *flower = garden->flowers + i;
		flower_ini(flower, alloc, garden->textures[i]);
	}
#if 0
	for(size i = 0; i < (size)ARRLEN(garden->cells); ++i) {
		i32 column               = i;
		struct garden_cell *cell = garden->cells + i;
		for(size j = 0; j < (size)ARRLEN(cell->handles); ++j) {
			enum block_type type  = rndm_range_i32(NULL, BLOCK_TYPE_A, BLOCK_TYPE_F);
			u16 handle            = garden_flower_spawn(garden, column, type, frame);
			struct flower *flower = garden->flowers + handle;
			// flower_iterations_max(flower, frame);
			flower_water_add(flower, rndm_range_i32(NULL, 20, 100), frame);
			cell->handles[j] = handle;
		}
	}
#endif

#if 0
	enum block_type type  = BLOCK_TYPE_F;
	u16 handle            = garden_flower_spawn(garden, 2, type, frame);
	struct flower *flower = garden->flowers + handle;
	flower_iterations_max(flower, frame);
#endif

	{
		garden->time.day       = 1;
		garden->time.hour      = 7;
		garden->time.is_day    = true;
		garden->time.timestamp = frame.timestamp;
	}
}

b32
garden_seed_add(struct garden *garden, i32 column, enum block_type type, struct frame_info frame)
{
	dbg_assert(type != BLOCK_TYPE_NONE);
	dbg_assert(column < (size)(ARRLEN(garden->cells)));
	b32 res                  = false;
	struct garden_cell *cell = garden->cells + column;

	// TODO: Check if flower is full and reset
	for(size i = 0; i < (size)ARRLEN(cell->handles); ++i) {
		u16 handle = cell->handles[i];
		if(handle == 0) {
			cell->handles[i] = garden_flower_spawn(garden, column, type, frame);
			return true;
		}
	}

	return false;
}

void
garden_time_adv(struct garden *garden, struct frame_info frame)
{
	struct garden_time *tim = &garden->time;
	tim->timestamp          = frame.timestamp;
	tim->hour++;
	if(tim->hour > 12) {
		tim->hour   = 0;
		tim->is_day = !tim->is_day;
		if(tim->is_day) {
			tim->day++;
		}
	}
}

void
garden_water_add(struct garden *garden, i32 column, i32 value, struct frame_info frame)
{
	dbg_assert(column < (size)(ARRLEN(garden->cells)));
	struct garden_cell *cell = garden->cells + column;

	// TODO: Check if flower is full and reset
	for(size i = 0; i < (size)ARRLEN(cell->handles); ++i) {
		u16 handle = cell->handles[i];
		if(handle != 0) {
			struct flower *flower = garden->flowers + handle;
			flower_water_add(flower, value, frame);
		}
	}
}

void
garden_upd(struct garden *garden, struct frame_info frame)
{
	f32 timestamp = frame.timestamp;
	if(inp_key_just_pressed('m')) {
		for(size i = 0; i < (size)arr_len(garden->flowers); ++i) {
		}
	}
	if(inp_key_just_pressed('n')) {
		for(size i = 0; i < (size)arr_len(garden->flowers); ++i) {
		}
	}

	struct flower *flower = garden->flowers + (garden->slicing.index + 1);
	flower_upd(flower, frame);
	garden->slicing.index = (garden->slicing.index + 1) % (arr_len(garden->flowers) - 1);
	if((garden->time.timestamp + 30.0f) < frame.timestamp) {
		garden_time_adv(garden, frame);
	}
}

void
garden_drw(struct garden *garden, rec_i32 rec, enum game_theme theme)
{
	struct garden_time *tim = &garden->time;
	i32 is_day              = tim->is_day;
	f32 alpha               = 1.0f;
	is_day                  = true;
	if(tim->hour < 6) {
		alpha = clamp_f32(lerp_inv(0, 4, tim->hour), 0.0f, 1.0f);
	} else {
		alpha = 1.0f - clamp_f32(lerp_inv(8, 12, tim->hour), 0.0f, 1.0f);
	}
	alpha = max_f32(alpha, 0.2f);
	alpha = 1.0f;
	if(is_day) {
		g_color(PRIM_MODE_WHITE);
	} else {
		g_color(PRIM_MODE_BLACK);
	}
	g_rec_fill(REC_UNPACK(rec));
	g_pat(gfx_pattern_bayer_4x4(alpha * 16));
	for(size i = 1; i < (size)arr_len(garden->flowers); ++i) {
		struct flower *flower = garden->flowers + i;
		flower_drw(flower, rec.x, rec.y, is_day);
	}
	g_pat(gfx_pattern_100());
}

u16
garden_flower_spawn(struct garden *garden, i32 column, enum block_type type, struct frame_info frame)
{
	i32 block_size        = 28;
	u16 res               = garden->flower_idx + 1;
	struct flower *flower = garden->flowers + res;
	flower->p.x           = (column * block_size) + (block_size * 0.5f) + rndm_range_i32(0, block_size * 0.25f, block_size * 0.25f);
	flower_type_set(flower, type, frame);
	garden->flower_idx = (garden->flower_idx + 1) % (arr_len(garden->flowers) - 1);
	return res;
}
