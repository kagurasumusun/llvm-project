# LLVM/Clang Makefile for MaxSignal/Player 0.6.2.3-wince
#
# Overlay only. Do not patch Player C++.
# WinCE 6.0, no audio, SDL 1.2 + pixman + libpng + zlib + liblcf.
#
# CROSS is the CeGCC-named wrapper that binds --target=arm-pc-wince only.

CROSS   ?= arm-mingw32ce
CC      ?= $(CROSS)-gcc
CXX     ?= $(CROSS)-g++
AR      ?= $(CROSS)-ar
RANLIB  ?= $(CROSS)-ranlib

PREFIX  ?= /usr/local
OUT     ?= easyrpg-player.exe

# UNDER_CE is MaxSignal's clock/helper gate. Do not invent SUPPORT_AUDIO.
CPPFLAGS += -DUNDER_CE -DUSE_SDL=1 -DUNICODE -D_UNICODE -DNDEBUG
CPPFLAGS += -I. -Isrc
CPPFLAGS += -I$(PREFIX)/include
CPPFLAGS += -I$(PREFIX)/include/SDL
CPPFLAGS += -I$(PREFIX)/include/pixman-1
CPPFLAGS += -I$(PREFIX)/include/libpng16

CXXFLAGS += -std=c++14 -O2 -fno-math-errno
CFLAGS   += -O2

LDFLAGS  += -L$(PREFIX)/lib
# SDL_main before SDL. GNU lib*.a names only.
LIBS     += -lSDLmain -lSDL -llcf -lpixman-1 -lpng16 -lz -liconv -lcecompat

SRCS = \
	src/async_handler.cpp \
	src/audio.cpp \
	src/background.cpp \
	src/baseui.cpp \
	src/battle_animation.cpp \
	src/bitmap.cpp \
	src/cache.cpp \
	src/drawable.cpp \
	src/drawable_list.cpp \
	src/drawable_mgr.cpp \
	src/exe_reader.cpp \
	src/filefinder.cpp \
	src/font.cpp \
	src/fps_overlay.cpp \
	src/frame.cpp \
	src/game_actor.cpp \
	src/game_actors.cpp \
	src/game_battle.cpp \
	src/game_battlealgorithm.cpp \
	src/game_battler.cpp \
	src/game_character.cpp \
	src/game_clock.cpp \
	src/game_commonevent.cpp \
	src/game_enemy.cpp \
	src/game_enemyparty.cpp \
	src/game_event.cpp \
	src/game_interpreter.cpp \
	src/game_interpreter_battle.cpp \
	src/game_interpreter_map.cpp \
	src/game_map.cpp \
	src/game_message.cpp \
	src/game_party.cpp \
	src/game_party_base.cpp \
	src/game_pictures.cpp \
	src/game_player.cpp \
	src/game_quit.cpp \
	src/game_screen.cpp \
	src/game_switches.cpp \
	src/game_system.cpp \
	src/game_targets.cpp \
	src/game_variables.cpp \
	src/game_vehicle.cpp \
	src/graphics.cpp \
	src/hslrgb.cpp \
	src/image_bmp.cpp \
	src/image_png.cpp \
	src/image_xyz.cpp \
	src/input.cpp \
	src/input_buttons_desktop.cpp \
	src/input_source.cpp \
	src/instrumentation.cpp \
	src/main_data.cpp \
	src/message_overlay.cpp \
	src/meta.cpp \
	src/output.cpp \
	src/pending_message.cpp \
	src/plane.cpp \
	src/platform.cpp \
	src/player.cpp \
	src/rect.cpp \
	src/rtp.cpp \
	src/rtp_table.cpp \
	src/scene.cpp \
	src/scene_actortarget.cpp \
	src/scene_battle.cpp \
	src/scene_battle_rpg2k.cpp \
	src/scene_battle_rpg2k3.cpp \
	src/scene_debug.cpp \
	src/scene_end.cpp \
	src/scene_equip.cpp \
	src/scene_file.cpp \
	src/scene_gamebrowser.cpp \
	src/scene_gameover.cpp \
	src/scene_import.cpp \
	src/scene_item.cpp \
	src/scene_load.cpp \
	src/scene_logo.cpp \
	src/scene_map.cpp \
	src/scene_menu.cpp \
	src/scene_name.cpp \
	src/scene_order.cpp \
	src/scene_save.cpp \
	src/scene_shop.cpp \
	src/scene_skill.cpp \
	src/scene_status.cpp \
	src/scene_teleport.cpp \
	src/scene_title.cpp \
	src/screen.cpp \
	src/sdl_ui.cpp \
	src/sprite.cpp \
	src/sprite_airshipshadow.cpp \
	src/sprite_battler.cpp \
	src/sprite_character.cpp \
	src/sprite_timer.cpp \
	src/spriteset_battle.cpp \
	src/spriteset_map.cpp \
	src/state.cpp \
	src/text.cpp \
	src/tilemap.cpp \
	src/tilemap_layer.cpp \
	src/transition.cpp \
	src/utils.cpp \
	src/weather.cpp \
	src/window.cpp \
	src/window_about.cpp \
	src/window_actorinfo.cpp \
	src/window_actorsp.cpp \
	src/window_actorstatus.cpp \
	src/window_actortarget.cpp \
	src/window_base.cpp \
	src/window_battlecommand.cpp \
	src/window_battlemessage.cpp \
	src/window_battlestatus.cpp \
	src/window_command.cpp \
	src/window_equip.cpp \
	src/window_equipitem.cpp \
	src/window_equipstatus.cpp \
	src/window_face.cpp \
	src/window_gamelist.cpp \
	src/window_gold.cpp \
	src/window_help.cpp \
	src/window_import_progress.cpp \
	src/window_item.cpp \
	src/window_keyboard.cpp \
	src/window_menustatus.cpp \
	src/window_message.cpp \
	src/window_name.cpp \
	src/window_numberinput.cpp \
	src/window_paramstatus.cpp \
	src/window_savefile.cpp \
	src/window_selectable.cpp \
	src/window_settings.cpp \
	src/window_shop.cpp \
	src/window_shopbuy.cpp \
	src/window_shopnumber.cpp \
	src/window_shopparty.cpp \
	src/window_shopsell.cpp \
	src/window_shopstatus.cpp \
	src/window_skill.cpp \
	src/window_skillstatus.cpp \
	src/window_targetstatus.cpp \
	src/window_teleport.cpp \
	src/window_varlist.cpp \
	src/wince_clock.cpp \
	src/main.cpp

OBJS = $(SRCS:.cpp=.o)

.PHONY: all clean
all: $(OUT)

$(OUT): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(OUT)
