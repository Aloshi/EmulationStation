#ifndef _RENDERER_H_
#define _RENDERER_H_

#define LAYER_COUNT 3

#define BIT(x) (1 << (x))

#include "GuiComponent.h"
#include <vector>
namespace Renderer
{
	void registerComponent(GuiComponent* comp);
	void unregisterComponent(GuiComponent* comp);

	void render();

	std::vector<GuiComponent*> renderVector;
}

#endif
