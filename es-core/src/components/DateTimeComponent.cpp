#include "components/DateTimeComponent.h"

#include "resources/Font.h"
#include "Renderer.h"
#include "Util.h"
#include <boost/date_time.hpp>

DateTimeComponent::DateTimeComponent(Window* window, DisplayMode dispMode) : GuiComponent(window), 
	mEditing(false), mEditIndex(0), mDisplayMode(dispMode), mRelativeUpdateAccumulator(0), 
	mColor(0x777777FF), mFont(Font::get(FONT_SIZE_SMALL, FONT_PATH_LIGHT)), mUppercase(false), mAutoSize(true)
{
	updateTextCache();
}

void DateTimeComponent::setDisplayMode(DisplayMode mode)
{
	mDisplayMode = mode;
	updateTextCache();
}

bool DateTimeComponent::input(InputConfig* config, Input input)
{
	if(input.value == 0)
		return false;

	if(config->isMappedTo("a", input))
	{
		if(mDisplayMode != DISP_RELATIVE_TO_NOW) //don't allow editing for relative times
			mEditing = !mEditing;

		if(mEditing)
		{
			//started editing
			mTimeBeforeEdit = mTime;

			//initialize to now if unset
			if(mTime == boost::posix_time::not_a_date_time)
			{
				mTime = boost::posix_time::ptime(boost::gregorian::day_clock::local_day());
				updateTextCache();
			}
		}

		return true;
	}

	if(mEditing)
	{
		if(config->isMappedTo("b", input))
		{
			mEditing = false;
			mTime = mTimeBeforeEdit;
			updateTextCache();
			return true;
		}

		int incDir = 0;
		if(config->isMappedTo("up", input) || config->isMappedTo("pageup", input))
			incDir = 1;
		else if(config->isMappedTo("down", input) || config->isMappedTo("pagedown", input))
			incDir = -1;

		if(incDir != 0)
		{
			tm new_tm = boost::posix_time::to_tm(mTime);

			if(mEditIndex == 0)
			{
				new_tm.tm_mon += incDir;

				if(new_tm.tm_mon > 11)
					new_tm.tm_mon = 11;
				else if(new_tm.tm_mon < 0)
					new_tm.tm_mon = 0;
				
			}else if(mEditIndex == 1)
			{
				new_tm.tm_mday += incDir;
				int days_in_month = mTime.date().end_of_month().day().as_number();
				if(new_tm.tm_mday > days_in_month)
					new_tm.tm_mday = days_in_month;
				else if(new_tm.tm_mday < 1)
					new_tm.tm_mday = 1;

			}else if(mEditIndex == 2)
			{
				new_tm.tm_year += incDir;
				if(new_tm.tm_year < 0)
					new_tm.tm_year = 0;
			}

			//validate day
			int days_in_month = boost::gregorian::date((unsigned short)new_tm.tm_year + 1900, (unsigned short)new_tm.tm_mon + 1, 1).end_of_month().day().as_number();
			if(new_tm.tm_mday > days_in_month)
				new_tm.tm_mday = days_in_month;

			mTime = boost::posix_time::ptime_from_tm(new_tm);
			
			updateTextCache();
			return true;
		}

		if(config->isMappedTo("right", input))
		{
			mEditIndex++;
			if(mEditIndex >= (int)mCursorBoxes.size())
				mEditIndex--;
			return true;
		}
		
		if(config->isMappedTo("left", input))
		{
			mEditIndex--;
			if(mEditIndex < 0)
				mEditIndex++;
			return true;
		}
	}

	return GuiComponent::input(config, input);
}

void DateTimeComponent::update(int deltaTime)
{
	if(mDisplayMode == DISP_RELATIVE_TO_NOW)
	{
		mRelativeUpdateAccumulator += deltaTime;
		if(mRelativeUpdateAccumulator > 1000)
		{
			mRelativeUpdateAccumulator = 0;
			updateTextCache();
		}
	}

	GuiComponent::update(deltaTime);
}

void DateTimeComponent::render(const Transform4x4f& parentTrans)
{
	Transform4x4f trans = parentTrans * getTransform();

	if(mTextCache)
	{
		// vertically center
		Vector3f off(0, (mSize.y() - mTextCache->metrics.size.y()) / 2, 0);
		trans.translate(off);
		trans.round();

		Renderer::setMatrix(trans);

		std::shared_ptr<Font> font = getFont();

		mTextCache->setColor((mColor & 0xFFFFFF00) | getOpacity());
		font->renderTextCache(mTextCache.get());

		if(mEditing)
		{
			if(mEditIndex >= 0 && (unsigned int)mEditIndex < mCursorBoxes.size())
			{
				Renderer::drawRect((int)mCursorBoxes[mEditIndex][0], (int)mCursorBoxes[mEditIndex][1], 
					(int)mCursorBoxes[mEditIndex][2], (int)mCursorBoxes[mEditIndex][3], 0x00000022);
			}
		}
	}
}

void DateTimeComponent::setValue(const std::string& val)
{
	mTime = string_to_ptime(val);
	updateTextCache();
}

std::string DateTimeComponent::getValue() const
{
	return boost::posix_time::to_iso_string(mTime);
}

DateTimeComponent::DisplayMode DateTimeComponent::getCurrentDisplayMode() const
{
	/*if(mEditing)
	{
		if(mDisplayMode == DISP_RELATIVE_TO_NOW)
		{
			//TODO: if time component == 00:00:00, return DISP_DATE, else return DISP_DATE_TIME
			return DISP_DATE;
		}
	}*/

	return mDisplayMode;
}

std::string DateTimeComponent::getDisplayString(DisplayMode mode) const
{
	std::string fmt;
	switch(mode)
	{
	case DISP_DATE:
		fmt = "%m/%d/%Y";
		break;
	case DISP_DATE_TIME:
		fmt = "%m/%d/%Y %H:%M:%S";
		break;
	case DISP_RELATIVE_TO_NOW:
		{
			//relative time
			using namespace boost::posix_time;

			if(mTime == not_a_date_time)
				return "never";

			ptime now = second_clock::universal_time();
			time_duration dur = now - mTime;

			if(dur < seconds(2))
				return "just now";
			if(dur < seconds(60))
				return std::to_string((long long)dur.seconds()) + " secs ago";
			if(dur < minutes(60))
				return std::to_string((long long)dur.minutes()) + " min" + (dur < minutes(2) ? "" : "s") + " ago";
			if(dur < hours(24))
				return std::to_string((long long)dur.hours()) + " hour" + (dur < hours(2) ? "" : "s") + " ago";

			long long days = (long long)(dur.hours() / 24);
			return std::to_string(days) + " day" + (days < 2 ? "" : "s") + " ago";
		}
		break;
	}
	
	if(mTime == boost::posix_time::not_a_date_time)
		return "unknown";

	boost::posix_time::time_facet* facet = new boost::posix_time::time_facet();
	facet->format(fmt.c_str());
	std::locale loc(std::locale::classic(), facet);

	std::stringstream ss;
	ss.imbue(loc);
	ss << mTime;
	return ss.str();
}

std::shared_ptr<Font> DateTimeComponent::getFont() const
{
	if(mFont)
		return mFont;

	return Font::get(FONT_SIZE_MEDIUM);
}

void DateTimeComponent::updateTextCache()
{
	DisplayMode mode = getCurrentDisplayMode();
	const std::string dispString = mUppercase ? strToUpper(getDisplayString(mode)) : getDisplayString(mode);
	std::shared_ptr<Font> font = getFont();
	mTextCache = std::unique_ptr<TextCache>(font->buildTextCache(dispString, 0, 0, mColor));

	if(mAutoSize)
	{
		mSize = mTextCache->metrics.size;

		mAutoSize = false;
		if(getParent())
			getParent()->onSizeChanged();
	}

	//set up cursor positions
	mCursorBoxes.clear();

	if(dispString.empty() || mode == DISP_RELATIVE_TO_NOW)
		return;

	//month
	Vector2f start(0, 0);
	Vector2f end = font->sizeText(dispString.substr(0, 2));
	Vector2f diff = end - start;
	mCursorBoxes.push_back(Vector4f(start[0], start[1], diff[0], diff[1]));

	//day
	start[0] = font->sizeText(dispString.substr(0, 3)).x();
	end = font->sizeText(dispString.substr(0, 5));
	diff = end - start;
	mCursorBoxes.push_back(Vector4f(start[0], start[1], diff[0], diff[1]));

	//year
	start[0] = font->sizeText(dispString.substr(0, 6)).x();
	end = font->sizeText(dispString.substr(0, 10));
	diff = end - start;
	mCursorBoxes.push_back(Vector4f(start[0], start[1], diff[0], diff[1]));

	//if mode == DISP_DATE_TIME do times too but I don't wanna do the logic for editing times because no one will ever use it so screw it
}

void DateTimeComponent::setColor(unsigned int color)
{
	mColor = color;
	if(mTextCache)
		mTextCache->setColor(color);
}

void DateTimeComponent::setFont(std::shared_ptr<Font> font)
{
	mFont = font;
	updateTextCache();
}

void DateTimeComponent::onSizeChanged()
{
	mAutoSize = false;
	updateTextCache();
}

void DateTimeComponent::setUppercase(bool uppercase)
{
	mUppercase = uppercase;
	updateTextCache();
}

void DateTimeComponent::applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties)
{
	const ThemeData::ThemeElement* elem = theme->getElement(view, element, "datetime");
	if(!elem)
		return;

	// We set mAutoSize BEFORE calling GuiComponent::applyTheme because it calls
	// setSize(), which will call updateTextCache(), which will reset mSize if 
	// mAutoSize == true, ignoring the theme's value.
	if(properties & ThemeFlags::SIZE)
		mAutoSize = !elem->has("size");

	GuiComponent::applyTheme(theme, view, element, properties);

	using namespace ThemeFlags;

	if(properties & COLOR && elem->has("color"))
		setColor(elem->get<unsigned int>("color"));

	if(properties & FORCE_UPPERCASE && elem->has("forceUppercase"))
		setUppercase(elem->get<bool>("forceUppercase"));

	setFont(Font::getFromTheme(elem, properties, mFont));
}
