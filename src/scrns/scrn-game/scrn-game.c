#include "scrn-game.h"

#include "app/app-data.h"
#include "app/app.h"
#include "base/marena.h"
#include "base/mathfunc.h"
#include "base/rec.h"
#include "base/str.h"
#include "base/types.h"
#include "base/v2.h"
#include "block/block-type.h"
#include "block/block.h"
#include "board/board.h"
#include "engine/assets/assets.h"
#include "engine/debug-draw/debug-draw.h"
#include "engine/gfx/gfx.h"
#include "engine/input.h"
#include "garden/garden.h"
#include "globals/g-dbg.h"
#include "globals/g-sfx-refs.h"
#include "globals/g-tex-refs.h"
#include "globals/g-ui-actions.h"
#include "globals/g-ui.h"
#include "lib/rndm.h"
#include "piece/piece.h"
#include "scrns/scrn-game/scrn-game-data.h"
#include "sys/sys.h"

#include "block/block-defs.h"
#include "board/board-defs.h"
#include "globals/g-gfx.h"
#include "scrns/scrn-game/scrn-game-defs.h"
#include "vfx/vfx.h"

static inline v2_i32 scrn_game_mouse_to_board(struct scrn_game *scrn);
static inline void scrn_game_drw_bg(struct scrn_game *scrn);
static inline void scrn_game_drw_walls(struct scrn_game *scrn);
static inline void scrn_game_drw_hud(struct scrn_game *scrn);
static inline void scrn_game_drw_game(struct scrn_game *scrn);
static inline void scrn_game_drw_block(struct scrn_game *scrn, struct block *block);
static void scrn_game_piece_spawn_rndm(struct scrn_game *scrn);
static void scrn_game_piece_set_blocks(struct scrn_game *scrn);
static inline void scrn_game_matches_vfx(struct scrn_game *scrn);

static inline void scrn_game_play_upd(struct scrn_game *scrn);
static inline void scrn_game_falling_upd(struct scrn_game *scrn);
static inline void scrn_game_pause_upd(struct scrn_game *scrn);
static inline void scrn_game_pause_drw(struct scrn_game *scrn);
static inline void scrn_game_editor_upd(struct scrn_game *scrn);
static inline void scrn_game_collapse_upd(struct scrn_game *scrn);
static inline void scrn_game_over_upd(struct scrn_game *scrn);
static inline void scrn_game_over_drw(struct scrn_game *scrn);
static inline void scrn_game_exit(struct scrn_game *scrn, enum scrn_type value);

static inline i32 scrn_game_matches(struct scrn_game *scrn);
static inline rec_i32 scrn_game_get_board_rec(struct scrn_game *scrn);
static inline rec_i32 scrn_game_get_garden_rec(struct scrn_game *scrn);
static inline void scrn_game_vfx_score(struct scrn_game *scrn, i32 x, i32 y, u32 value, u32 chain);

#define GAME_WALL_W 3
#define GAME_HUD_W  SYS_DISPLAY_W - (GAME_WALL_W * 2) - (BLOCK_SIZE * (BOARD_COLUMNS * 2)) - 2
// #define BOARD_FULL

#define BOARD_PRESET    1
#define GAME_TIME_SCALE 1.0f

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

	{
		vfxs_init(&scrn->matches_vfx, BOARD_COLUMNS * BOARD_ROWS, alloc);
		vfxs_init(&scrn->vfxs, 20, alloc);
	}

	f32 timestamp         = scrn->frame.timestamp;
	struct board *board   = &scrn->board;
	struct garden *garden = &scrn->garden;
	board_ini(board, timestamp);
	{
		struct rec_i32 rec = scrn_game_get_garden_rec(scrn);
		garden_load(&app->scrn_game.garden, (v2_i32){rec.w, rec.h}, ASSETS.alloc);
		garden_ini(garden, alloc, timestamp);
	}
	scrn->editor.type = BLOCK_TYPE_A;
	scrn->piece       = (struct piece){0};
	scrn->theme       = GAME_THEME;
	scrn->exit_to     = SCRN_TYPE_NONE;
	scrn->score       = 0;
	scrn->score_ui    = 0;
	scrn->chain       = 0;

#if defined(BOARD_FULL)
	enum g_tex_id ref = GAME_THEME_REFS[scrn->theme];
	i32 id            = g_tex_refs_id_get(ref);
	struct tex t      = asset_tex(id);
	i32 max_type      = (t.w / board->block_size) - 1;
	i32 block_count   = BOARD_COLUMNS * BOARD_ROWS;
	for(size i = 0; i < block_count; ++i) {
		struct block block = {
			.type = rndm_range_i32(NULL, BLOCK_TYPE_NONE + 1, max_type),
		};
		v2_i32 p = board_idx_to_coords(board, i);
		board_block_set(board, block, p.x, p.y);
	}
#else
	if(BOARD_PRESET > 0) {
		u8 *preset = GAME_PRESET_BOARDS[BOARD_PRESET];
		i32 count  = BOARD_ROWS * BOARD_COLUMNS;
		for(size i = 0; i < count; ++i) {
			enum block_type type = preset[i];
			if(type != BLOCK_TYPE_NONE) {
				v2_i32 p           = board_idx_to_coords(board, i);
				struct block block = {.type = type};
				board_block_set(board, block, p.x, (board->rows - 1) - p.y);
			}
		}
	}
	scrn_game_piece_spawn_rndm(scrn);
	scrn_game_state_set(scrn, SCRN_GAME_STATE_PLAY);
#endif
}

void
scrn_game_upd(struct app *app, f32 dt)
{
	f32 time_scale           = GAME_TIME_SCALE;
	struct scrn_game *scrn   = &app->scrn_game;
	struct frame_info *frame = &scrn->frame;
	frame->dt                = dt * time_scale;
	frame->timestamp         = frame->timestamp + frame->dt;
	f32 timestamp            = frame->timestamp;
	struct alloc scratch     = frame->alloc;
	marena_reset(&frame->marena);

	struct piece *piece   = &scrn->piece;
	struct board *board   = &scrn->board;
	struct garden *garden = &scrn->garden;

	g_dbg_str(SCRN_GAME_STATE_LABELS[scrn->state]);
	// g_dbg_str(piece_to_str(piece, board, scratch));
	// g_dbg_str(str8_fmt_push(scratch, "chain:%d", scrn->chain));
	garden_upd(garden, scrn->frame);

	switch(scrn->state) {
	case SCRN_GAME_STATE_PAUSE: {
		scrn_game_pause_upd(scrn);
	} break;
	case SCRN_GAME_STATE_EDITOR: {
		scrn_game_editor_upd(scrn);
	} break;
	case SCRN_GAME_STATE_FALLING: {
		scrn_game_falling_upd(scrn);
	} break;
	case SCRN_GAME_STATE_PLAY: {
		scrn->chain = 0;
		scrn_game_play_upd(scrn);
		board_fallings_upd(board, scrn->frame);
	} break;
	case SCRN_GAME_STATE_COLLAPSE: {
		scrn_game_collapse_upd(scrn);
	} break;
	case SCRN_GAME_STATE_OVER: {
		scrn_game_over_upd(scrn);
	} break;
	default: {
	} break;
	}

	{
		// score ui upd
		scrn->score_ui = scrn->score_ui + MAX(ABS((scrn->score - scrn->score_ui) * 0.25f), 1);
		scrn->score_ui = MIN(scrn->score_ui, scrn->score);
	}

	vfxs_upd(&scrn->matches_vfx, scrn->frame);
	vfxs_upd(&scrn->vfxs, scrn->frame);

	if(scrn->exit_to != SCRN_TYPE_NONE) {
		app_set_scrn(app, scrn->exit_to);
	}
}

void
scrn_game_drw(struct app *app)
{
	enum g_txt_style style     = G_TXT_STYLE_DEBUG;
	struct scrn_game *scrn     = &app->scrn_game;
	struct frame_info frame    = scrn->frame;
	f32 timestamp              = frame.timestamp;
	struct alloc scratch       = scrn->frame.alloc;
	struct board *board        = &scrn->board;
	struct garden *garden      = &scrn->garden;
	rec_i32 board_rec          = scrn_game_get_board_rec(scrn);
	rec_i32 garden_rec         = scrn_game_get_garden_rec(scrn);
	struct v2_i32 board_offset = {board_rec.x, board_rec.y + BLOCK_SIZE};

	g_drw_offset(0, 0);
	scrn_game_drw_bg(scrn);
	{
		i32 x = (SYS_DISPLAY_W * 0.5f) - ((scrn->board.columns) * 0.5f);
		i32 y = SYS_DISPLAY_H - 20;
		g_drw_offset(x, y);
		board_drw_dbg(&scrn->board);
	}

	{
		g_drw_offset(board_offset.x, board_offset.y);
		debug_draw_set_offset(board_offset.x, board_offset.y);
		scrn_game_drw_game(scrn);
		vfxs_drw(&scrn->matches_vfx, scrn->frame);
	}
	{
		g_drw_offset(0, 0);
		debug_draw_set_offset(0, 0);
		// debug_draw_rec_fill(REC_UNPACK(garden_rec));
		garden_drw(garden, garden_rec, scrn->theme);
	}

	debug_draw_set_offset(0, 0);
	g_drw_offset(0, 0);
	scrn_game_drw_walls(scrn);
	scrn_game_drw_hud(scrn);

	switch(scrn->state) {
	case SCRN_GAME_STATE_PAUSE: {
		scrn_game_pause_drw(scrn);
	} break;
	case SCRN_GAME_STATE_EDITOR: {
	} break;
	case SCRN_GAME_STATE_FALLING: {
	} break;
	case SCRN_GAME_STATE_PLAY: {
	} break;
	case SCRN_GAME_STATE_COLLAPSE: {
	} break;
	case SCRN_GAME_STATE_OVER: {
		scrn_game_over_drw(scrn);
	} break;
	default: {
	} break;
	}

	vfxs_drw(&scrn->vfxs, scrn->frame);

#if DEBUG
	{
		struct str_join params = {.sep = str8_lit(" ")};
		struct str8_list list  = {0};
		v2_i32 rel             = scrn_game_mouse_to_board(scrn);
		v2_i32 mouse           = v2_add_i32(rel, board_offset);
		v2_i32 coords          = board_px_to_coords(board, rel.x, rel.y);
		v2_i32 coords_px       = board_coords_to_px(board, coords.x, coords.y);
		rec_i32 tile_rec       = {coords_px.x + board_offset.x, coords_px.y + board_offset.y, board->block_size, board->block_size};
		i32 idx                = board_coords_to_idx(board, coords.x, coords.y);
		struct block *block    = board_block_get(board, coords.x, coords.y);
		debug_draw_cir(mouse.x, mouse.y, 3);
		debug_draw_rec(REC_UNPACK(tile_rec));

		str8_list_pushf(scratch, &list, "tile:%d,%d", coords.x, coords.y);
		str8_list_pushf(scratch, &list, "mouse:%d,%d", rel.x, rel.y);
		str8_list_pushf(scratch, &list, "idx:%d", idx);
		if(block != NULL) {
			str8_list_pushf(scratch, &list, "type:%s", BLOCK_TYPE_LABELS[block->type]);
		}

		{
			enum g_txt_style style = G_TXT_STYLE_DEBUG_SML;
			str8 str               = str8_list_join(scratch, &list, &params);
			v2_i32 size            = g_txt_size(str, style);
			rec_i32 layout         = rec_i32_anchor(APP_SCRN_REC, rec_i32_expand((rec_i32){SYS_DISPLAY_W, SYS_DISPLAY_H, size.x, size.y}, 1), (v2){0.0f, 1.0f});

			g_color(PRIM_MODE_BLACK);
			g_rec_fill(REC_UNPACK(layout));
			g_spr_mode(SPR_MODE_WHITE);
			g_txt(str, layout.x + 1, layout.y + 1, style);
		}
	}
	if(app->debug_drw || scrn->state == SCRN_GAME_STATE_EDITOR) {
		debug_draw_do(0, 0);
	}
#endif
}

static inline void
scrn_game_pause_drw(struct scrn_game *scrn)
{
	f32 timestamp          = scrn->frame.timestamp;
	struct board *board    = &scrn->board;
	f32 t                  = clamp_f32((timestamp - scrn->t) / 0.2f, 0.0f, 1.0f);
	rec_i32 rec            = scrn_game_get_board_rec(scrn);
	str8 str               = str8_lit("PAUSE");
	enum g_txt_style style = G_TXT_STYLE_HUD;

	g_color(PRIM_MODE_BLACK);

	{
		f32 alpha = lerp(0, 0.8f, t);
		g_pat(gfx_pattern_bayer_4x4(alpha * 16));
		g_rec_fill(rec.x, rec.y, rec.w, rec.h);
	}

	{
		f32 alpha      = lerp(0, 1.0f, t);
		v2_i32 size    = g_txt_size(str, style);
		rec_i32 layout = rec_i32_expand((rec_i32){0, 0, size.x + 1, size.y}, 1);
		layout         = rec_i32_anchor(rec, layout, (v2){0.5f, 0.5f});

		g_pat(gfx_pattern_bayer_4x4(alpha * 16));
		g_rec_fill(REC_UNPACK(layout));
		g_spr_mode(SPR_MODE_WHITE);
		g_txt(str, layout.x + 2, layout.y + 3, style);
	}

	g_pat(gfx_pattern_white());
}

static inline void
scrn_game_over_drw(struct scrn_game *scrn)
{
	f32 timestamp       = scrn->frame.timestamp;
	struct board *board = &scrn->board;
	f32 t               = clamp_f32((timestamp - scrn->t) / 0.2f, 0.0f, 1.0f);
	rec_i32 board_rec   = scrn_game_get_board_rec(scrn);

	g_color(PRIM_MODE_BLACK);

	{
		f32 alpha = lerp(0, 0.8f, t);
		g_pat(gfx_pattern_bayer_4x4(alpha * 16));
		g_rec_fill(board_rec.x, board_rec.y, board_rec.w, board_rec.h);
	}

	{
		str8 str               = str8_lit("FINISHED!");
		enum g_txt_style style = G_TXT_STYLE_HUD;
		f32 alpha              = lerp(0, 1.0f, t);
		v2_i32 size            = g_txt_size(str, style);
		rec_i32 layout         = rec_i32_expand((rec_i32){0, 0, size.x + 1, size.y}, 1);
		layout                 = rec_i32_anchor(board_rec, layout, (v2){0.5f, 0.5f});

		g_pat(gfx_pattern_bayer_4x4(alpha * 16));
		g_rec_fill(REC_UNPACK(layout));
		g_spr_mode(SPR_MODE_WHITE);
		g_txt(str, layout.x + 2, layout.y + 3, style);
	}

	g_pat(gfx_pattern_white());

	if(timestamp - scrn->t < 1.0f) { return; }

	{
		str8 str               = str8_lit("Again");
		enum g_txt_style style = G_TXT_STYLE_STORY;
		f32 alpha              = lerp(0, 1.0f, t);
		v2_i32 size            = g_txt_size(str, style);
		rec_i32 layout         = rec_i32_expand((rec_i32){0, 0, size.x + 1, size.y}, 1);
		layout                 = rec_i32_anchor(board_rec, layout, (v2){0.5f, 0.5f});
		layout.x += (size.x * 0.5f) + 2;
		layout.y += 20;

		g_pat(gfx_pattern_bayer_4x4(alpha * 16));
		g_rec_fill(REC_UNPACK(layout));
		g_txt(str, layout.x + 2, layout.y + 2, style);
	}

	{
		str8 str               = str8_lit("Back");
		enum g_txt_style style = G_TXT_STYLE_STORY;
		f32 alpha              = lerp(0, 1.0f, t);
		v2_i32 size            = g_txt_size(str, style);
		rec_i32 layout         = rec_i32_expand((rec_i32){0, 0, size.x + 1, size.y}, 1);
		layout                 = rec_i32_anchor(board_rec, layout, (v2){0.5f, 0.5f});
		layout.x -= (size.x * 0.5f) + 5;
		layout.y += 20;

		g_pat(gfx_pattern_bayer_4x4(alpha * 16));
		g_rec_fill(REC_UNPACK(layout));
		g_txt(str, layout.x + 2, layout.y + 2, style);
	}
}

static inline void
scrn_game_drw_bg(struct scrn_game *scrn)
{
	g_color(PRIM_MODE_BLACK);
	g_rec_fill(0, 0, SYS_DISPLAY_W, SYS_DISPLAY_H);
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
scrn_game_drw_hud(struct scrn_game *scrn)
{
	enum g_txt_style style = G_TXT_STYLE_HUD;
	i32 hud_w              = GAME_HUD_W;
	i32 hud_h              = SYS_DISPLAY_H - GAME_WALL_W;
	i32 margin             = 8;
	rec_i32 root           = rec_i32_anchor(APP_SCRN_REC, (rec_i32){.w = hud_w, .h = hud_h}, (v2){0.5f, 0.5f});
	rec_i32 inset          = rec_i32_inset_x(root, 2);
	enum game_theme theme  = scrn->theme;
	struct alloc scratch   = scrn->frame.alloc;

	g_color(PRIM_MODE_BLACK);
	g_rec_fill(root.x, root.y, root.w, root.h);
	g_color(PRIM_MODE_WHITE);
	g_lin(inset.x + inset.w - 1, inset.y, inset.x + inset.w - 1, inset.y + inset.h); // right
	g_lin(inset.x, inset.y, inset.x, inset.y + inset.h);                             // left
	// debug_draw_rec_i32(root);
	rec_i32_cut_top(&root, 13);
	// debug_draw_rec_i32(root);
	g_color(PRIM_MODE_WHITE);
	{
		str8 str       = str8_lit("Next\nseed");
		v2_i32 size    = g_txt_size(str, style);
		rec_i32 layout = rec_i32_cut_top(&root, size.y);
		v2_i32 cntr    = rec_i32_cntr(layout);
		g_txt_pivot(str, cntr.x, cntr.y, (v2){0.5f, 0.5f}, style);
		debug_draw_rec_i32(layout);
	}
	rec_i32_cut_top(&root, margin * 0.5f);
	{
		rec_i32 layout = rec_i32_cut_top(&root, BLOCK_SIZE);
		v2_i32 cntr    = rec_i32_cntr(layout);
		{
			i32 block_size     = scrn->board.block_size;
			i32 x              = cntr.x - (block_size * 0.5f);
			i32 y              = cntr.y - (block_size * 0.5f);
			struct block block = {.type = scrn->editor.type};
			block_drw(&block, theme, x, y, block_size);
		}
	}
	rec_i32_cut_top(&root, margin);
	{
		str8 str       = str8_lit("Level\n01");
		v2_i32 size    = g_txt_size(str, style);
		rec_i32 layout = rec_i32_cut_top(&root, size.y);
		v2_i32 cntr    = rec_i32_cntr(layout);
		g_txt_pivot(str, cntr.x, cntr.y, (v2){0.5f, 0.5f}, style);
		debug_draw_rec_i32(layout);
	}
	rec_i32_cut_top(&root, margin);
	{
		str8 str       = str8_fmt_push(scratch, "SCORE\n%" PRIu32 "", scrn->score_ui);
		v2_i32 size    = g_txt_size(str, style);
		rec_i32 layout = rec_i32_cut_top(&root, size.y);
		v2_i32 cntr    = rec_i32_cntr(layout);
		g_txt_pivot(str, cntr.x, cntr.y, (v2){0.5f, 0.5f}, style);
		debug_draw_rec_i32(layout);
	}
}

static inline void
scrn_game_drw_game(struct scrn_game *scrn)
{
	struct piece *piece = &scrn->piece;
	struct board *board = &scrn->board;
	board_drw(board, scrn->theme);
	if(piece->types[0] != 0) {
		piece_drw(piece, board, scrn->theme);
	}

	switch(scrn->state) {
	case SCRN_GAME_STATE_PLAY: {
	} break;
	default: {
	} break;
	}
}

static inline v2_i32
scrn_game_mouse_to_board(struct scrn_game *scrn)
{
	struct board *board = &scrn->board;
	v2_i32 offset       = {GAME_WALL_W, SYS_DISPLAY_H - GAME_WALL_W - (BLOCK_SIZE * board->rows)};
	i32 x               = inp_mouse_x();
	i32 y               = inp_mouse_y();
	i32 block_size      = board->block_size;
	i32 r               = board->rows;
	i32 c               = board->columns;
	v2_i32 res          = {
        clamp_i32(x, offset.x, (offset.x - 1) + (c * block_size)),
        y,
    };
	res = v2_sub_i32(res, offset);
	return res;
}

static void
scrn_game_piece_spawn_rndm(struct scrn_game *scrn)
{
	struct piece *piece = &scrn->piece;
	f32 timestamp       = scrn->frame.timestamp;
	i32 c               = scrn->board.columns;
	i32 r               = scrn->board.rows;
	i32 block_size      = scrn->board.block_size;
	enum g_tex_id ref   = GAME_THEME_REFS[scrn->theme];
	i32 id              = g_tex_refs_id_get(ref);
	struct tex t        = asset_tex(id);
	i32 max_type        = (t.w / block_size) - 1;
	piece->types[0]     = rndm_range_i32(NULL, BLOCK_TYPE_NONE + 1, max_type);
	piece->types[1]     = rndm_range_i32(NULL, BLOCK_TYPE_NONE + 1, max_type);
	piece->p.x          = (c / 2) - 1;
	piece->p.y          = r;
	piece->o.y          = -14;
	piece_ini(piece, timestamp);
}

static void
scrn_game_piece_set_blocks(struct scrn_game *scrn)
{
	struct board *board = &scrn->board;
	struct piece *piece = &scrn->piece;
	v2_i32 coords       = piece->p;
	v2_i32 rot          = PIECE_ROTATIONS[piece->rot];

	for(size i = 0; i < (size)ARRLEN(piece->types); ++i) {
		v2_i32 p = {
			coords.x + (rot.x * i),
			coords.y + (rot.y * i),
		};
		struct block block = {.type = piece->types[i]};
		board_block_set(board, block, p.x, p.y);
	}
}

void
scrn_game_state_set(struct scrn_game *scrn, enum scrn_game_state value)
{
	switch(value) {
	case SCRN_GAME_STATE_PLAY: {
	} break;
	case SCRN_GAME_STATE_COLLAPSE: {
		scrn_game_matches_vfx(scrn);
	} break;
	default: {
	} break;
	}
	scrn->t          = scrn->frame.timestamp;
	scrn->prev_state = scrn->state;
	scrn->state      = value;
}

void
scrn_game_state_pop(struct scrn_game *scrn)
{
	scrn->state = scrn->prev_state;
}

static inline void
scrn_game_play_upd(struct scrn_game *scrn)
{
	struct board *board     = &scrn->board;
	struct garden *garden   = &scrn->garden;
	struct piece *piece     = &scrn->piece;
	struct frame_info frame = scrn->frame;
	struct alloc scratch    = scrn->frame.alloc;
	b32 piece_collided      = false;
	b32 is_game_over        = false;
	f32 timestamp           = frame.timestamp;

	board_fallings_spawn(board);
	board_upd(board, frame);

	piece_collided = piece_upd(piece, board, frame);
	if(piece_collided) {
		scrn_game_piece_set_blocks(scrn);
		scrn_game_piece_spawn_rndm(scrn);
	}

	scrn_game_matches(scrn);

	// Update game over

	if(inp_key_just_pressed('p')) {
		scrn_game_state_set(scrn, SCRN_GAME_STATE_PAUSE);
	}
	if(inp_key_just_pressed('g')) {
		scrn_game_state_set(scrn, SCRN_GAME_STATE_EDITOR);
	}

	for(size i = 0; i < (size)ARRLEN(board->blocks); ++i) {
		if(board->blocks[i].type != BLOCK_TYPE_NONE) {
			struct v2_i32 coords = board_idx_to_coords(board, i);
			if(coords.y >= board->rows) {
				scrn_game_state_set(scrn, SCRN_GAME_STATE_OVER);
			}
		}
	}
}

static inline void
scrn_game_pause_upd(struct scrn_game *scrn)
{
	f32 timestamp = scrn->frame.timestamp;
	f32 t         = (timestamp - scrn->t);
	if(t < 0.4f) { return; }
	if(inp_key_just_pressed('p')) {
		scrn_game_state_pop(scrn);
	}
}

static inline void
scrn_game_editor_upd(struct scrn_game *scrn)
{
	f32 timestamp                   = scrn->frame.timestamp;
	struct scrn_game_editor *editor = &scrn->editor;
	struct board *board             = &scrn->board;

	if(inp_key_just_pressed('g')) {
		scrn_game_state_pop(scrn);
	}
	if(inp_key_just_pressed('1')) {
		scrn->editor.type = BLOCK_TYPE_NONE + 1;
	}
	if(inp_key_just_pressed('2')) {
		scrn->editor.type = BLOCK_TYPE_NONE + 2;
	}
	if(inp_key_just_pressed('3')) {
		scrn->editor.type = BLOCK_TYPE_NONE + 3;
	}
	if(inp_key_just_pressed('4')) {
		scrn->editor.type = BLOCK_TYPE_NONE + 4;
	}
	if(inp_key_just_pressed('5')) {
		scrn->editor.type = BLOCK_TYPE_NONE + 5;
	}
	if(inp_key_just_pressed('6')) {
		scrn->editor.type = BLOCK_TYPE_NONE + 6;
	}
	if(inp_key_just_pressed('7')) {
		scrn->editor.type = BLOCK_TYPE_NONE + 7;
	}
	if(inp_key_just_pressed('8')) {
		scrn->editor.type = BLOCK_TYPE_NONE + 8;
	}
	if(inp_key_just_pressed('9')) {
		scrn->editor.type = BLOCK_TYPE_NONE + 9;
	}
	if(inp_key_just_pressed('0')) {
		scrn->editor.type = BLOCK_TYPE_NONE + 10;
	}

	v2_i32 px     = scrn_game_mouse_to_board(scrn);
	v2_i32 coords = board_px_to_coords(board, px.x, px.y);

	if(inp_pressed(INP_MOUSE_LEFT) && editor->timestamp[coords.x] < timestamp) {
		struct block block = {.type = editor->type};
		board_block_set(&scrn->board, block, coords.x, coords.y);
		editor->timestamp[coords.x] = timestamp + 0.3f;
	}
	if(inp_pressed(INP_MOUSE_RIGHT)) {
		board_block_clr(&scrn->board, coords.x, coords.y);
	}
}

static inline void
scrn_game_falling_upd(struct scrn_game *scrn)
{
	struct board *board  = &scrn->board;
	f32 timestamp        = scrn->frame.timestamp;
	struct alloc scratch = scrn->frame.alloc;
	board_fallings_upd(board, scrn->frame);
	if(board->falling_count == 0) {
		i32 matches = scrn_game_matches(scrn);
	}
}

static inline void
scrn_game_collapse_upd(struct scrn_game *scrn)
{
	struct board *board     = &scrn->board;
	struct frame_info frame = scrn->frame;
	f32 timestamp           = frame.timestamp;
	struct alloc scratch    = frame.alloc;

	str8 str = str8_fmt_push(scratch, "vfx:%d", scrn->matches_vfx.count);
	g_dbg_str(str);

	if(scrn->matches_vfx.count == 0) {
		board_matches_clr(&scrn->board);
		board_fallings_spawn(board);
		scrn_game_state_set(scrn, SCRN_GAME_STATE_FALLING);
	}
}

static inline void
scrn_game_over_upd(struct scrn_game *scrn)
{
	f32 timestamp = scrn->frame.timestamp;
	f32 t         = (timestamp - scrn->t);
	if(t < 1.0f) { return; }
	if(g_ui_was_pressed(G_UI_ACCEPT)) {
		scrn_game_exit(scrn, SCRN_TYPE_GAME);
	}
	if(g_ui_was_pressed(G_UI_BACK)) {
		scrn_game_exit(scrn, SCRN_TYPE_TITLE);
	}
}

static inline void
scrn_game_matches_vfx(struct scrn_game *scrn)
{
	struct board *board = &scrn->board;
	f32 timestamp       = scrn->frame.timestamp;

	for(size i = 0; i < (size)ARRLEN(board->matches); ++i) {
		if(board->matches[i]) {
			v2_i32 p       = board_idx_to_px(board, i);
			i32 d          = board->block_size;
			i32 r          = board->block_size / 2;
			struct vfx vfx = {
				.p.x       = p.x + r,
				.p.y       = p.y + r,
				.timestamp = timestamp,
				.duration  = 0.4f,
				.type      = VFX_TYPE_SHAPE,
				.blink     = {
						.type       = VFX_BLINK_TRANSPARENT,
						.period_min = 0.25f,
						.period_max = 0.50f,
                },
				.shape = {
					.type  = VFX_SHAPE_TYPE_CIR_FILL,
					.color = PRIM_MODE_WHITE,
					.d     = d,
				},
			};
			vfxs_create(&scrn->matches_vfx, vfx);
		}
	}
}

static inline void
scrn_game_exit(struct scrn_game *scrn, enum scrn_type value)
{
	scrn->exit_to = value;
}

static inline rec_i32
scrn_game_get_board_rec(struct scrn_game *scrn)
{
	rec_i32 res         = {0};
	struct board *board = &scrn->board;
	res.x               = GAME_WALL_W;
	res.y               = SYS_DISPLAY_H - GAME_WALL_W - (BLOCK_SIZE * (board->rows + 1));
	res.w               = (board->columns * board->block_size) + 1;
	res.h               = (board->rows + 1) * board->block_size;
	return res;
}

static inline rec_i32
scrn_game_get_garden_rec(struct scrn_game *scrn)
{
	rec_i32 res         = {0};
	struct board *board = &scrn->board;
	i32 w               = (board->columns * board->block_size) + 1;
	res.x               = GAME_WALL_W + GAME_HUD_W + w;
	res.y               = 0;
	res.w               = w;
	res.h               = SYS_DISPLAY_H - GAME_WALL_W;
	return res;
}

static inline void
scrn_game_vfx_score(struct scrn_game *scrn, i32 x, i32 y, u32 value, u32 chain)
{
	f32 timestamp   = scrn->frame.timestamp;
	v2_i32 offset   = g_drw_offset(0, 0);
	v2 vfx_p        = {x + offset.x, y + offset.y};
	struct vfx data = {
		.p         = vfx_p,
		.p_target  = (v2){vfx_p.x, vfx_p.y + SCORE_VFX_Y_OFFSET},
		.timestamp = timestamp,
		.duration  = SCORE_VFX_LIFETIME,
		.type      = VFX_TYPE_TXT,
		.txt       = {
				  .style = G_TXT_STYLE_VFX,
				  .pivot = (v2){0.5f, 0.5f},
        },
	};
	g_drw_offset(offset.x, offset.y);

	struct vfx_handle handle = vfxs_create(&scrn->vfxs, data);
	struct vfx *vfx          = vfxs_get(&scrn->vfxs, handle);
	struct marena arena      = {0};
	marena_init(&arena, vfx->txt.buf, ARRLEN(vfx->txt.buf) * sizeof(vfx->txt.buf));
	struct alloc alloc = marena_allocator(&arena);
	str8 str           = str8_fmt_push(alloc, "%" PRIu32 " Chain", chain);
	vfx->txt.str       = str;
}

static inline i32
scrn_game_matches(struct scrn_game *scrn)
{
	struct board *board              = &scrn->board;
	struct garden *garden            = &scrn->garden;
	struct alloc scratch             = scrn->frame.alloc;
	f32 timestamp                    = scrn->frame.timestamp;
	struct board_matches_res matches = board_matches_upd(board, scratch);
	i32 block_size                   = board->block_size;

	switch(matches.total) {
	case 0: {
	} break;
	case 2: {
		g_sfx(G_SFX_MATCH_02, 1);
	} break;
	case 3: {
		g_sfx(G_SFX_MATCH_03, 1);
	} break;
	case 4: {
		g_sfx(G_SFX_MATCH_04, 1);
	} break;
	default: {
		g_sfx(G_SFX_MATCH_01, 1);
	} break;
	}

	u32 chain_end = scrn->chain + matches.groups.len;
	for(size i = 0; i < matches.groups.len; ++i) {
		v2_i32 p                       = {.y = I32_MAX};
		size group_count               = matches.groups.items[i].len;
		u8 column_count[BOARD_COLUMNS] = {0};
		enum block_type type           = BLOCK_TYPE_NONE;
		scrn->chain++;
		for(size j = 0; j < group_count; ++j) {
			i16 idx       = matches.groups.items[i].items[j];
			type          = board->blocks[idx].type;
			v2_i32 coords = board_idx_to_coords(board, idx);
			v2_i32 px     = board_idx_to_px(board, idx);
			p.x           = p.x + (px.x + block_size * 0.5f);
			p.y           = min_i32(px.y, p.y);
			column_count[coords.y]++;
		}
		p.x       = p.x / group_count;
		u32 score = (1 * (group_count - 3)) * scrn->chain;
		scrn_game_vfx_score(scrn, p.x, p.y, score, scrn->chain);
		scrn->score += score;
		i32 column_with_more = 0;
		for(size i = 0; i < (size)ARRLEN(column_count); ++i) {
			if(column_count[i] > column_count[column_with_more]) {
				column_with_more = i;
			}
		}
		garden_seed_add(garden, column_with_more, type, timestamp);
	}

	if(matches.total > 0) {
		scrn_game_state_set(scrn, SCRN_GAME_STATE_COLLAPSE);
	} else if(scrn->state != SCRN_GAME_STATE_PLAY) {
		scrn_game_state_set(scrn, SCRN_GAME_STATE_PLAY);
	}

	return matches.total;
}
