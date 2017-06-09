ifeq ($(OS),Windows_NT)
	platform=windows
else
	UNAME:=$(shell uname -s)
	ifeq ($(UNAME),Linux)
		platform=linux
	endif
	ifeq ($(UNAME),Darwin)
		platform=macos
	endif
endif
ifeq ($(platform),emscripten)
CC=emcc
CXX=em++
endif
CXXFLAGS=-c -std=c++11 -Wall -O2 -Iexternal/ouzel/ouzel -I/opt/local/include -Winvalid-pch -include src/Prefix.hpp
LDFLAGS=-O2 -L. -louzel -L/opt/local/lib -lvlc
ifeq ($(platform),windows)
LDFLAGS+=-u WinMain -ld3d11 -lopengl32 -ldxguid -lxinput9_1_0 -lshlwapi -ldinput8 -luser32 -lgdi32 -lshell32 -lole32 -loleaut32 -ldsound
else ifeq ($(platform),raspbian)
CXXFLAGS+=-DRASPBIAN
LDFLAGS+=-L/opt/vc/lib -lGLESv2 -lEGL -lbcm_host -lopenal -lpthread
else ifeq ($(platform),linux)
LDFLAGS+=-lX11 -lGL -lopenal -lpthread -lXcursor -lXss
else ifeq ($(platform),macos)
LDFLAGS+=-framework AudioToolbox \
	-framework AudioToolbox \
	-framework Cocoa \
	-framework CoreVideo \
	-framework IOKit \
	-framework Metal \
	-framework OpenAL \
	-framework OpenGL \
	-framework QuartzCore
else ifeq ($(platform),emscripten)
	LDFLAGS+=--embed-file Resources -s TOTAL_MEMORY=33554432
endif
SOURCES=src/main.cpp \
	src/Player.cpp \
	src/VideoLibvlc.cpp
BASE_NAMES=$(basename $(SOURCES))
OBJECTS=$(BASE_NAMES:=.o)
ifeq ($(platform),emscripten)
EXECUTABLE=video_play.js
else
EXECUTABLE=video_play
endif

.PHONY: all
all: bundle

.PHONY: debug
debug: target=debug
debug: CXXFLAGS+=-DDEBUG -g
debug: bundle

.PHONY: bundle
bundle: $(EXECUTABLE)
ifeq ($(platform),macos)
bundle:
	mkdir -p $(EXECUTABLE).app
	mkdir -p $(EXECUTABLE).app/Contents
	cp -f macos/app/Info.plist $(EXECUTABLE).app/Contents
	mkdir -p $(EXECUTABLE).app/Contents/MacOS
	cp -f $(EXECUTABLE) $(EXECUTABLE).app/Contents/MacOS
	mkdir -p $(EXECUTABLE).app/Contents/Resources
	cp -f Resources/* $(EXECUTABLE).app/Contents/Resources/
	cp -f macos/app/AppIcon.icns $(EXECUTABLE).app/Contents/Resources
endif

$(EXECUTABLE): ouzel $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

$(OBJECTS): src/Prefix.hpp.gch

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

%.hpp.gch: %.hpp
	$(CXX) $(CXXFLAGS) $< -o $@

.PHONY: ouzel
ouzel:
	$(MAKE) -f external/ouzel/build/Makefile platform=$(platform) $(target)

.PHONY: clean
clean:
	$(MAKE) -f external/ouzel/build/Makefile clean
ifeq ($(platform),windows)
	-del /f /q $(EXECUTABLE).exe *.o *.js.mem *.js *.hpp.gch
else
	$(RM) $(EXECUTABLE) *.o *.js.mem *.js *.hpp.gch $(EXECUTABLE).exe
	$(RM) -r $(EXECUTABLE).app
endif