#
# Cross Platform Makefile
# Compatible with MSYS2/MINGW, Ubuntu 14.04.1 and Mac OS X
#
# You will need SDL2 (http://www.libsdl.org):
# Ubuntu:
#   apt-get install libsdl2-dev
# Mac OS X:
#   brew install sdl2
# MSYS2:
#   pacman -S mingw-w64-i686-SDL2
#
# You'll also need sdl-gfx for sdl2
# Ubuntu:
#   apt-get install libsdl2-gfx-dev 
# Mac OS X: #TODO: document this
# MSYS2: #TODO: document this

#For MSYS2, install
#opengl
#sdl2
#sdl2-gfx

#CXX = g++
#CXX = clang++

#TODO:
# Automatic dependency generation is required to correctly rebuild the project when .h files are changed.

EXE = robots
IMGUI_DIR = ./imgui
#sources
SOURCES = main.cpp application.cpp texture.cpp helpers.cpp robot_controller.cpp robot.cpp robot_arena.cpp timer.cpp file_dialog.cpp
#imgui library sources
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp $(IMGUI_DIR)/misc/cpp/imgui_stdlib.h
#imgui library sources for SDL
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp $(IMGUI_DIR)/backends/imgui_impl_sdlrenderer2.cpp
#the teenyat robot virtual machine
SOURCES += robot_vm/teenyat.c
#targets to make
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))

UNAME_S := $(shell uname -s)

DEBUGFLAGS += -g -Wall -Wformat -fsanitize=address

CXXFLAGS = -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CXXFLAGS += -std=c++20
CXXFLAGS += $(DEBUGFLAGS)

LIBS =

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

##modified to inlucde sdl2_gfxprimitives
ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	LIBS += -lGL -ldl `sdl2-config --libs` -lSDL2_gfx `pkg-config --libs SDL2_gfx`

	CXXFLAGS += `sdl2-config --cflags` `pkg-config --cflags SDL2_gfx`
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"
	LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo `sdl2-config --libs` `pkg-config --libs sdl2_gfx`
	LIBS += -L/usr/local/lib -L/opt/local/lib

	CXXFLAGS += `sdl2-config --cflags` `pkg-config --cflags sdl2_gfx`
	CXXFLAGS += -I/usr/local/include -I/opt/local/include
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "MinGW"
	LIBS += -lgdi32 -lopengl32 -limm32 `pkg-config --static --libs sdl2` `pkg-config --static --libs sdl2_gfx`

	CXXFLAGS += `pkg-config --cflags sdl2` `pkg-config --cflags sdl2_gfx`
	CFLAGS = $(CXXFLAGS)
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

all : $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o : $(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o : $(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o : $(IMGUI_DIR)/misc/cpp/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o : robot_vm/%.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(EXE) : $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)
clean :
	rm -f $(EXE) $(OBJS)

txt_to_bin :
	gcc robot_vm/txt_to_bin.c -o robot_vm/txt_to_bin

tny_prog : txt_to_bin
	./robot_vm/txt_to_bin robot_vm/tny_prog.txt robot_vm/tny_prog.bin

wall_rider : txt_to_bin
	./robot_vm/txt_to_bin robot_vm/wall_rider.txt robot_vm/wall_rider.bin

test_interface : txt_to_bin
	./robot_vm/txt_to_bin robot_vm/test_interface.txt robot_vm/test_interface.bin
