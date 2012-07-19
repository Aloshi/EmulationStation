#ifndef _GUICOMPONENT_H_
#define _GUICOMPONENT_H_

class GuiComponent
{
public:
	GuiComponent();
	~GuiComponent();
	virtual void render() { };
	virtual unsigned int getLayer() { return 0; };
};

#endif
