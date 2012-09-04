#include "Renderer.h"
#include "GuiComponent.h"

std::vector<GuiComponent*> renderVector;

void Renderer::registerComponent(GuiComponent* comp)
{
	renderVector.push_back(comp);
}

void Renderer::unregisterComponent(GuiComponent* comp)
{
	for(unsigned int i = 0; i < renderVector.size(); i++)
	{
		if(renderVector.at(i) == comp)
		{
			renderVector.erase(renderVector.begin() + i);
			break;
		}
	}
}

void Renderer::deleteAll()
{
	for(unsigned int i = 0; i < renderVector.size(); i++)
	{
		delete renderVector.at(i);
	}
	renderVector.clear();
}

void Renderer::render()
{
	for(unsigned int i = 0; i < renderVector.size(); i++)
	{
		renderVector.at(i)->render();
	}
}

void Renderer::onInit()
{
	for(unsigned int i = 0; i < renderVector.size(); i++)
	{
		renderVector.at(i)->init();
	}
}

void Renderer::onDeinit()
{
	for(unsigned int i = 0; i < renderVector.size(); i++)
	{
		renderVector.at(i)->deinit();
	}
}
