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
	pool->count       = 0;
	pool->items       = arr_new_clr(pool->items, count, alloc);
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
			if(item->p_target.x != 0 && item->p_target.y != 0) {
				item->p.x = item->p.x + (item->p_target.x - item->p.x) / 10;
				item->p.y = item->p.y + (item->p_target.y - item->p.y) / 10;
				// f.y+=(f.ty-f.y)/10
			}

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
			vfx_drw(item, timestamp);
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

void
vfx_drw(struct vfx *vfx, f32 timestamp)
{
	if(vfx->id == 0) { return; }
	if(vfx->timestamp > timestamp) { return; }
	f32 cur    = timestamp - vfx->timestamp;
	f32 t      = cur / vfx->duration;
	f32 period = t > 0.5f ? vfx->blink.period_min : vfx->blink.period_max;
	f32 blink  = sin_f32(cur * (PI2_FLOAT / period));
	if(vfx->blink.type == VFX_BLINK_TRANSPARENT) {
		if(blink < 0.5f) { return; }
	}

	switch(vfx->type) {
	case VFX_TYPE_TXT: {
		g_txt_pivot(
			vfx->txt.str,
			vfx->p.x,
			vfx->p.y,
			vfx->txt.pivot,
			vfx->txt.style);
		debug_draw_cir(vfx->p.x, vfx->p.y, 3);

	} break;
	case VFX_TYPE_SPR: {
	} break;
	case VFX_TYPE_SHAPE: {
		enum prim_mode color = vfx->shape.color;
		if(vfx->blink.type == VFX_BLINK_COLOR) {
			if(blink > 0.5f) {
				color = color == PRIM_MODE_BLACK ? PRIM_MODE_WHITE : PRIM_MODE_BLACK;
			}
		}
		enum prim_mode old = g_color(color);
		switch(vfx->shape.type) {
		case VFX_SHAPE_TYPE_CIR: {
			g_cir(vfx->p.x, vfx->p.y, vfx->shape.d);
		} break;
		case VFX_SHAPE_TYPE_CIR_FILL: {
			g_cir_fill(vfx->p.x, vfx->p.y, vfx->shape.d);
		} break;
		case VFX_SHAPE_TYPE_REC: {
			g_rec(
				vfx->p.x + vfx->shape.rec.x,
				vfx->p.y + vfx->shape.rec.y,
				vfx->shape.rec.w,
				vfx->shape.rec.h);
		} break;
		case VFX_SHAPE_TYPE_REC_FILL: {
			g_rec_fill(
				vfx->p.x + vfx->shape.rec.x,
				vfx->p.y + vfx->shape.rec.y,
				vfx->shape.rec.w,
				vfx->shape.rec.h);
		} break;
		default: {
			dbg_sentinel("vfx");
		} break;
		}
		g_color(old);
	} break;
	default: {
	} break;
	}

error:
	return;
}
