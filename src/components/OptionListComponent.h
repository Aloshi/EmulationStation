#pragma once

#include "../GuiComponent.h"
#include <vector>
#include <functional>

//Used to display a list of options.
//Can select one or multiple options.

struct ListEntry
{
	std::string name;
	unsigned int color;
	bool selected;
};

class OptionListComponent : public GuiComponent
{
public:
	OptionListComponent(Window* window);

	bool input(InputConfig* config, Input input);
	void render(const Eigen::Affine3f& trans);

	void setClosedCallback(std::function<void(std::vector<const ListEntry*>)> callback);
	
	template<typename T>
	void populate(const std::vector<T>& vec, std::function<ListEntry(const T&)> selector);

	std::vector<const ListEntry*> getSelected();
private:
	void close();

	std::function<void(std::vector<const ListEntry*>)> mClosedCallback;

	std::vector<ListEntry> mEntries;
};

