#pragma once

#include "IList.h"
#include <functional>

struct ComponentListElement
{
	ComponentListElement(const std::shared_ptr<GuiComponent>& cmp = nullptr, bool resize_w = true) : component(cmp), resize_width(resize_w) { };

	std::shared_ptr<GuiComponent> component;
	bool resize_width;
};

struct ComponentListRow
{
	std::vector<ComponentListElement> elements;

	// The input handler is called when the user enters any input while this row is highlighted (including up/down).
	// Return false to let the list try to use it or true if the input has been consumed.
	// If no input handler is supplied (input_handler == nullptr), the default behavior is to forward the input to 
	// the rightmost element in the currently selected row.
	std::function<bool(InputConfig*, Input)> input_handler;
	
	inline void addElement(const std::shared_ptr<GuiComponent>& component, bool resize_width)
	{
		elements.push_back(ComponentListElement(component, resize_width));
	}

	// Utility method for making an input handler for "when the users presses A on this, do func."
	inline void makeAcceptInputHandler(const std::function<void()>& func)
	{
		input_handler = [func](InputConfig* config, Input input) -> bool {
			if(config->isMappedTo("a", input) && input.value != 0)
			{
				func();
				return true;
			}
			return false;
		};
	}
};

class ComponentList : public IList<ComponentListRow, void*>
{
public:
	ComponentList(Window* window);

	void addRow(const ComponentListRow& row);

	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	void render(const Eigen::Affine3f& parentTrans) override;

	void onSizeChanged() override;

protected:
	void onCursorChanged(const CursorState& state) override;

private:
	void updateElementPosition(const ComponentListRow& row);
	void updateElementSize(const ComponentListRow& row);

	float getRowHeight(const ComponentListRow& row) const;
	float getTotalRowHeight() const;

	float mSelectorBarOffset;
	float mCameraOffset;
};
