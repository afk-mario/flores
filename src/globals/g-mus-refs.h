#pragma once

#include "base/utils.h"
#include "engine/assets/asset-db.h"

enum g_mus_id {
	G_MUS_NONE,

	G_MUS_NUM_COUNT,
};

static const str8 G_MUS_REFS_PATH_MAP[G_MUS_NUM_COUNT] = {0};

static struct asset_handle G_MUS_REFS_HANDLES[G_MUS_NUM_COUNT];

static inline void
g_mus_refs_handle_set(enum g_mus_id id, struct str8 path)
{
	struct asset_handle handle = {
		.type      = ASSET_TYPE_SOUND,
		.path_hash = hash_string(path),
	};
	G_MUS_REFS_HANDLES[id] = handle;
}

static inline struct asset_handle
g_mus_refs_handle_get(enum g_mus_id id)
{
	return G_MUS_REFS_HANDLES[id];
}

static inline str8
g_mus_refs_path_get(enum g_mus_id id)
{
	return G_MUS_REFS_PATH_MAP[id];
}

void
g_mus_refs_ini(void)
{
	for(usize i = 0; i < ARRLEN(G_MUS_REFS_PATH_MAP); ++i) {
		str8 path = G_MUS_REFS_PATH_MAP[i];
		g_mus_refs_handle_set(i, path);
	}
}
