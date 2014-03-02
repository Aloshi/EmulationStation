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
	std::function<bool(InputConfig*, Input)> input_handler;
	
	inline void addElement(const std::shared_ptr<GuiComponent>& component, bool resize_width)
	{
		elements.push_back(ComponentListElement(component, resize_width));
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
