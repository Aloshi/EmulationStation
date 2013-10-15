#pragma once

#include "../GuiComponent.h"
#include <boost/date_time.hpp>
#include "../resources/Font.h"

class DateTimeComponent : public GuiComponent
{
public:
	DateTimeComponent(Window* window);

	void setValue(const std::string& val) override;
	std::string getValue() const override;

	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	void render(const Eigen::Affine3f& parentTrans) override;

	enum DisplayMode
	{
		DISP_DATE,
		DISP_DATE_TIME,
		DISP_RELATIVE_TO_NOW
	};

	void setDisplayMode(DisplayMode mode);
	void setColor(unsigned int color);
	void setFont(std::shared_ptr<Font> font);

private:
	std::shared_ptr<Font> getFont() const;

	std::string getDisplayString(DisplayMode mode) const;
	DisplayMode getCurrentDisplayMode() const;
	
	void updateTextCache();

	boost::posix_time::ptime mTime;
	boost::posix_time::ptime mTimeBeforeEdit;

	bool mEditing;
	int mEditIndex;
	DisplayMode mDisplayMode;

	int mRelativeUpdateAccumulator;

	std::unique_ptr<TextCache> mTextCache;
	std::vector<Eigen::Vector4f> mCursorBoxes;
	std::shared_ptr<Font> mFont;
};
