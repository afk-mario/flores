#pragma once

#include "app/app-defs.h"
#include "scrns/scrn-loading/scrn-loading.h"

static struct loading_state SCRN_LOADING_FUNCS[] = {
	{.load = scrn_loading_start},
	{.load = scrn_loading_db_init},
	{.load = scrn_loading_push_paths},
	{.load = scrn_loading_load_assets},
	{.load = scrn_loading_load_music_paths},
	{.load = scrn_loading_end},

};
