#include "scrn-title.h"
#include "app/app-data.h"
#include "app/app.h"
#include "base/arr.h"
#include "base/mathfunc.h"
#include "base/rec.h"
#include "base/str.h"
#include "engine/assets/assets.h"
#include "garden/flower.h"
#include "garden/garden.h"
#include "globals/g-gfx.h"
#include "globals/g-ui.h"
#include "sys/sys.h"

#define SCRN_TITLE_TRANSITION_01_DURATION 0.5f
#define SCRN_TITLE_TRANSITION_02_DURATION 2.0f

void
scrn_title_ini(struct app *app)
{
	app->upd                = scrn_title_upd;
	app->drw                = scrn_title_drw;
	struct scrn_title *scrn = &app->scrn_title;
	scrn->timestamp         = 0;
	scrn->transition_in_t   = 0;
	scrn->transition_in     = true;
	{
		struct alloc alloc    = app->alloc_transient;
		scrn->frame.timestamp = 0;
		usize frame_mem_size  = MKILOBYTE(50);
		void *frame_mem       = alloc.allocf(alloc.ctx, frame_mem_size);
		marena_init(&scrn->frame.marena, frame_mem, frame_mem_size);
		scrn->frame.alloc = marena_allocator(&scrn->frame.marena);
	}
	{
		struct alloc alloc    = app->alloc_transient;
		struct garden *garden = &scrn->garden;
		struct rec_i32 rec    = {0, 0, SYS_DISPLAY_W, SYS_DISPLAY_H};
		garden_load(garden, (v2_i32){rec.w, rec.h}, ASSETS.alloc);
		garden_ini(garden, alloc, scrn->frame);
	}
	{
		struct frame_info frame = scrn->frame;
		struct garden *garden   = &scrn->garden;
		for(size i = 1; i < (size)arr_len(garden->flowers); ++i) {
			enum block_type type  = rndm_range_i32(NULL, BLOCK_TYPE_A, BLOCK_TYPE_F);
			u16 handle            = garden_flower_spawn(garden, 0, type, frame);
			struct flower *flower = garden->flowers + handle;
			flower->p.x           = rndm_range_i32(NULL, 0, SYS_DISPLAY_W);
			// flower_iterations_max(flower, frame);
			flower_water_add(flower, rndm_range_i32(NULL, 20, 100), frame);
		}
	}
}

void
scrn_title_upd(struct app *app, f32 dt)
{
	struct scrn_title *scrn = &app->scrn_title;
	f32 time_scale          = 1.0f;
	scrn->frame.dt          = dt * time_scale;
	scrn->frame.timestamp   = scrn->frame.timestamp + scrn->frame.dt;
	f32 timestamp           = scrn->frame.timestamp;

	if(scrn->transition_in) {
		if(scrn->transition_in_t + 0.5f < scrn->frame.timestamp) {
			scrn->transition_in = false;
		}
		return;
	}

	if(scrn->timestamp == 0) {
		if(g_ui_just_pressed_any()) {
			scrn->timestamp = timestamp;
		}
	} else if((scrn->timestamp + SCRN_TITLE_TRANSITION_01_DURATION + SCRN_TITLE_TRANSITION_02_DURATION) < timestamp) {
		app_set_scrn(app, SCRN_TYPE_GAME);
	}

#if 1
	struct garden *garden = &scrn->garden;
	garden_upd(garden, scrn->frame);
#endif
}

void
scrn_title_drw(struct app *app)
{
	struct scrn_title *scrn = &app->scrn_title;
#if 1
	g_drw_offset(0, 0);
	{
		rec_i32 rec           = {0, 0, SYS_DISPLAY_W, SYS_DISPLAY_H};
		struct garden *garden = &scrn->garden;
		garden_drw(garden, rec, GAME_THEME_ABSTRACT);
	}
	{
		f32 alpha = 0.5f;
		if(scrn->timestamp > 0) {
			f32 time_past = scrn->frame.timestamp - scrn->timestamp;
			f32 t         = clamp_f32(time_past / SCRN_TITLE_TRANSITION_01_DURATION, 0.0f, 1.0f);
			alpha         = lerp(0.5f, 1.0, t);
		}
		g_pat(gfx_pattern_bayer_4x4(16 * alpha));
		g_color(PRIM_MODE_BLACK);
		g_rec_fill(0, 0, SYS_DISPLAY_W, SYS_DISPLAY_H);
		g_pat(gfx_pattern_100());
		g_color(PRIM_MODE_BLACK);
		g_spr_mode(SPR_MODE_COPY);
	}
	{
		rec_i32 rec = {0, 0, SYS_DISPLAY_W * 0.5f, 50};
		rec         = rec_i32_anchor(APP_SCRN_REC, rec, (v2){0.5f, 0.5f});
		g_rec_fill(REC_UNPACK(rec));
	}
#endif

	{
		f32 alpha = 1.0f;
		if(scrn->timestamp > 0) {
			if(scrn->timestamp + SCRN_TITLE_TRANSITION_01_DURATION < scrn->frame.timestamp) {
				f32 time_past = scrn->frame.timestamp - (scrn->timestamp + SCRN_TITLE_TRANSITION_01_DURATION);
				f32 t         = clamp_f32(time_past / SCRN_TITLE_TRANSITION_02_DURATION, 0.0f, 1.0f);
				alpha         = 1.0f - lerp(0.0f, 1.0, t);
			}
		}
		g_pat(gfx_pattern_bayer_4x4(16 * alpha));

		{
			enum g_txt_style style = G_TXT_STYLE_STORY_TITLES;
			str8 str               = str8_lit("Flores");

			g_txt_pivot(str, SYS_DISPLAY_W * 0.5f, SYS_DISPLAY_H * 0.5f, (v2){0.5f, 1.0f}, style);
		}
		{
			enum g_txt_style style = G_TXT_STYLE_STORY;
			str8 str               = str8_lit("By Mario & Fernanda");

			g_txt_pivot(str, SYS_DISPLAY_W * 0.5f, SYS_DISPLAY_H * 0.5f + 16, (v2){0.5f, 1.0f}, style);
		}
		g_pat(gfx_pattern_100());
	}

	if(scrn->transition_in) {
		f32 time_past = scrn->frame.timestamp - scrn->transition_in_t;
		f32 t         = clamp_f32(time_past / 1.0f, 0.0f, 1.0f);
		f32 alpha     = 1.0f - t;
		g_pat(gfx_pattern_bayer_4x4(16 * alpha));
		g_color(PRIM_MODE_BLACK);
		g_rec_fill(0, 0, SYS_DISPLAY_W, SYS_DISPLAY_H);
		g_pat(gfx_pattern_100());
	}
}
