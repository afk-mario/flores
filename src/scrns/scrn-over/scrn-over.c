#include "scrn-over.h"
#include "app/app.h"
#include "base/str.h"
#include "globals/g-gfx.h"
#include "globals/g-ui.h"
#include "sys/sys.h"

void
scrn_over_ini(struct app *app)
{
	app->upd = scrn_over_upd;
	app->drw = scrn_over_drw;
}

void
scrn_over_upd(struct app *app, f32 dt)
{
	if(g_ui_just_pressed(G_UI_BACK)) {
		app_set_scrn(app, SCRN_TYPE_GAME);
	}
	if(g_ui_just_pressed(G_UI_ACCEPT)) {
		app_set_scrn(app, SCRN_TYPE_TITLE);
	}
}

void
scrn_over_drw(struct app *app)
{
	enum g_txt_style style = G_TXT_STYLE_HUD;
	str8 str               = str8_lit("Game Over");

	g_spr_mode(SPR_MODE_WHITE);
	g_txt_pivot(str, SYS_DISPLAY_W * 0.5f, SYS_DISPLAY_H * 0.5f, (v2){0.5f, 0.5f}, style);
	g_spr_mode(0);
}
