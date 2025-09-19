#pragma once

#include "frame/frame.h"
#include "vfx/vfx-defs.h"

void vfxs_init(struct vfxs *pool, size count, struct alloc alloc);
void vfxs_upd(struct vfxs *pool, struct frame_info frame);
void vfxs_drw(struct vfxs *pool, struct frame_info frame);
struct vfx_handle vfxs_create(struct vfxs *pool, struct vfx vfx);
struct vfx *vfxs_get(struct vfxs *pool, struct vfx_handle handle);
b32 vfxs_remove(struct vfxs *pool, struct vfx_handle handle);
void vfxs_remove_all(struct vfxs *pool, struct vfx_handle handle);

void vfx_drw(struct vfx *vfx, f32 timestamp);
