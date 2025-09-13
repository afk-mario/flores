#pragma once

#include "base/marena.h"
#include "scrns/scrn-game/scrn-game-defs.h"
#include "scrns/scrn-loading/scrn-loading-defs.h"
#include "scrns/scrn-over/scrn-over-defs.h"
#include "scrns/scrn-type.h"

struct app {
	char version[20];
	b32 debug_drw;
	enum scrn_type scrn;

	struct alloc alloc_permanent;
	struct alloc alloc_transient;
	struct alloc alloc_debug;

	struct marena mem_permanent;
	struct marena mem_transient;
	struct marena mem_debug;

	void (*upd)(struct app *app, f32 dt);
	void (*drw)(struct app *app);

	struct scrn_loading scrn_loading;
	struct scrn_game scrn_game;
	struct scrn_over scrn_over;
};
