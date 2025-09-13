#include "scrn-game.h"
#include "app/app.h"
#include "base/marena.h"
#include "base/str.h"
#include "base/types.h"
#include "engine/gfx/gfx.h"
#include "globals/g-gfx.h"
#include "globals/g-ui.h"
#include "scrns/scrn-game/scrn-game-defs.h"
#include "sys/sys.h"

#define GAME_WALL_W        3
#define GAME_BLOCK_S       28
#define GAME_BOARD_COLUMNS 6
#define GAME_BOARD_ROWS    8
#define GAME_HUD_W         SYS_DISPLAY_W - GAME_WALL_W - GAME_WALL_W - (GAME_BLOCK_S * 6 * 2)

static inline void scrn_game_drw_bg(struct scrn_game *scrn);
static inline void scrn_game_drw_walls(struct scrn_game *scrn);
static inline void scrn_game_drw_game(struct scrn_game *scrn);
static inline void scrn_game_drw_block(struct scrn_game *scrn, struct block *block);

void
scrn_game_ini(struct app *app)
{
	log_info("Game", "init");

	app->upd               = scrn_game_upd;
	app->drw               = scrn_game_drw;
	struct scrn_game *scrn = &app->scrn_game;
	struct alloc alloc     = app->alloc_transient;

	{
		scrn->frame.timestamp = 0;
		usize frame_mem_size  = MKILOBYTE(50);
		void *frame_mem       = alloc.allocf(alloc.ctx, frame_mem_size);
		marena_init(&scrn->frame.marena, frame_mem, frame_mem_size);
		scrn->frame.alloc = marena_allocator(&scrn->frame.marena);
	}
}

void
scrn_game_upd(struct app *app, f32 dt)
{
	struct scrn_game *scrn   = &app->scrn_game;
	struct frame_info *frame = &scrn->frame;
	frame->timestamp         = frame->timestamp + dt;
	frame->dt                = dt;
	f32 timestamp            = frame->timestamp;
	marena_reset(&frame->marena);

	if(g_ui_just_pressed_any()) {
		app_set_scrn(app, SCRN_TYPE_TITLE);
	}
}

void
scrn_game_drw(struct app *app)
{
	enum g_txt_style style  = G_TXT_STYLE_DEBUG;
	struct scrn_game *scrn  = &app->scrn_game;
	struct frame_info frame = scrn->frame;

	g_drw_offset(0, 0);
	scrn_game_drw_bg(scrn);
	scrn_game_drw_walls(scrn);
	g_drw_offset(
		GAME_WALL_W,
		SYS_DISPLAY_H - GAME_WALL_W - (GAME_BLOCK_S * GAME_BOARD_ROWS));
	scrn_game_drw_game(scrn);
}

static inline void
scrn_game_drw_bg(struct scrn_game *scrn)
{
	g_color(PRIM_MODE_BLACK);
	g_color(PRIM_MODE_WHITE);
	// g_pat(gfx_pattern_25());
	g_rec_fill(0, 0, SYS_DISPLAY_W, SYS_DISPLAY_H);
	g_pat(gfx_pattern_white());
}

static inline void
scrn_game_drw_walls(struct scrn_game *scrn)
{
	i32 wall_s = GAME_WALL_W;

	{
		g_color(PRIM_MODE_BLACK);
		g_rec_fill(0, 0, wall_s, SYS_DISPLAY_H);                      // left
		g_rec_fill(SYS_DISPLAY_W - wall_s, 0, wall_s, SYS_DISPLAY_H); // right
		g_rec_fill(0, SYS_DISPLAY_H - wall_s, SYS_DISPLAY_W, wall_s); // bottom
	}
	{
		i32 hud_s = GAME_HUD_W;
		i32 hud_x = (SYS_DISPLAY_W * 0.5f) - (hud_s * 0.5f);
		g_rec_fill(
			hud_x,
			0,
			hud_s,
			SYS_DISPLAY_H); // HUD
		{
			i32 x                  = SYS_DISPLAY_W * 0.5f;
			enum g_txt_style style = G_TXT_STYLE_HUD;
			str8 str               = str8_lit("Next\npiece");
			g_color(PRIM_MODE_WHITE);
			g_txt_pivot(str, x, 15, (v2){0.5f, 0.5f}, style);
		}
		{
			i32 x                  = SYS_DISPLAY_W * 0.5f;
			enum g_txt_style style = G_TXT_STYLE_HUD;
			str8 str               = str8_lit("Level\n01");
			g_color(PRIM_MODE_WHITE);
			g_txt_pivot(str, x, 50, (v2){0.5f, 0.5f}, style);
		}
		{
			i32 x                  = SYS_DISPLAY_W * 0.5f;
			enum g_txt_style style = G_TXT_STYLE_HUD;
			str8 str               = str8_lit("SCORE\n0000");
			g_color(PRIM_MODE_WHITE);
			g_txt_pivot(str, x, 80, (v2){0.5f, 0.5f}, style);
		}
	}
	{
		g_color(PRIM_MODE_WHITE);
		i32 space = 2;
		i32 x1    = wall_s - (space + 1);
		i32 y1    = 0;
		i32 x2    = SYS_DISPLAY_W - wall_s + space;
		i32 y2    = SYS_DISPLAY_H - wall_s + space;
		g_lin(x1, y1, x1, y2); // Left
		g_lin(x2, y1, x2, y2); // Right
		g_lin(x1, y2, x2, y2); // bottom
	}
}

static inline void
scrn_game_drw_game(struct scrn_game *scrn)
{
	i32 w = GAME_BLOCK_S;
	i32 h = GAME_BLOCK_S;
	i32 r = GAME_BOARD_ROWS;
	i32 c = GAME_BOARD_COLUMNS;
	for(size i = 0; i < r * c; ++i) {
		struct block block = {.x = i % c, .y = i / c};
		scrn_game_drw_block(scrn, &block);
	}
}

static inline void
scrn_game_drw_block(struct scrn_game *scrn, struct block *block)
{
	i32 w     = GAME_BLOCK_S;
	i32 h     = GAME_BLOCK_S;
	i32 x     = block->x * GAME_BLOCK_S;
	i32 y     = block->y * GAME_BLOCK_S;
	i32 cx    = x + (w * 0.5f);
	i32 cy    = y + (h * 0.5f);
	rec_i32 r = {.x = x + 1, .y = y + 1, .w = w - 2, .h = h - 2};
	g_color(PRIM_MODE_BLACK);
	g_rec(r.x, r.y, r.w, r.h);
	g_pat(gfx_pattern_white());
	g_color(PRIM_MODE_WHITE);
	g_rec(r.x, r.y, 1, 1);
	g_rec(r.x + r.w - 1, r.y, 1, 1);
	g_rec(r.x, r.y + r.h - 1, 1, 1);
	g_rec(r.x + r.w - 1, r.y + r.h - 1, 1, 1);
	g_color(PRIM_MODE_BLACK);
	g_pat(gfx_pattern_100());
	g_rec_fill(r.x + 2, r.y + 2, r.w - 4, r.h - 4);
	g_color(PRIM_MODE_WHITE);
	g_cir_fill(cx, cy, w / 2);
}
