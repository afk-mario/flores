#include "scrn-game.h"

#include "app/app-data.h"
#include "app/app.h"
#include "base/marena.h"
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
#include "globals/g-dbg.h"
#include "globals/g-tex-refs.h"
#include "lib/rndm.h"
#include "piece/piece.h"
#include "scrns/scrn-game/scrn-game-data.h"
#include "sys/sys.h"

#include "block/block-defs.h"
#include "board/board-defs.h"
#include "globals/g-gfx.h"
#include "scrns/scrn-game/scrn-game-defs.h"

static inline v2_i32 scrn_game_mouse_to_board(struct scrn_game *scrn);
static inline void scrn_game_drw_bg(struct scrn_game *scrn);
static inline void scrn_game_drw_walls(struct scrn_game *scrn);
static inline void scrn_game_drw_hud(struct scrn_game *scrn);
static inline void scrn_game_drw_game(struct scrn_game *scrn);
static inline void scrn_game_drw_block(struct scrn_game *scrn, struct block *block);
static void scrn_game_piece_spawn_rndm(struct scrn_game *scrn);
static b32 scrn_game_piece_set_blocks(struct scrn_game *scrn);

#define GAME_WALL_W 3
#define GAME_HUD_W  SYS_DISPLAY_W - (GAME_WALL_W * 2) - (BLOCK_SIZE * (BOARD_COLUMNS * 2))
// #define BOARD_FULL

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

	f32 timestamp       = scrn->frame.timestamp;
	struct board *board = &scrn->board;
	board_ini(board, timestamp);
	scrn->editor.type = BLOCK_TYPE_A;
	scrn->piece       = (struct piece){0};
	scrn->theme       = GAME_THEME;

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
	scrn_game_piece_spawn_rndm(scrn);
	struct block block = {
		.type = rndm_range_i32(NULL, BLOCK_TYPE_NONE + 1, BLOCK_TYPE_C),
	};
#endif
}

void
scrn_game_upd(struct app *app, f32 dt)
{
	struct scrn_game *scrn   = &app->scrn_game;
	struct frame_info *frame = &scrn->frame;
	frame->timestamp         = frame->timestamp + dt;
	frame->dt                = dt;
	f32 timestamp            = frame->timestamp;
	struct alloc scratch     = frame->alloc;
	marena_reset(&frame->marena);

	board_upd(&scrn->board, scrn->frame);

	// if(scrn->board.state == BOARD_STATE_OVER) {
	// 	app_set_scrn(app, SCRN_TYPE_OVER);
	// }

	if(inp_key_just_pressed('p')) {
		scrn->state = scrn->state == SCRN_GAME_STATE_PAUSE ? SCRN_GAME_STATE_PLAY : SCRN_GAME_STATE_PAUSE;
	}
	{
		if(inp_key_just_pressed('0')) {
			scrn->editor.type = BLOCK_TYPE_NONE;
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

		{
			struct board *board             = &scrn->board;
			struct scrn_game_editor *editor = &scrn->editor;
			v2_i32 px                       = scrn_game_mouse_to_board(scrn);
			v2_i32 coords                   = board_px_to_coords(board, px.x, px.y);

			if(inp_pressed(INP_MOUSE_LEFT)) {
				struct block block = {.type = editor->type};
				board_block_set(&scrn->board, block, coords.x, coords.y);
			}
			if(inp_pressed(INP_MOUSE_RIGHT)) {
				board_block_clr(&scrn->board, coords.x, coords.y);
			}
		}
	}

	struct piece *piece = &scrn->piece;
	struct board *board = &scrn->board;

	g_dbg_str(piece_to_str(piece, board, scratch));
	switch(scrn->state) {
	case SCRN_GAME_STATE_PLAY: {
		if(piece_upd(piece, board, *frame)) {
			if(scrn_game_piece_set_blocks(scrn)) {
				app_set_scrn(app, SCRN_TYPE_OVER);
			} else {
				scrn_game_piece_spawn_rndm(scrn);
			}
		}
	} break;
	default: {
	} break;
	}

	if(scrn->state == SCRN_GAME_STATE_OVER) {
	}
}

void
scrn_game_drw(struct app *app)
{
	enum g_txt_style style  = G_TXT_STYLE_DEBUG;
	struct scrn_game *scrn  = &app->scrn_game;
	struct frame_info frame = scrn->frame;
	struct alloc scratch    = scrn->frame.alloc;
	struct board *board     = &scrn->board;
	v2_i32 board_offset     = {GAME_WALL_W, SYS_DISPLAY_H - GAME_WALL_W - (BLOCK_SIZE * board->rows)};

	g_drw_offset(0, 0);
	scrn_game_drw_bg(scrn);
	scrn_game_drw_walls(scrn);
	scrn_game_drw_hud(scrn);
	{
		i32 x = (SYS_DISPLAY_W * 0.5f) - ((scrn->board.columns) * 0.5f);
		i32 y = SYS_DISPLAY_H - 20;
		g_drw_offset(x, y);
		board_drw_dbg(&scrn->board);
	}
	g_drw_offset(board_offset.x, board_offset.y);
	debug_draw_set_offset(board_offset.x, board_offset.y);
	scrn_game_drw_game(scrn);
	debug_draw_set_offset(0, 0);
	g_drw_offset(0, 0);

#if DEBUG
	{
		struct str_join params = {.sep = str8_lit(" ")};
		struct str8_list list  = {0};
		v2_i32 offset          = board_offset;
		v2_i32 rel             = scrn_game_mouse_to_board(scrn);
		v2_i32 mouse           = v2_add_i32(rel, offset);
		v2_i32 coords          = board_px_to_coords(board, rel.x, rel.y);
		v2_i32 coords_px       = board_coords_to_px(board, coords.x, coords.y);
		rec_i32 tile_rec       = {coords_px.x + offset.x, coords_px.y + offset.y, board->block_size, board->block_size};
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
			rec_i32 layout         = rec_i32_anchor(APP_SCRN_REC, rec_i32_expand((rec_i32){SYS_DISPLAY_W, SYS_DISPLAY_H, size.x, size.y}, 1), (v2){1.0f, 1.0f});

			g_color(PRIM_MODE_BLACK);
			g_rec_fill(REC_UNPACK(layout));
			g_spr_mode(SPR_MODE_WHITE);
			g_txt(str, layout.x + 1, layout.y + 1, style);
		}
	}
	if(app->debug_drw) {
		debug_draw_do(0, 0);
	}
#endif
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

	g_color(PRIM_MODE_BLACK);
	g_rec_fill(root.x, root.y, root.w, root.h);
	g_color(PRIM_MODE_WHITE);
	g_lin(inset.x + inset.w - 1, inset.y, inset.x + inset.w - 1, inset.y + inset.h); // right
	g_lin(inset.x, inset.y, inset.x, inset.y + inset.h);                             // left
	debug_draw_rec_i32(root);
	rec_i32_cut_top(&root, 13);
	debug_draw_rec_i32(root);
	debug_draw_rec_i32(root);
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
			struct block block = {.type = BLOCK_TYPE_A};
			block_drw(&block, theme, x, y, block_size);
		}
		// {
		// 	i32 id                 = g_tex_refs_id_get(G_TEX_ACORN);
		// 	struct tex t           = asset_tex(id);
		// 	struct tex_rec tex_rec = asset_tex_rec(id, 0, 0, t.w, t.h);
		// 	g_spr_piv(tex_rec, cntr.x, cntr.y, 0, (v2){0.5f, 0.5f});
		// }
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
		str8 str       = str8_lit("SCORE\n0000");
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
	piece->p.x          = rndm_range_i32(NULL, 0, c - 2);
	piece->p.y          = r;
	piece_ini(piece, timestamp);
	scrn->state = SCRN_GAME_STATE_PLAY;
}

static b32
scrn_game_piece_set_blocks(struct scrn_game *scrn)
{
	b32 res             = false;
	struct board *board = &scrn->board;
	struct piece *piece = &scrn->piece;
	v2_i32 coords       = piece->p;
	v2_i32 rot          = PIECE_ROTATIONS[piece->rot];

	if(coords.y >= board->rows) {
		res = true;
	} else {
		for(size i = 0; i < (size)ARRLEN(piece->types); ++i) {
			struct block block = {.type = piece->types[i]};
			board_block_set(
				board,
				block,
				coords.x + (rot.x * i),
				coords.y + (rot.y * i));
		}
	}
	return res;
}
