//EmulationStation, a graphical front-end for ROM browsing. Created by Alec "Aloshi" Lofquist.
//http://www.aloshi.com

#include <SDL.h>
#include <iostream>
#include <iomanip>
#include "Renderer.h"
#include "components/GuiGameList.h"
#include "SystemData.h"
#include <boost/filesystem.hpp>
#include "components/GuiDetectDevice.h"
#include "AudioManager.h"
#include "platform.h"
#include "Log.h"
#include "Window.h"
#include "EmulationStation.h"
#include "Settings.h"

#ifdef _RPI_
	#include <bcm_host.h>
#endif

#include <sstream>

namespace fs = boost::filesystem;

bool parseArgs(int argc, char* argv[], unsigned int* width, unsigned int* height)
{
	if(argc > 1)
	{
		for(int i = 1; i < argc; i++)
		{
			if(strcmp(argv[i], "-w") == 0)
			{
				*width = atoi(argv[i + 1]);
				i++; //skip the argument value
			}else if(strcmp(argv[i], "-h") == 0)
			{
				*height = atoi(argv[i + 1]);
				i++; //skip the argument value
			}else if(strcmp(argv[i], "--gamelist-only") == 0)
			{
				Settings::getInstance()->setBool("PARSEGAMELISTONLY", true);
			}else if(strcmp(argv[i], "--ignore-gamelist") == 0)
			{
				Settings::getInstance()->setBool("IGNOREGAMELIST", true);
			}else if(strcmp(argv[i], "--draw-framerate") == 0)
			{
				Settings::getInstance()->setBool("DRAWFRAMERATE", true);
			}else if(strcmp(argv[i], "--no-exit") == 0)
			{
				Settings::getInstance()->setBool("DONTSHOWEXIT", true);
			}else if(strcmp(argv[i], "--debug") == 0)
			{
				Settings::getInstance()->setBool("DEBUG", true);
				Log::setReportingLevel(LogDebug);
			}else if(strcmp(argv[i], "--dimtime") == 0)
			{
				Settings::getInstance()->setInt("DIMTIME", atoi(argv[i + 1]) * 1000);
				i++; //skip the argument value
			}else if(strcmp(argv[i], "--windowed") == 0)
			{
				Settings::getInstance()->setBool("WINDOWED", true);
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
				std::cout << "--debug				even more logging\n";
				std::cout << "--dimtime [seconds]		time to wait before dimming the screen (default 30, use 0 for never)\n";

				#ifdef USE_OPENGL_DESKTOP
					std::cout << "--windowed			not fullscreen\n";
				#endif

				std::cout << "--help				summon a sentient, angry tuba\n\n";
				std::cout << "More information available in README.md.\n";
				return false; //exit after printing help
			}
		}
	}

	return true;
}

bool verifyHomeFolderExists()
{
	//make sure the config directory exists
	std::string home = getHomePath();
	std::string configDir = home + "/.emulationstation";
	if(!fs::exists(configDir))
	{
		std::cout << "Creating config directory \"" << configDir << "\"\n";
		fs::create_directory(configDir);
		if(!fs::exists(configDir))
		{
			std::cerr << "Config directory could not be created!\n";
			return false;
		}
	}

	return true;
}

//called on exit, assuming we get far enough to have the log initialized
void onExit()
{
	Log::close();

	#ifdef _RPI_
		bcm_host_deinit();
	#endif
}

int main(int argc, char* argv[])
{
	unsigned int width = 0;
	unsigned int height = 0;

	if(!parseArgs(argc, argv, &width, &height))
		return 0;

	//if ~/.emulationstation doesn't exist and cannot be created, bail
	if(!verifyHomeFolderExists())
		return 1;

	#ifdef _RPI_
		bcm_host_init();
	#endif

	//start the logger
	Log::open();
	LOG(LogInfo) << "EmulationStation - " << PROGRAM_VERSION_STRING;

	//always close the log and deinit the BCM library on exit
	atexit(&onExit);

	Window window;
	if(!window.init(width, height))
	{
		LOG(LogError) << "Window failed to initialize!";
		return 1;
	}

	//try loading the system config file
	if(!SystemData::loadConfig(SystemData::getConfigPath(), true))
	{
		LOG(LogError) << "Error parsing system config file!";
		return 1;
	}

	//make sure it wasn't empty
	if(SystemData::sSystemVector.size() == 0)
	{
		LOG(LogError) << "No systems found! Does at least one system have a game present? (check that extensions match!)";
		return 1;
	}

	//choose which GUI to open depending on if an input configuration already exists
	if(fs::exists(InputManager::getConfigPath()))
	{
		GuiGameList::create(&window);
	}else{
		window.pushGui(new GuiDetectDevice(&window));
	}

	bool sleeping = false;
	unsigned int timeSinceLastEvent = 0;
	int lastTime = 0;
	bool running = true;

	while(running)
	{
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_JOYHATMOTION:
				case SDL_JOYBUTTONDOWN:
				case SDL_JOYBUTTONUP:
				case SDL_KEYDOWN:
				case SDL_KEYUP:
				case SDL_JOYAXISMOTION:
					if(window.getInputManager()->parseEvent(event))
					{
						sleeping = false;
						timeSinceLastEvent = 0;
					}
					break;
				case SDL_USEREVENT:
					//try to poll input devices, but do not necessarily wake up...
					window.getInputManager()->parseEvent(event);
					break;
				case SDL_QUIT:
					running = false;
					break;
			}
		}

		if(sleeping)
		{
			lastTime = SDL_GetTicks();
			SDL_Delay(1); //this doesn't need to be accurate
			continue;
		}

		int curTime = SDL_GetTicks();
		int deltaTime = curTime - lastTime;
		lastTime = curTime;

		window.update(deltaTime);
		Renderer::swapBuffers(); //swap here so we can read the last screen state during updates (see ImageComponent::copyScreen())
		window.render();

		if(Settings::getInstance()->getBool("DRAWFRAMERATE"))
		{
			static int timeElapsed = 0;
			static int nrOfFrames = 0;
			static std::string fpsString;

			nrOfFrames++;
			timeElapsed += deltaTime;
			//wait until half a second has passed to recalculate fps
			if (timeElapsed >= 500) {
				std::stringstream ss;
				ss << std::fixed << std::setprecision(1) << (1000.0f * (float)nrOfFrames / (float)timeElapsed) << "fps, ";
				ss << std::fixed << std::setprecision(2) << ((float)timeElapsed / (float)nrOfFrames) << "ms";
				fpsString = ss.str();
				nrOfFrames = 0;
				timeElapsed = 0;
			}
			Font::get(*window.getResourceManager(), Font::getDefaultPath(), FONT_SIZE_MEDIUM)->drawText(fpsString, 50, 50, 0x00FF00FF);
		}

		//sleep if we're past our threshold
		//sleeping entails setting a flag to start skipping frames
		//and initially drawing a black semi-transparent rect to dim the screen
		timeSinceLastEvent += deltaTime;
		if(timeSinceLastEvent >= (unsigned int)Settings::getInstance()->getInt("DIMTIME") && Settings::getInstance()->getInt("DIMTIME") != 0)
		{
			sleeping = true;
			timeSinceLastEvent = 0;
			Renderer::drawRect(0, 0, Renderer::getScreenWidth(), Renderer::getScreenHeight(), 0x000000A0);
			Renderer::swapBuffers();
		}

		Log::flush();
	}

	Renderer::deinit();
	SystemData::deleteSystems();

	std::cout << "EmulationStation cleanly shutting down...\n";

	return 0;
}
