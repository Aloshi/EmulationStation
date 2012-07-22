#include "InputManager.h"
#include "GuiComponent.h"
#include <iostream>
#include <fstream>
#include <sstream>

std::vector<GuiComponent*> InputManager::inputVector;
SDL_Event* InputManager::lastEvent = NULL;

std::map<int, InputManager::InputButton> InputManager::joystickButtonMap, InputManager::joystickAxisMap;

int InputManager::deadzone = 32000;

void InputManager::registerComponent(GuiComponent* comp)
{
	inputVector.push_back(comp);
}

void InputManager::unregisterComponent(GuiComponent* comp)
{
	for(unsigned int i = 0; i < inputVector.size(); i++)
	{
		if(inputVector.at(i) == comp)
		{
			inputVector.erase(inputVector.begin() + i);
			break;
		}
	}
}

void InputManager::processEvent(SDL_Event* event)
{
	bool keyDown = false;
	InputButton button;

	lastEvent = event;

	//keyboard events
	if(event->type == SDL_KEYDOWN || event->type == SDL_KEYUP)
	{
		if(event->key.state == SDL_PRESSED)
			keyDown = true;

		//get InputButton from the event
		switch(event->key.keysym.sym)
		{
			case SDLK_LEFT:
				button = LEFT;
				break;
			case SDLK_RIGHT:
				button = RIGHT;
				break;
			case SDLK_UP:
				button = UP;
				break;
			case SDLK_DOWN:
				button = DOWN;
				break;
			case SDLK_RETURN:
				button = BUTTON1;
				break;

			default:
				button = UNKNOWN;
				break;
		}

		//catch emergency quit event
		if(event->key.keysym.sym == SDLK_F4)
		{
			//I have no idea if SDL will delete this event, but we're quitting, so I don't think it really matters
			SDL_Event* quit = new SDL_Event();
			quit->type = SDL_QUIT;
			SDL_PushEvent(quit);
			std::cout << "Pushing quit event\n";
		}
	}else{
		if(event->type == SDL_JOYBUTTONDOWN || event->type == SDL_JOYBUTTONUP) //joystick button events
		{
			if(event->type == SDL_JOYBUTTONDOWN) //defaults to false, so no else
				keyDown = true;

			button = joystickButtonMap[event->jbutton.button];
		}else{
			if(event->type == SDL_JOYHATMOTION)
			{
				//no keyUp for hat movement yet, but this should be easily accomplished by
				//keeping the previous hat movement and checking if it moved from centered to anything else (keyDown) or back to centered (keyUp)
				keyDown = true;
				if(event->jhat.value & SDL_HAT_UP)
					button = UP;
				if(event->jhat.value & SDL_HAT_DOWN)
					button = DOWN;
				if(event->jhat.value & SDL_HAT_LEFT)
					button = LEFT;
				if(event->jhat.value & SDL_HAT_RIGHT)
					button = RIGHT;
			}
		}
	}

	for(unsigned int i = 0; i < inputVector.size(); i++)
	{
		inputVector.at(i)->onInput(button, keyDown);
	}
}

void InputManager::loadConfig(std::string path)
{
	//clear any old config
	joystickButtonMap.clear();
	joystickAxisMap.clear();

	std::ifstream file(path.c_str());

	while(file.good())
	{
		std::string line;
		std::getline(file, line);

		//skip blank lines and comments
		if(line.empty() || line[0] == *"#")
			continue;


		//I know I could probably just read from the file stream directly, but I feel it would be harder to catch errors in a readable way
		std::istringstream stream(line);

		std::string token[3];
		int tokNum = 0;

		while(std::getline(stream, token[tokNum], ' '))
		{
			if(tokNum >= 3)
			{
				std::cerr << "Error - input config line \"" << line << "\" has more than three tokens!\n";
				return;
			}
			tokNum++;
		}


		if(token[0] == "BUTTON")
		{
			joystickButtonMap[atoi(token[1].c_str())] = (InputButton)atoi(token[2].c_str());
		}else if(token[0] == "AXIS")
		{
			joystickAxisMap[atoi(token[1].c_str())] = (InputButton)atoi(token[2].c_str());
		}else{
			std::cerr << "Invalid input type - " << token[0] << "\n";
			return;
		}

	}

	if(SDL_NumJoysticks() > 0)
	{
		SDL_JoystickOpen(0);
	}
}
