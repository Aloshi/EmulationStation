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

int lastTime = 0;

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
				i++; //skip the argument value
			}else if(strcmp(argv[i], "-h") == 0)
			{
				height = atoi(argv[i + 1]);
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
#ifdef _RPI_
			}else if(strcmp(argv[i], "--pihdmisleep") == 0)
			{
				Settings::getInstance()->setBool("PIHDMISLEEP", true);
#endif
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
#ifdef _RPI_
				std::cout << "--pihdmisleep			put the hdmi display into standby instead of dimming the screen.  This restarts ES on wake.\n";
#endif

				#ifdef USE_OPENGL_DESKTOP
					std::cout << "--windowed			not fullscreen\n";
				#endif

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

	//make sure the config directory exists
	std::string home = getHomePath();
	std::string configDir = home + "/.emulationstation";
	if(!fs::exists(configDir))
	{
		std::cout << "Creating config directory \"" << configDir << "\"\n";
		fs::create_directory(configDir);
	}

	//start the logger
	Log::open();
	LOG(LogInfo) << "EmulationStation - " << PROGRAM_VERSION_STRING;

	//the renderer also takes care of setting up SDL for input and sound
	bool renderInit = Renderer::init(width, height);
	if(!renderInit)
	{
		std::cerr << "Error initializing renderer!\n";
		Log::close();
		return 1;
	}

	Window window; //don't call Window.init() because we manually pass the resolution to Renderer::init
	window.getInputManager()->init();

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
			std::cerr << "Does at least one system have a game present?\n";
			running = false;
		}else{
			//choose which GUI to open depending on Input configuration
			if(fs::exists(InputManager::getConfigPath()))
			{
				//an input config already exists - we have input, proceed to the gamelist as usual.
				GuiGameList::create(&window);
			}else{
				window.pushGui(new GuiDetectDevice(&window));
			}
		}
	}

	unsigned int timeSinceLastEvent = 0;
	bool sleeping = false;

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
#ifdef _RPI_
					// window has been deinitted, so we
					// can't use it's input manager to parse
					// the event, so do this first.
					if(sleeping && Settings::getInstance()->getBool("PIHDMISLEEP")) {
						LOG(LogInfo) << "Waking HDMI";
						window.wake();

						// Ignore the waking event
						sleeping = false;
						timeSinceLastEvent = 0;
						break;
					}
#endif
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
#ifdef _RPI_
			if(Settings::getInstance()->getBool("PIHDMISLEEP")) {
				LOG(LogInfo) << "Sleeping HDMI";
				window.sleep();
			} else {
#endif
			Renderer::drawRect(0, 0, Renderer::getScreenWidth(), Renderer::getScreenHeight(), 0x000000A0);
			Renderer::swapBuffers();
#ifdef _RPI_
			}
#endif
		}

		Log::flush();
	}

	Renderer::deinit();
	SystemData::deleteSystems();

	std::cout << "EmulationStation cleanly shutting down...\n";

	Log::close();

	#ifdef _RPI_
		bcm_host_deinit();
	#endif

	return 0;
}
