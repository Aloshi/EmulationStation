#include "InputManager.h"
#include "GuiComponent.h"
#include <iostream>
#include <fstream>
#include <sstream>

std::vector<GuiComponent*> InputManager::inputVector;
SDL_Event* InputManager::lastEvent = NULL;

std::map<int, InputManager::InputButton> InputManager::joystickButtonMap, InputManager::joystickAxisPosMap, InputManager::joystickAxisNegMap;
std::map<int, int> InputManager::axisState;
InputManager::InputButton InputManager::hatState = InputManager::UNKNOWN;

int InputManager::deadzone = 28000;

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
	InputButton button = UNKNOWN;

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
			case SDLK_ESCAPE:
				button = BUTTON2;
				break;
			case SDLK_F1:
				button = MENU;
				break;

			default:
				button = UNKNOWN;
				break;
		}

		//catch emergency quit event
		if(event->key.keysym.sym == SDLK_F4 && keyDown)
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
				int hat = event->jhat.value;

				if(hat == 0) //centered
				{
					//we need to send a keyUp event for the last hat
					//keyDown is already false
					button = hatState;
				}else{
					keyDown = true;
				}

				if(hat & SDL_HAT_LEFT)
					button = LEFT;
				if(hat & SDL_HAT_RIGHT)
					button = RIGHT;

				if(hat & SDL_HAT_UP)
					button = UP;
				if(hat & SDL_HAT_DOWN)
					button = DOWN;

				if(button == hatState && keyDown)
				{
					//ignore this hat event since the user most likely just made it a diagonal (but it still is using the old direction)
					button = UNKNOWN;
				}else{
					if(hatState != UNKNOWN && keyDown)
					{
						//this will occur if the user went down -> downLeft -> Left or similar
						button = hatState;
						keyDown = false;
						hatState = UNKNOWN;
						processEvent(event);
					}else{
						if(!keyDown)
							hatState = UNKNOWN;
						else
							hatState = button;
					}
				}

				//for debugging hats
				//if(button != UNKNOWN)
				//	std::cout << "hat event, button: " << button << ", keyDown: " << keyDown << "\n";

			}else{
				if(event->type == SDL_JOYAXISMOTION)
				{
					int axis = event->jaxis.axis;
					int value = event->jaxis.value;

					//if this axis was previously not centered, it can only keyUp
					if(axisState[axis] != 0)
					{
						if(abs(value) < deadzone) //if it has indeed centered
						{
							if(axisState[axis] > 0)
								button = joystickAxisPosMap[axis];
							else
								button = joystickAxisNegMap[axis];
							axisState[axis] = 0;
						}
					}else{
						if(value > deadzone)
						{
							//axisPos keyDown
							axisState[axis] = 1;
							keyDown = true;
							button = joystickAxisPosMap[axis];
						}else if(value < -deadzone)
						{
							axisState[axis] = -1;
							keyDown = true;
							button = joystickAxisNegMap[axis];
						}
					}
				}
			}
		}
	}

	for(unsigned int i = 0; i < inputVector.size(); i++)
	{
		inputVector.at(i)->onInput(button, keyDown);
	}
}

void InputManager::loadConfig()
{
	//clear any old config
	joystickButtonMap.clear();
	joystickAxisPosMap.clear();
	joystickAxisNegMap.clear();

	std::string path = getConfigPath();

	std::ifstream file(path.c_str());

	std::string joystickName = "";

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
			tokNum++;

			//JOYNAME can have spaces
			if(tokNum == 1 && token[0] == "JOYNAME")
			{
				std::getline(stream, token[1]);
				break;
			}

			if(tokNum >= 3)
				break;
		}


		if(token[0] == "BUTTON")
		{
			joystickButtonMap[atoi(token[1].c_str())] = (InputButton)atoi(token[2].c_str());
		}else if(token[0] == "AXISPOS")
		{
			joystickAxisPosMap[atoi(token[1].c_str())] = (InputButton)atoi(token[2].c_str());
		}else if(token[0] == "AXISNEG")
		{
			joystickAxisNegMap[atoi(token[1].c_str())] = (InputButton)atoi(token[2].c_str());
		}else if(token[0] == "JOYNAME")
		{
			joystickName = token[1];
		}else{
			std::cerr << "Invalid input type - " << token[0] << "\n";
			return;
		}

	}

	//if any joystick is plugged in
	if(SDL_NumJoysticks() > 0)
	{
		if(!joystickName.empty())
		{
			for(int i = 0; i < SDL_NumJoysticks(); i++)
			{
				if(strcmp(SDL_JoystickName(i), joystickName.c_str()) == 0)
				{
					std::cout << "opening joystick " << joystickName << "\n";
					SDL_JoystickOpen(i);
					break;
				}
			}
		}else{
			SDL_JoystickOpen(0);  //if we don't have a specific joystick in mind, take the first
		}
	}
}

std::string InputManager::getConfigPath()
{
	std::string home = getenv("HOME");
	home += "/.emulationstation/es_input.cfg";
	return home;
}
