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
}

void
garden_ini(struct garden *garden, struct alloc alloc, f32 timestamp)
{
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
			u16 handle            = garden_flower_spawn(garden, column, type, timestamp);
			struct flower *flower = garden->flowers + handle;
			flower_iterations_max(flower, timestamp);
			cell->handles[j] = handle;
		}
	}
#endif
}

void
garden_seed_add(struct garden *garden, i32 column, enum block_type type, f32 timestamp)
{
	dbg_assert(type != BLOCK_TYPE_NONE);
	dbg_assert(column < (size)(ARRLEN(garden->cells)));
	struct garden_cell *cell = garden->cells + column;

	for(size i = 0; i < (size)ARRLEN(cell->handles); ++i) {
		u16 handle                 = cell->handles[i];
		struct flower *flower      = garden->flowers + handle;
		struct flower_rules *rules = FLOWERS_RULES + flower->type;
		if(flower->type == type && flower->iterations < rules->iterations_max) {
			flower_iterations_add(flower, 1, timestamp);
			return;
		} else {
			// replace one of the flowers with a new one?
		}
	}

	for(size i = 0; i < (size)ARRLEN(cell->handles); ++i) {
		u16 handle = cell->handles[i];
		if(handle == 0) {
			cell->handles[i] = garden_flower_spawn(garden, column, type, timestamp);
			return;
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

	for(size i = 1; i < (size)arr_len(garden->flowers); ++i) {
		struct flower *flower = garden->flowers + i;
		flower_upd(flower, frame);
	}
}

void
garden_drw(struct garden *garden, rec_i32 rec, enum game_theme theme)
{
	for(size i = 1; i < (size)arr_len(garden->flowers); ++i) {
		struct flower *flower = garden->flowers + i;
		flower_drw(flower, rec.x, rec.y);
	}
}

u16
garden_flower_spawn(struct garden *garden, i32 column, enum block_type type, f32 timestamp)
{
	i32 block_size        = 28;
	u16 res               = garden->flower_idx + 1;
	struct flower *flower = garden->flowers + res;
	flower->p.x           = (column * block_size) + (block_size * 0.5f) + rndm_range_i32(0, block_size * 0.25f, block_size * 0.25f);
	flower_type_set(flower, type, timestamp);
	garden->flower_idx = (garden->flower_idx + 1) % (arr_len(garden->flowers) - 1);
	return res;
}
