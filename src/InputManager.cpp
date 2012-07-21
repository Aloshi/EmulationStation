#include "InputManager.h"
#include "GuiComponent.h"
#include <iostream>

std::vector<GuiComponent*> InputManager::inputVector;

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
	if(event->key.state == SDL_PRESSED)
		keyDown = true;

	//get InputButton from the event
	InputButton button;
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

		//so the compiler doesn't complain
		default:
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


	for(unsigned int i = 0; i < inputVector.size(); i++)
	{
		inputVector.at(i)->onInput(button, keyDown);
	}
}

