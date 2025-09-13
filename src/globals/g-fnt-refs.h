#pragma once

#include "base/str.h"
#include "base/types.h"
#include "base/utils.h"
#include "engine/assets/assets.h"

enum g_fnt_id {
	G_FNT_NONE,

	G_FNT_MONOGRAM,
	G_FNT_PICO8,
	G_FNT_THICKET,

	G_FNT_NUM_COUNT,
};

static const str8 G_FNT_PATH_MAP[G_FNT_NUM_COUNT] = {
	[G_FNT_MONOGRAM] = str8_lit_comp("assets/fonts/monogram-16.fnt"),
	[G_FNT_PICO8]    = str8_lit_comp("assets/fonts/pico8.fnt"),
	[G_FNT_THICKET]  = str8_lit_comp("assets/fonts/Thicket-12.fnt"),
};

static i32 G_FNT_IDS[G_FNT_NUM_COUNT];
static inline void
g_fnt_refs_id_set(enum g_fnt_id index, usize id)
{
	G_FNT_IDS[index] = id;
}

static inline void
g_fnt_refs_id_path_set(enum g_fnt_id index, str8 path)
{
	i32 id           = asset_fnt_get_id(path);
	G_FNT_IDS[index] = id;
}

static inline i32
g_fnt_refs_id_get(enum g_fnt_id index)
{
	return G_FNT_IDS[index];
}

static inline void
g_fnt_refs_ini(void)
{
	for(usize i = 0; i < ARRLEN(G_FNT_PATH_MAP); ++i) {
		str8 path = G_FNT_PATH_MAP[i];
		g_fnt_refs_id_path_set(i, path);
	}
}
