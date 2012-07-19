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

void Renderer::render()
{
	for(unsigned int layer = 0; layer < LAYER_COUNT; layer++)
	{
		unsigned int layerBit = BIT(layer);
		for(unsigned int i = 0; i < renderVector.size(); i++)
		{
			if(renderVector.at(i)->getLayer() & layerBit)
				renderVector.at(i)->render();
		}
	}
}

