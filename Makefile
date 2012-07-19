CC=g++
CFLAGS=-c -Wall
LDFLAGS=-lSDL
SRCSOURCES=main.cpp Renderer.cpp Renderer_draw.cpp GuiComponent.cpp components/GuiTitleScreen.cpp
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
