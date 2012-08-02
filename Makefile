CC=g++
CFLAGS=-c -Wall
LDFLAGS=-lSDL -lSDL_ttf -lSDL_image -lboost_system -lboost_filesystem
SRCSOURCES=main.cpp Renderer.cpp Renderer_draw.cpp GuiComponent.cpp InputManager.cpp SystemData.cpp GameData.cpp FolderData.cpp XMLReader.cpp components/GuiGameList.cpp components/GuiInputConfig.cpp components/GuiImage.cpp components/GuiMenu.cpp pugiXML/pugixml.cpp
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
