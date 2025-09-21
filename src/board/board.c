#include "board.h"
#include "base/log.h"
#include "base/mem.h"
#include "base/rec.h"
#include "base/types.h"
#include "base/v2.h"
#include "block/block-type.h"
#include "block/block.h"
#include "engine/debug-draw/debug-draw.h"
#include "falling/falling.h"
#include "globals/g-gfx.h"
#include "globals/g-sfx-refs.h"
#include "piece/piece-defs.h"
#include "scrns/scrn-game/scrn-game-data.h"

void
board_ini(struct board *board, f32 timestamp)
{
	i32 r                 = board->rows;
	i32 c                 = board->columns;
	board->columns        = BOARD_COLUMNS;
	board->rows           = BOARD_ROWS;
	board->block_size     = BLOCK_SIZE;
	board->block_size_inv = 1.0f / (f32)BLOCK_SIZE;
	board->falling_count  = 0;

	mclr_array(board->blocks);
	mclr_array(board->fallings);
}

void
board_upd(struct board *board, struct frame_info frame)
{
	f32 timestamp        = frame.timestamp;
	struct alloc scratch = frame.alloc;

	for(size i = 0; i < (size)ARRLEN(board->blocks); ++i) {
		struct block *block = board->blocks + i;
		if(block->type == BLOCK_TYPE_NONE) { continue; }
		block_upd(block, board->block_size);
	}
}

void
board_fallings_upd(struct board *board, struct frame_info frame)
{
	f32 timestamp = frame.timestamp;
	for(size i = 0; i < (size)ARRLEN(board->fallings); ++i) {
		struct falling *falling = board->fallings + i;
		if(falling->id == 0) { continue; }
		if(falling_upd(falling, board, frame)) {
			board_falling_remove(board, (struct falling_handle){.id = i});
			struct block block = {.type = falling->type};
			board_block_set(board, block, falling->p.x, falling->p.y);
		}
	}
}

void
board_drw(struct board *board, enum game_theme theme)
{
	i32 r          = board->rows;
	i32 c          = board->columns;
	i32 block_size = board->block_size;

	{
		enum prim_mode bg = GAME_BOARD_BG[theme];
		g_color(bg);
		i32 x = 0;
		i32 y = -1 * board->block_size;
		i32 w = (board->columns + 1) * board->block_size;
		i32 h = (board->rows + 1) * board->block_size;
		g_rec_fill(x, y, w, h);
		debug_draw_rec(x, y, w, h);
	}

	g_color(PRIM_MODE_BLACK);
	for(size i = 0; i < (size)ARRLEN(board->blocks); ++i) {
		struct block *block = board->blocks + i;
		if(block->type == BLOCK_TYPE_NONE) { continue; }
		v2_i32 p      = board_idx_to_px(board, i);
		v2_i32 coords = board_idx_to_coords(board, i);

		for(size j = 0; j < (size)ARRLEN(PIECE_ROTATIONS); ++j) {
			v2_i32 rotation     = PIECE_ROTATIONS[j];
			v2_i32 other_coords = v2_add_i32(coords, rotation);
			if(other_coords.x > board->columns - 1) {
				continue;
			}
			i32 other_id = board_coords_to_idx(board, other_coords.x, other_coords.y);
			if(other_id > -1) {
				struct block *other_block = board->blocks + other_id;
				if(block->type == other_block->type) {
					block_link_drw(theme, p.x, p.y, rotation.x, rotation.y, block_size);
				}
			}
		}

		// block_drw(block, theme, p.x, p.y, board->block_size);
	}

	for(size i = 0; i < (size)ARRLEN(board->blocks); ++i) {
		struct block *block = board->blocks + i;
		if(block->type == BLOCK_TYPE_NONE) { continue; }
		v2_i32 p = board_idx_to_px(board, i);
		block_drw(block, theme, p.x, p.y, board->block_size);
	}

	for(size i = 0; i < (size)ARRLEN(board->fallings); ++i) {
		struct falling *falling = board->fallings + i;
		if(falling->id == 0) { continue; }
		falling_drw(falling, board, theme);
	}
}

void
board_block_set(struct board *board, struct block block, i32 x, i32 y)
{
	i16 idx = board_coords_to_idx(board, x, y);
	if(idx < 0 || idx > (size)ARRLEN(board->blocks)) {
		log_warn("board", "tried setting at invalid: %d,%d[%d]=%d", x, y, idx, block.type);
		return;
	}
	board->blocks[idx] = block;
}

void
board_block_clr(struct board *board, i32 x, i32 y)
{
	i16 idx = board_coords_to_idx(board, x, y);
	if(idx < 0 || idx > (size)ARRLEN(board->blocks)) { return; }
	board->blocks[idx].type = BLOCK_TYPE_NONE;
}

b32
board_block_has(struct board *board, i16 x, i16 y)
{
	b32 res = false;
	i16 idx = board_coords_to_idx(board, x, y);
	if(idx < 0 || idx > (size)ARRLEN(board->blocks)) { return res; }
	res = board->blocks[idx].type != BLOCK_TYPE_NONE;
	return res;
}

struct block *
board_block_get(struct board *board, i32 x, i32 y)
{
	struct block *res = NULL;
	i16 idx           = board_coords_to_idx(board, x, y);
	if(idx < 0 || idx > (size)ARRLEN(board->blocks)) { return res; }
	res = board->blocks + idx;
	return res;
}

v2_i32
board_idx_to_coords(struct board *board, i16 idx)
{
	v2_i32 res = {
		.x = idx % board->columns,
		.y = idx / board->columns,
	};
	return res;
}

v2_i32
board_idx_to_px(struct board *board, i16 idx)
{
	v2_i32 coords = board_idx_to_coords(board, idx);
	v2_i32 res    = board_coords_to_px(board, coords.x, coords.y);
	return res;
}

i16
board_coords_to_idx(struct board *board, i32 x, i32 y)
{
	i16 res = -1;
	if(y < 0 || x < 0) { return res; }
	res = (y * board->columns) + x;
	if(res > (size)ARRLEN(board->blocks)) { return res = -1; }
	return res;
}

v2_i32
board_coords_to_px(struct board *board, i32 x, i32 y)
{
	// top-left pixel of the tile at (x, y), with y=0 = bottom row
	v2_i32 res = {
		x * board->block_size,
		(board->rows - 1 - y) * board->block_size};
	return res;
}

v2_i32
board_px_to_coords(struct board *board, i32 px, i32 py)
{
	i32 cx     = px * board->block_size_inv;
	i32 tile_y = div_i32_floor(py, board->block_size);
	i32 cy     = (board->rows - 1) - tile_y;
	v2_i32 res = {cx, cy};
	return res;
}

b32
board_is_wall_x(struct board *board, i32 x)
{
	b32 res = x < 0 || x > board->columns - 1;
	return res;
}

b32
board_is_wall_y(struct board *board, i32 y)
{
	b32 res = y < 0;
	return res;
}

b32
board_is_wall(struct board *board, i32 x, i32 y)
{
	b32 res = board_is_wall_x(board, x) || board_is_wall_y(board, y);
	return res;
}

void
board_drw_dbg(struct board *board)
{
	i32 r          = board->rows;
	i32 c          = board->columns;
	rec_i32 rec    = {0, 0, board->columns, board->rows};
	rec_i32 border = rec_i32_expand(rec, 1);
	g_color(PRIM_MODE_BLACK);
	g_rec_fill(REC_UNPACK(rec));
	g_color(PRIM_MODE_WHITE);
	g_rec(REC_UNPACK(border));

	g_color(PRIM_MODE_WHITE);
	for(size i = 0; i < (size)ARRLEN(board->blocks); ++i) {
		struct block *block = board->blocks + i;
		if(block->type == BLOCK_TYPE_NONE) { continue; }
		v2_i32 p = board_idx_to_coords(board, i);
		g_px(p.x, p.y);
	}
}

struct falling_handle
board_falling_spawn(struct board *board, struct falling value)
{
	struct falling_handle res = {0};
	for(size i = 1; i < (size)ARRLEN(board->fallings); ++i) {
		struct falling *item = board->fallings + i;
		if(item->id == 0) {
			res.id = i;
		}
	}
	if(res.id != 0) {
		board->falling_count++;
		struct falling *item = board->fallings + res.id;
		*item                = value;
		item->id             = res.id;
	}
	return res;
}

void
board_falling_remove(struct board *board, struct falling_handle handle)
{
	dbg_assert(handle.id > 0 && handle.id < (size)ARRLEN(board->fallings));
	if(board->fallings[handle.id].id != 0) {
		board->falling_count--;
	}
	board->fallings[handle.id].id = 0;
}

i32
board_flood_fill(struct board *board, i16 x, i16 y, i16 *group)
{
	static const i32 dir_x[4] = {-1, 1, 0, 0};
	static const i32 dir_y[4] = {0, 0, -1, 1};

	i32 res                           = 0;
	u8 *visited                       = board->visited;
	i32 r                             = board->rows;
	i32 c                             = board->columns;
	i32 q[BOARD_COLUMNS * BOARD_ROWS] = {0};
	i32 qh                            = 0;
	i32 qt                            = 0;
	i32 start_idx                     = board_coords_to_idx(board, x, y);
	enum block_type type              = board->blocks[start_idx].type;

	q[qt++]            = start_idx;
	visited[start_idx] = 1;

	while(qh < qt) {
		i32 idx       = q[qh++];
		v2_i32 coords = board_idx_to_coords(board, idx);
		group[res++]  = idx;
		for(size k = 0; k < 4; ++k) {
			i32 ny = coords.y + dir_y[k];
			i32 nx = coords.x + dir_x[k];
			if(ny < 0 || ny >= r || nx < 0 || nx >= c) continue;

			i32 nidx = board_coords_to_idx(board, nx, ny);
			if(!visited[nidx] && board->blocks[nidx].type == (i16)type) {
				visited[nidx] = 1;
				q[qt++]       = nidx;
			}
		}
	}

	return res;
}

struct board_matches_res
board_matches_upd(struct board *board, struct alloc alloc)
{
	mclr_array(board->visited);
	mclr_array(board->matches);
	struct board_matches_res res = {0};
	i32 rows                     = board->rows;
	i32 cols                     = board->columns;
	i32 count                    = rows * cols;
	u8 *visited                  = board->visited;
	u8 *matches                  = board->matches;
	i16 group[BOARD_COLUMNS * BOARD_ROWS];
	i32 max_groups   = 10;
	res.groups.items = alloc_arr(alloc, struct board_matches_group, max_groups);

	for(size y = 0; y < rows; ++y) {
		for(size x = 0; x < cols; ++x) {
			i32 idx              = board_coords_to_idx(board, x, y);
			enum block_type type = board->blocks[idx].type;
			if(type == BLOCK_TYPE_NONE || visited[idx]) {
				continue;
			}

			i32 n = board_flood_fill(board, x, y, group);
			if(n >= 4) {
				dbg_assert(res.groups.len < max_groups);
				res.groups.items[res.groups.len++] =
					(struct board_matches_group){
						.len   = n,
						.items = alloc_arr(alloc, i16, n),
					};
				for(size i = 0; i < n; ++i) {
					matches[group[i]]                             = 1;
					res.groups.items[res.groups.len - 1].items[i] = group[i];
					res.total++;
				}
			}
		}
	}

	return res;
}

void
board_matches_clr(struct board *board)
{
	for(size i = 0; i < (size)ARRLEN(board->matches); ++i) {
		if(board->matches[i] != 0) {
			v2_i32 coords = board_idx_to_coords(board, i);
			board_block_clr(board, coords.x, coords.y);
		}
	}
}

void
board_fallings_spawn(struct board *board)
{
	for(size i = 0; i < (size)ARRLEN(board->blocks); ++i) {
		enum block_type type = board->blocks[i].type;
		if(type != BLOCK_TYPE_NONE) {
			v2_i32 coords = board_idx_to_coords(board, i);
			if(coords.y == 0) { continue; }
			i16 idx = board_coords_to_idx(board, coords.x, coords.y - 1);
			if(board->blocks[idx].type == BLOCK_TYPE_NONE) {
				struct falling falling = {.type = type, .p = coords};
				board_falling_spawn(board, falling);
				board_block_clr(board, coords.x, coords.y);
				g_sfx(G_SFX_SPAWN_01, 1);
			}
		}
	}
}
