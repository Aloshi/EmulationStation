#include <iostream>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include "Renderer.h"
#include "components/GuiGameList.h"
#include "SystemData.h"
#include <boost/filesystem.hpp>
#include "components/GuiInputConfig.h"

bool PARSEGAMELISTONLY = false;
bool IGNOREGAMELIST = false;

#ifdef DRAWFRAMERATE
	float FRAMERATE = 0;
#endif

namespace fs = boost::filesystem;

int main(int argc, char* argv[])
{
	bool running = true;

	//by the way, if anyone ever tries to port this to a different renderer but leave SDL as input -
	//KEEP INITIALIZING VIDEO. It starts SDL's event system, and without it, input won't work.
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0)
	{
		std::cerr << "Error - could not initialize SDL!\n";
		std::cerr << "	" << SDL_GetError() << "\n";
		std::cerr << "Are you in the 'video' and 'input' groups? Are you running with X closed?\n";
		return 1;
	}
	if(TTF_Init() != 0)
	{
		std::cerr << "Error - could not initialize SDL_ttf!\n";
		std::cerr << "	" << TTF_GetError() << "\n";
		return 1;
	}


	int width = 0;
	int height = 0;
	if(argc > 1)
	{
		for(int i = 1; i < argc; i++)
		{
			if(strcmp(argv[i], "-w") == 0)
			{
				width = atoi(argv[i + 1]);
				i++;
			}else if(strcmp(argv[i], "-h") == 0)
			{
				height = atoi(argv[i + 1]);
				i++;
			}else if(strcmp(argv[i], "--gamelist-only") == 0)
			{
				PARSEGAMELISTONLY = true;
			}else if(strcmp(argv[i], "--ignore-gamelist") == 0)
			{
				IGNOREGAMELIST = true;
			}
		}
	}

	Renderer::screen = SDL_SetVideoMode(width, height, 16, SDL_SWSURFACE); //| SDL_FULLSCREEN);

	if(Renderer::screen == NULL)
	{
		std::cerr << "Error - could not set video mode!\n";
		std::cerr << "	" << SDL_GetError() << "\n";
		std::cerr << "\nYou may want to try using -w and -h to specify a resolution.\n";
		return 1;
	}else{
		std::cout << "Video mode is " << Renderer::screen->w << "x" << Renderer::screen->h << "\n";
	}

	SDL_ShowCursor(false);
	SDL_JoystickEventState(SDL_ENABLE);



	//make sure the config directory exists
	std::string home = getenv("HOME");
	std::string configDir = home + "/.emulationstation";
	if(!fs::exists(configDir))
	{
		std::cout << "Creating config directory \"" << configDir << "\"\n";
		fs::create_directory(configDir);
	}

	//check if there are config files in the old places, and if so, move them to the new directory
	std::string oldSysPath = home + "/.es_systems.cfg";
	std::string oldInpPath = home + "/.es_input.cfg";
	if(fs::exists(oldSysPath))
	{
		std::cout << "Moving old system config file " << oldSysPath << " to new path at " << SystemData::getConfigPath() << "\n";
		fs::copy_file(oldSysPath, SystemData::getConfigPath());
		fs::remove(oldSysPath);
	}
	if(fs::exists(oldInpPath))
	{
		std::cout << "Deleting old input config file\n";
		fs::remove(oldInpPath);
	}



	//try loading the system config file
	if(!fs::exists(SystemData::getConfigPath())) //if it doesn't exist, create the example and quit
	{
		std::cerr << "A system config file in " << SystemData::getConfigPath() << " was not found. An example will be created.\n";
		SystemData::writeExampleConfig();
		std::cerr << "Set it up, then re-run EmulationStation.\n";
		running = false;
	}else{
		SystemData::loadConfig();

		if(SystemData::sSystemVector.size() == 0) //if it exists but was empty, notify the user and quit
		{
			std::cerr << "A system config file in " << SystemData::getConfigPath() << " was found, but contained no systems.\n";
			std::cerr << "You should probably go read that, or delete it.\n";
			running = false;
		}else{

			bool useDetail = false;

			//see if any systems had gamelists present, if so we'll use the detailed GuiGameList
			if(!IGNOREGAMELIST)
			{
				for(unsigned int i = 0; i < SystemData::sSystemVector.size(); i++)
				{
					if(SystemData::sSystemVector.at(i)->hasGamelist())
					{
						useDetail = true;
						break;
					}
				}
			}

			//choose which Gui to open up
			if(fs::exists(InputManager::getConfigPath()))
			{
				InputManager::loadConfig();
				new GuiGameList(useDetail);
			}else{
				if(SDL_NumJoysticks() > 0)
				{
					new GuiInputConfig();
				}else{
					std::cout << "Note - it looks like you have no joysticks connected.\n";
					new GuiGameList(useDetail);
				}
			}
		}
	}

	int lastTime = 0;
	while(running)
	{
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_JOYHATMOTION:
				case SDL_JOYAXISMOTION:
				case SDL_JOYBUTTONDOWN:
				case SDL_JOYBUTTONUP:
				case SDL_KEYDOWN:
				case SDL_KEYUP:
					InputManager::processEvent(&event);
					break;

				case SDL_QUIT:
					running = false;
					break;
			}
		}

		int curTime = SDL_GetTicks();
		int deltaTime = curTime - lastTime;
		lastTime = curTime;

		#ifdef DRAWFRAMERATE
			FRAMERATE = 1/((float)deltaTime)*1000;
		#endif

		GuiComponent::processTicks(deltaTime);

		Renderer::render();
		SDL_Flip(Renderer::screen);
	}

	Renderer::deleteAll();
	SystemData::deleteSystems();

	std::cout << "EmulationStation cleanly shutting down...\n";

	TTF_Quit();
	SDL_Quit();
	return 0;
}
