CC=g++
CFLAGS=-c -Wall
LDFLAGS=-lSDL -lSDL_ttf -lboost_system -lboost_filesystem
SRCSOURCES=main.cpp Renderer.cpp Renderer_draw.cpp GuiComponent.cpp InputManager.cpp SystemData.cpp GameData.cpp components/GuiTitleScreen.cpp components/GuiList.cpp components/GuiGameList.cpp
SOURCES=$(addprefix src/,$(SRCSOURCES))
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=emulationstation

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf src/*o src/components/*o $(EXECUTABLE)
