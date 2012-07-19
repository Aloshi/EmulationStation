#include "GuiComponent.h"
#include "Renderer.h"

GuiComponent::GuiComponent()
{
	Renderer::registerComponent(this);
}

GuiComponent::~GuiComponent()
{
	Renderer::unregisterComponent(this);
}
