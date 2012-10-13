CC=g++
CFLAGS=-c -Wall -I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads -I/usr/include/freetype2 -I/usr/include/SDL -I/usr/include -D_RPI_
LDFLAGS=-L/opt/vc/lib -lbcm_host -lEGL -lGLESv2 -lfreetype -lSDL -lboost_system -lboost_filesystem -lfreeimage -lSDL_mixer
SRCSOURCES=main.cpp Renderer.cpp Renderer_init.cpp Font.cpp Renderer_draw_gl.cpp GuiComponent.cpp InputManager.cpp SystemData.cpp GameData.cpp FolderData.cpp XMLReader.cpp MathExp.cpp components/GuiGameList.cpp components/GuiInputConfig.cpp components/GuiImage.cpp components/GuiMenu.cpp components/GuiTheme.cpp components/GuiFastSelect.cpp components/GuiBox.cpp AudioManager.cpp Sound.cpp pugiXML/pugixml.cpp
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

