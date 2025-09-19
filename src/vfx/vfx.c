#include "vfx.h"
#include "base/arr.h"
#include "base/mathfunc.h"
#include "engine/debug-draw/debug-draw.h"
#include "engine/gfx/gfx.h"
#include "globals/g-gfx.h"

void
vfxs_init(struct vfxs *pool, size count, struct alloc alloc)
{
	dbg_assert(count < U8_MAX);
	dbg_assert(count > 0);
	pool->items       = arr_new(pool->items, count, alloc);
	pool->free_list   = arr_new(pool->free_list, count, alloc);
	pool->generations = arr_new(pool->generations, count, alloc);

	for(size i = 0; i < count; i++) {
		arr_push(pool->generations, i + 1);
	}
	for(size i = 0; i < count; i++) {
		arr_push(pool->free_list, count - i);
	}
	for(size i = 0; i < count; i++) {
		arr_push(pool->items, (struct vfx){0});
	}

	pool->free_list_cursor = count - 1;
}

void
vfxs_upd(struct vfxs *pool, struct frame_info frame)
{
	f32 timestamp = frame.timestamp;
	for(size i = (size)arr_len(pool->items) - 1; i >= 0; --i) {
		struct vfx *item = pool->items + i;
		if(item->id != 0) {
			if(item->timestamp + item->duration <= timestamp) {
				vfxs_remove(pool, (struct vfx_handle){item->id});
			}
		}
	}
}

void
vfxs_drw(struct vfxs *pool, struct frame_info frame)
{
	f32 period    = 0.25f; // one full cycle every 0.25s
	f32 timestamp = frame.timestamp;
	for(size i = 0; i < (size)arr_len(pool->items); ++i) {
		struct vfx *item = pool->items + i;
		if(item->id != 0) {
			i32 x      = item->x;
			i32 y      = item->y;
			i32 w      = item->w;
			i32 h      = item->h;
			f32 cur    = timestamp - item->timestamp;
			f32 t      = cur / item->duration;
			i32 cx     = x + (item->w * 0.5f);
			i32 cy     = y + (item->h * 0.5f);
			f32 period = t > 0.5f ? 0.25f : 0.5f;
			f32 flash  = sin_f32(cur * (PI2_FLOAT / period));
			if(flash < 0.5f) {
				if(t > 0.5f) {
					g_color(PRIM_MODE_WHITE);
				} else {
					g_color(PRIM_MODE_BLACK);
				}
				g_color(PRIM_MODE_WHITE);
				g_cir_fill(cx, cy, w);
			}
			debug_draw_cir_fill(x + (w * 0.5f), y + (h * 0.5f), 3);
		}
	}
	g_spr_mode(SPR_MODE_COPY);
}

struct vfx_handle
vfxs_create(struct vfxs *pool, struct vfx vfx)
{
	dbg_assert(pool->free_list_cursor >= 0);
	u8 slot          = pool->free_list[pool->free_list_cursor--];
	struct vfx *item = &pool->items[slot];
	*item            = vfx;
	// Store the slot and generation in the id
	u16 id                = (((u16)slot) << 8) | ++pool->generations[slot];
	item->id              = id;
	struct vfx_handle res = {.id = item->id};
	pool->count++;
	return res;
}

struct vfx *
vfxs_get(struct vfxs *pool, struct vfx_handle handle)
{
	u16 slot           = (u16)((handle.id >> 8) & 0xff);
	struct vfx *result = &pool->items[slot];
	if(result->id == handle.id) return result;
	return pool->items;
}

b32
vfxs_remove(struct vfxs *pool, struct vfx_handle handle)
{

	struct vfx *item = vfxs_get(pool, handle);
	if(item->id == 0) return false;

	u16 slot                                  = (u16)((item->id >> 8) & 0xff);
	pool->free_list[++pool->free_list_cursor] = slot;
	item->id                                  = 0;
	pool->count--;
	return true;
}

void
vfxs_remove_all(struct vfxs *pool, struct vfx_handle handle)
{
	for(size i = (size)arr_len(pool->items) - 1; i >= 0; --i) {
		struct vfx *item = pool->items + i;
		vfxs_remove(pool, (struct vfx_handle){.id = item->id});
	}
}
