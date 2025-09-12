.PHONY: all clean linux www pd macos

COMPANY_NAME := afk-mario
GAME_NAME    := flores
DESTDIR      := build/

all: linux_run

clean:
	rm -rf $(DESTDIR)

tools_build:
	make -f ./luna/tools.mk clean
	make -f ./luna/tools.mk CDEFS="-DSYS_LOG_LEVEL=1" DEBUG=1 luna-meta-gen
	make -f ./luna/tools.mk CDEFS="-DSYS_LOG_LEVEL=1" DEBUG=1 luna-assets-gen

assets_gen_test:
	mkdir -p ./tmp
	./bin/luna-assets src/assets ./tmp

assets_gen: assets_gen_build assets_gen_test

pd:
	make -f ./luna/playdate.mk \
		DESTDIR=$(DESTDIR) \
		GAME_NAME=$(GAME_NAME)

pd_build:
	make -f ./luna/playdate.mk build \
		DESTDIR=$(DESTDIR) \
		GAME_NAME=$(GAME_NAME)

pd_build_sim:
	make -f ./luna/playdate.mk build_sim \
		DESTDIR=$(DESTDIR) \
		GAME_NAME=$(GAME_NAME)

pd_clean:
	make -f ./luna/playdate.mk clean \
		DESTDIR=$(DESTDIR) \
		GAME_NAME=$(GAME_NAME)

pd_release:
	make -f ./luna/playdate.mk release \
		DESTDIR=$(DESTDIR) \
		GAME_NAME=$(GAME_NAME) \
		CDEFS="-DSYS_SHOW_FPS=0 -DSYS_LOG_LEVEL=2" \
		DEBUG=0

pd_run:
	make -f ./luna/playdate.mk run \
		DESTDIR=$(DESTDIR) \
		GAME_NAME=$(GAME_NAME)


linux:
	make -f ./luna/linux.mk \
		DESTDIR=$(DESTDIR) \
		GAME_NAME=$(GAME_NAME)

linux_clean:
	make -f ./luna/linux.mk clean \
		DESTDIR=$(DESTDIR)

linux_build:
	make -f ./luna/linux.mk build \
		DESTDIR=$(DESTDIR) \
		GAME_NAME=$(GAME_NAME)

linux_run:
	make -f ./luna/linux.mk run \
		DESTDIR=$(DESTDIR) \
		GAME_NAME=$(GAME_NAME)

linux_release:
	make -f ./luna/linux.mk release \
		DESTDIR=$(DESTDIR) \
		GAME_NAME=$(GAME_NAME) \
		CDEFS="-DSYS_LOG_LEVEL=2 -DDEV=1"

linux_publish:
	make -f ./luna/linux.mk publish \
		DESTDIR=$(DESTDIR) \
		GAME_NAME=$(GAME_NAME) \
		COMPANY_NAME=$(COMPANY_NAME) \
		CDEFS="-DSYS_LOG_LEVEL=2 -DDEV=1"


www:
	make -f ./luna/www.mk \
		DESTDIR=$(DESTDIR) \
		GAME_NAME=$(GAME_NAME)

www_clean:
	make -f ./luna/www.mk clean \
		DESTDIR=$(DESTDIR)

www_build:
	make -f ./luna/www.mk build \
		DESTDIR=$(DESTDIR) \
		GAME_NAME=$(GAME_NAME)

www_run:
	make -f ./luna/www.mk run \
		DESTDIR=$(DESTDIR) \
		GAME_NAME=$(GAME_NAME)

www_release:
	make -f ./luna/www.mk release \
		DESTDIR=$(DESTDIR) \
		GAME_NAME=$(GAME_NAME) \
		CDEFS="-DSYS_LOG_LEVEL=2 -DDEV=1"

www_publish:
	make -f ./luna/www.mk publish \
		DESTDIR=$(DESTDIR) \
		GAME_NAME=$(GAME_NAME) \
		CDEFS="-DSYS_LOG_LEVEL=2 -DDEV=1"

macos:
	make -f ./luna/macos.mk \
		DESTDIR=$(DESTDIR) \
		GAME_NAME=$(GAME_NAME) \
		CDEFS="-DSYS_LOG_LEVEL=2 -DDEV=1"

macos_run:
	make -f ./luna/macos.mk run\
		DESTDIR=$(DESTDIR) \
		GAME_NAME=$(GAME_NAME)

macos_build:
	make -f ./luna/macos.mk build \
		DESTDIR=$(DESTDIR) \
		GAME_NAME=$(GAME_NAME)

macos_release:
	make -f ./luna/macos.mk release \
		DESTDIR=$(DESTDIR) \
		GAME_NAME=$(GAME_NAME) \
		CDEFS="-DSYS_LOG_LEVEL=2 -DDEV=1"

macos_publish:
	make -f ./luna/macos.mk publish \
		DESTDIR=$(DESTDIR) \
		GAME_NAME=$(GAME_NAME) \
		CDEFS="-DSYS_LOG_LEVEL=2 -DDEV=1"

win:
	make -f ./luna/win.mk \
		DESTDIR=$(DESTDIR) \
		GAME_NAME=$(GAME_NAME) \
		CDEFS="-DSYS_LOG_LEVEL=2 -DDEV=1"

win_run:
	make -f ./luna/win.mk run\
		DESTDIR=$(DESTDIR) \
		GAME_NAME=$(GAME_NAME)

win_build:
	make -f ./luna/win.mk build \
		DESTDIR=$(DESTDIR) \
		GAME_NAME=$(GAME_NAME)

win_release:
	make -f ./luna/win.mk release \
		DESTDIR=$(DESTDIR) \
		GAME_NAME=$(GAME_NAME) \
		CDEFS="-DSYS_LOG_LEVEL=2 -DDEV=1"

win_publish:
	make -f ./luna/win.mk publish \
		DESTDIR=$(DESTDIR) \
		GAME_NAME=$(GAME_NAME) \
		CDEFS="-DSYS_LOG_LEVEL=2 -DDEV=1"
