#include "scrn-loading.h"
#include "app/app-data.h"
#include "app/app.h"
#include "base/trace.h"
#include "base/utils.h"
#include "engine/assets/assets.h"
#include "engine/gfx/gfx-defs.h"
#include "globals/g-fnt-refs.h"
#include "globals/g-gfx.h"
#include "globals/g-mus-refs.h"
#include "globals/g-sfx-refs.h"
#include "globals/g-tex-refs.h"
#include "globals/g-ui.h"
#include "scrns/scrn-loading/scrn-loading-data.h"
#include "sys/sys.h"

void
scrn_loading_ini(struct app *app)
{
	struct scrn_loading *scrn = &app->scrn_loading;
	struct gfx_ctx ctx        = gfx_ctx_display();
	struct alloc alloc        = app->alloc_transient;
	tex_clr(ctx.dst, GFX_COL_BLACK);

	{
		scrn->loadings = SCRN_LOADING_FUNCS;
		for(usize i = 0; i < scrn->loading_state.total; ++i) {
			scrn->loadings[i].is_finished = false;
			scrn->loadings[i].total       = 0;
			scrn->loadings[i].loaded      = 0;
			scrn->loadings[i].loaded_sub  = 0;
		}
	}

	{
		scrn->loading_state.total       = ARRLEN(SCRN_LOADING_FUNCS);
		scrn->loading_state.loaded      = 0;
		scrn->loading_state.is_finished = false;
		scrn->loading_state.loaded_sub  = 0;
	}

	app->drw = scrn_loading_drw;
	app->upd = scrn_loading_upd;
}
void
scrn_loading_upd(struct app *app, f32 dt)
{
	struct scrn_loading *scrn = &app->scrn_loading;
	TRACE_START(__func__);

	scrn->timestamp = scrn->timestamp + dt;
	f32 timestamp   = scrn->timestamp;

	if(!scrn->loading_state.is_finished) {
		b32 is_finished = scrn_loading_load(
			&scrn->loading_state,
			sys_seconds(),
			APP_LOAD_MAX_TIME,
			app);

		if(is_finished) {
			usize left  = marena_size_rem(&ASSETS.marena);
			usize total = ASSETS.marena.buf_size;
			usize used  = total - left;
			log_info(
				"Assets",
				"MEM: used:%$$u left:%$$u total:%$$u",
				(uint)used,
				(uint)left,
				(uint)total);
		}
	} else {
#if DEBUG
		app_set_scrn(app, SCRN_TYPE_GAME);
#else
		app_set_scrn(app, SCRN_TYPE_TITLE);
#endif
	}

	TRACE_END();
}

void
scrn_loading_drw(struct app *app)
{
}

b32
scrn_loading_start(struct loading_state *state, f32 time_start, f32 time_max, void *args)
{
	return true;
}

b32
scrn_loading_end(struct loading_state *state, f32 time_start, f32 time_max, void *args)
{
	struct app *app          = (struct app *)args;
	const struct gfx_ctx ctx = gfx_ctx_display();

	g_ui_init();
	g_tex_refs_ini();
	g_fnt_refs_ini();
	g_mus_refs_ini();
	g_drw_ctx(ctx);

	{
		enum g_txt_style style = G_TXT_STYLE_DEBUG;
		enum g_fnt_id id       = g_fnt_refs_id_get(G_FNT_MONOGRAM);
		if(id > 0) {
			struct fnt fnt = asset_fnt(id);
			dbg_assert(fnt.t.px);
			struct txt_style txt_style = {
				.fnt      = fnt,
				.tracking = 0,
				.leading  = 1,
				.mode     = SPR_MODE_WHITE,
			};
			g_txt_style(style, txt_style);
		}
	}

	{
		enum g_txt_style style = G_TXT_STYLE_DEBUG_SML;
		enum g_fnt_id id       = g_fnt_refs_id_get(G_FNT_PICO8);
		if(id > 0) {
			struct fnt fnt = asset_fnt(id);
			dbg_assert(fnt.t.px);
			struct txt_style txt_style = {
				.fnt      = fnt,
				.tracking = 0,
				.leading  = 1,
				.mode     = SPR_MODE_WHITE,
			};
			g_txt_style(style, txt_style);
		}
	}

#if DEBUG
	app_set_scrn(app, SCRN_TYPE_GAME);
#else
	app_set_scrn(app, SCRN_TYPE_TITLE);
#endif
	return true;
}

b32
scrn_loading_load(struct loading_state *state, f32 time_start, f32 time_max, void *args)
{
	struct loading_state *loadings = SCRN_LOADING_FUNCS;
	if(!state->is_finished) {
		f32 time_start = sys_seconds();
		f32 delta      = 0;
		while(delta < time_max && state->loaded < state->total) {
			struct loading_state *loading = loadings + state->loaded;
			loading->is_finished =
				loading->load(loading, time_start, time_max, args);
			if(!loading->is_finished) {
				return false;
			}
			state->loaded++;
			delta = sys_seconds() - time_start;
		}
		state->is_finished = state->loaded == state->total;
	}

	return state->is_finished;
}

b32
scrn_loading_db_init(struct loading_state *state, f32 time_start, f32 time_max, void *args)
{
	struct app *app            = (struct app *)args;
	i32 paths_count            = 100;
	i32 textures_count         = 100;
	i32 animation_clips_count  = 100;
	i32 animation_slices_count = 100;
	i32 snds_count             = G_SFX_NUM_COUNT;
	i32 fonts_count            = G_FNT_NUM_COUNT;
	i32 bets_count             = 0;

	asset_db_init(
		&ASSETS.db,
		paths_count,
		textures_count,
		animation_clips_count,
		animation_slices_count,
		fonts_count,
		snds_count,
		bets_count,
		ASSETS.alloc);

	state->is_finished = true;
	return state->is_finished;
}

static inline b32
push_paths_from(
	struct loading_state *state,
	const str8 *arr,
	usize count,
	f32 time_start,
	f32 time_max)
{
	f32 delta = sys_seconds() - time_start;
	while(delta < time_max && state->loaded_sub < count) {
		if(arr[state->loaded_sub].size > 0) {
			str8 path = arr[state->loaded_sub];
			asset_db_path_push(&ASSETS.db, path);
		}
		state->loaded_sub++;
		delta = sys_seconds() - time_start;
	}
	b32 finished = state->loaded_sub == count;
	if(finished) {
		state->loaded_sub = 0;
		state->loaded++;
	}
	return finished;
}

b32
scrn_loading_push_paths(struct loading_state *state, f32 time_start, f32 time_max, void *args)
{
	state->total         = 3;
	f32 delta            = sys_seconds() - time_start;
	usize load_index_end = state->total;
	log_info("Loading", "pushing path: %u->%u %f", (uint)state->loaded, (uint)state->total, (double)time_start);

	b32 is_finished = true;
	while(is_finished == true && delta < time_max && state->loaded < state->total) {
		switch(state->loaded) {
		case 0: {
			is_finished = push_paths_from(
				state,
				G_TEX_PATH_MAP,
				ARRLEN(G_TEX_PATH_MAP),
				time_start,
				time_max);
		} break;
		case 1: {
			is_finished = push_paths_from(
				state,
				G_FNT_PATH_MAP,
				ARRLEN(G_FNT_PATH_MAP),
				time_start,
				time_max);
		} break;
		case 2: {
			is_finished = push_paths_from(
				state,
				G_SFX_PATH_MAP,
				ARRLEN(G_SFX_PATH_MAP),
				time_start,
				time_max);
		} break;
		default: {
			is_finished = false;
		} break;
		}
		delta = sys_seconds() - time_start;
	}

	b32 res = state->loaded == state->total;
	return res;
}
