#include "scrn-game.h"

#include "app/app.h"
#include "base/marena.h"
#include "base/str.h"
#include "base/types.h"
#include "block/block-type.h"
#include "block/block.h"
#include "board/board.h"
#include "engine/debug-draw/debug-draw.h"
#include "engine/gfx/gfx.h"
#include "lib/layout.h"
#include "sys/sys.h"

#include "block/block-defs.h"
#include "board/board-defs.h"
#include "globals/g-gfx.h"
#include "globals/g-ui.h"
#include "scrns/scrn-game/scrn-game-defs.h"

#define GAME_WALL_W 3
#define GAME_HUD_W  SYS_DISPLAY_W - (GAME_WALL_W * 2) - (BLOCK_SIZE * (BOARD_COLUMNS * 2))

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

	board_ini(&scrn->board);
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
	struct board *board     = &scrn->board;

	g_drw_offset(0, 0);
	scrn_game_drw_bg(scrn);
	scrn_game_drw_walls(scrn);
	g_drw_offset(
		GAME_WALL_W,
		SYS_DISPLAY_H - GAME_WALL_W - (BLOCK_SIZE * board->rows));
	scrn_game_drw_game(scrn);

#if DEBUG
	if(app->debug_drw) {
		debug_draw_do(0, 0);
	}
#endif
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
		enum g_txt_style style = G_TXT_STYLE_HUD;
		i32 hud_w              = GAME_HUD_W;
		i32 margin             = 8;
		rec_i32 rec            = {
					   .x = (SYS_DISPLAY_W * 0.5f) - (hud_w * 0.5f),
					   .y = 0,
					   .w = GAME_HUD_W,
					   .h = SYS_DISPLAY_H,
        };
		struct ui_rec root = ui_rec_from_rec(rec.x, rec.y, rec.w, rec.h);

		g_color(PRIM_MODE_BLACK);
		g_rec_fill(rec.x, rec.y, rec.w, rec.h);
		debug_draw_ui_rec(root);
		ui_cut_top(&root, 13);
		debug_draw_ui_rec(root);
		debug_draw_ui_rec(root);
		g_color(PRIM_MODE_WHITE);
		{
			str8 str             = str8_lit("Next\npiece");
			v2_i32 size          = g_txt_size(str, style);
			struct ui_rec layout = ui_cut_top(&root, size.y);
			v2_i32 cntr          = ui_cntr_get(&layout);
			g_txt_pivot(str, cntr.x, cntr.y, (v2){0.5f, 0.5f}, style);
			debug_draw_ui_rec(layout);
		}
		ui_cut_top(&root, margin * 0.5f);
		{
			struct ui_rec layout = ui_cut_top(&root, BLOCK_SIZE);
			v2_i32 cntr          = ui_cntr_get(&layout);
			block_drw(&(struct block){.x = cntr.x - (BLOCK_SIZE * 0.5f), .y = cntr.y - (BLOCK_SIZE * 0.5f), .type = BLOCK_TYPE_A}, BLOCK_SIZE);
		}
		ui_cut_top(&root, margin);
		{
			str8 str             = str8_lit("Level\n01");
			v2_i32 size          = g_txt_size(str, style);
			struct ui_rec layout = ui_cut_top(&root, size.y);
			v2_i32 cntr          = ui_cntr_get(&layout);
			g_txt_pivot(str, cntr.x, cntr.y, (v2){0.5f, 0.5f}, style);
			debug_draw_ui_rec(layout);
		}
		ui_cut_top(&root, margin);
		{
			str8 str             = str8_lit("SCORE\n0000");
			v2_i32 size          = g_txt_size(str, style);
			struct ui_rec layout = ui_cut_top(&root, size.y);
			v2_i32 cntr          = ui_cntr_get(&layout);
			g_txt_pivot(str, cntr.x, cntr.y, (v2){0.5f, 0.5f}, style);
			debug_draw_ui_rec(layout);
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
	board_drw(&scrn->board);
}
