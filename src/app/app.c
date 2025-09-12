#include "app.h"

#include "base/dbg.h"
#include "base/utils.h"
#include "engine/assets/assets.h"
#include "engine/audio/audio.h"
#include "engine/debug-draw/debug-draw.h"
#include "engine/input.h"
#include "lib/pd-utils.h"
#include "lib/rndm.h"
#include "sys/sys.h"

#include "scrns/scrn-loading/scrn-loading.h"

struct app APP;

void
app_init(usize mem_max)
{
	size permanent = MMEGABYTE(5.0);
	size transient = MMEGABYTE(7.0);
	size debug     = 0;
#if defined(DEBUG)
	debug = mem_max - permanent - transient;
#endif

	rndm_seed(NULL, sys_epoch_2000(NULL));
	struct app_mem mem = sys_init_mem(permanent, transient, debug, false);
	dbg_check(mem.is_initialized, "App", "Failed to initialize mem");

	struct pdxinfo pdxinfo = {0};
#if defined(TARGET_SIMULATOR) || defined(TARGET_PLAYDATE)
	{
		struct marena marena = {0};
		struct alloc alloc   = marena_allocator(&marena);
		marena_init(&marena, mem.permanent.buffer, MKILOBYTE(100));
		pdxinfo_parse(&pdxinfo, alloc);
		mclr(marena.buf, marena.buf_size);
	}
#endif

	log_info("App", "Init");
	log_info("App", "Version: %s", pdxinfo.version);

	marena_init(&APP.mem_permanent, mem.permanent.buffer, mem.permanent.size);
	marena_init(&APP.mem_transient, mem.transient.buffer, mem.transient.size);
	marena_init(&APP.mem_debug, mem.debug.buffer, mem.debug.size);

	APP.alloc_permanent = marena_allocator(&APP.mem_permanent);
	APP.alloc_transient = marena_allocator(&APP.mem_transient);
	APP.alloc_debug     = marena_allocator(&APP.mem_debug);

	mcpy(APP.version, pdxinfo.version, ARRLEN(APP.version));

	assets_init(mem.permanent.buffer, mem.permanent.size);

#if defined DEBUG
#if !defined(APP_DISABLE_DEBUG_DRAW)
	debug_draw_init(mem.debug.buffer, mem.debug.size);
#endif
#endif

#if 1
	app_set_scrn(&APP, SCRN_TYPE_LOADING);
#endif

error:
	return;
}

void
app_tick(f32 dt)
{
	TRACE_START(__func__);
	inp_upd();

	if(APP.upd) {
		APP.upd(&APP, dt);
	}

	aud_cmd_queue_commit();
	TRACE_END();
}

void
app_audio(i16 *lbuf, i16 *rbuf, i32 len)
{
	aud_do(lbuf, rbuf, len);
}

void
app_draw(void)
{
	TRACE_START(__func__);
#if defined(BACKEND_PD)
	sys_pd_update_rows(0, 239);
#endif

	if(APP.drw) {
		APP.drw(&APP);
	}

	TRACE_END();
}

void
app_close(void)
{
}

void
app_pause(void)
{
	log_info("App", "Pause");
}

void
app_resume(void)
{
	log_info("App", "Resume");
}

void
app_set_scrn(struct app *app, enum scrn_type scrn)
{
	sys_menu_clr();
	marena_reset(&app->mem_transient);

	switch(scrn) {
	case SCRN_TYPE_LOADING: {
		scrn_loading_ini(&APP);
	} break;
	case SCRN_TYPE_TITLE: {
	} break;
	case SCRN_TYPE_GAME: {
	} break;
	case SCRN_TYPE_TEST: {
	} break;
	default: {
		dbg_sentinel("scrn");
	} break;
	}

	app->scrn = scrn;
error:;
}
