//EmulationStation, a graphical front-end for ROM browsing. Created by Alec "Aloshi" Lofquist.

#include <iostream>
#include "Renderer.h"
#include "components/GuiGameList.h"
#include "SystemData.h"
#include <boost/filesystem.hpp>
#include "components/GuiInputConfig.h"
#include <SDL.h>
#include "AudioManager.h"
#include "platform.h"

#ifdef _RPI_
	#include <bcm_host.h>
#endif

#include <sstream>

//these can be set by command-line arguments
bool PARSEGAMELISTONLY = false;
bool IGNOREGAMELIST = false;
bool DRAWFRAMERATE = false;
bool DONTSHOWEXIT = false;
bool DEBUG = false;

namespace fs = boost::filesystem;

int main(int argc, char* argv[])
{
	unsigned int width = 0;
	unsigned int height = 0;
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
			}else if(strcmp(argv[i], "--draw-framerate") == 0)
			{
				DRAWFRAMERATE = true;
			}else if(strcmp(argv[i], "--no-exit") == 0)
			{
				DONTSHOWEXIT = true;
			}else if(strcmp(argv[i], "--debug") == 0)
			{
				DEBUG = true;
			}else if(strcmp(argv[i], "--help") == 0)
			{
				std::cout << "EmulationStation, a graphical front-end for ROM browsing.\n";
				std::cout << "Command line arguments:\n";
				std::cout << "-w [width in pixels]		set screen width\n";
				std::cout << "-h [height in pixels]		set screen height\n";
				std::cout << "--gamelist-only			skip automatic game detection, only read from gamelist.xml\n";
				std::cout << "--ignore-gamelist		ignore the gamelist (useful for troubleshooting)\n";
				std::cout << "--draw-framerate		display the framerate\n";
				std::cout << "--no-exit			don't show the exit option in the menu\n";
				std::cout << "--debug				print additional output to console\n";
				std::cout << "--help				summon a sentient, angry tuba\n\n";
				std::cout << "More information available in README.md.\n";
				return 0;
			}
		}
	}


	#ifdef _RPI_
		bcm_host_init();
	#endif

	bool running = true;

	//the renderer also takes care of setting up SDL for input and sound
	bool renderInit = Renderer::init(width, height);
	if(!renderInit)
	{
		std::cerr << "Error initializing renderer!\n";
		return 1;
	}


	//initialize audio
	AudioManager::init();


	SDL_JoystickEventState(SDL_ENABLE);

	//make sure the config directory exists
	std::string home = getenv("HOME");
	std::string configDir = home + "/.emulationstation";
	if(!fs::exists(configDir))
	{
		std::cout << "Creating config directory \"" << configDir << "\"\n";
		fs::create_directory(configDir);
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
			std::cerr << "Does at least one system have a game presesnt?\n";
			running = false;
		}else{
			//choose which GUI to open depending on Input configuration
			if(fs::exists(InputManager::getConfigPath()))
			{
				if(DEBUG)
					std::cout << "Found input config in " << InputManager::getConfigPath() << "\n";

				//an input config already exists - load it and proceed to the gamelist as usual.
				InputManager::loadConfig();
				GuiGameList::create();
			}else{
				if(DEBUG)
					std::cout << "SDL_NumJoysticks() reports " << SDL_NumJoysticks() << " present.\n";

				//if no input.cfg is present, but a joystick is connected, launch the input config GUI
				if(SDL_NumJoysticks() > 0)
				{
					if(DEBUG)
						std::cout << "	at least one joystick detected, launching config GUI...\n";

					new GuiInputConfig();
				}else{
					if(DEBUG)
						std::cout << "	no joystick detected, ignoring...\n";

					GuiGameList::create();
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

		GuiComponent::processTicks(deltaTime);
		Renderer::render();

		if(DRAWFRAMERATE)
		{
			float framerate = 1/((float)deltaTime)*1000;
			std::stringstream ss;
			ss << framerate;
			std::string fps;
			ss >> fps;
			Renderer::drawText(fps, 50, 50, 0x00FF00FF, Renderer::getDefaultFont(Renderer::MEDIUM));
		}


		Renderer::swapBuffers();
	}


	AudioManager::deinit();
	Renderer::deleteAll();
	Renderer::deinit();
	SystemData::deleteSystems();

	std::cout << "EmulationStation cleanly shutting down...\n";

	SDL_Quit();

	#ifdef _RPI_
		bcm_host_deinit();
	#endif

	return 0;
}
