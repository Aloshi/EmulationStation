#include "guis/GuiSlideshowScreensaverOptions.h"

#include "components/SliderComponent.h"
#include "components/SwitchComponent.h"
#include "guis/GuiTextEditPopup.h"
#include "utils/StringUtil.h"
#include "Settings.h"
#include "Window.h"

GuiSlideshowScreensaverOptions::GuiSlideshowScreensaverOptions(Window* window, const char* title) : GuiScreensaverOptions(window, title)
{
	ComponentListRow row;

	// media duration (seconds)
	auto sss_media_sec = std::make_shared<SliderComponent>(mWindow, 1.f, 60.f, 1.f, "s");
	sss_media_sec->setValue((float)(Settings::getInstance()->getInt("ScreenSaverSwapMediaTimeout") / (1000)));
	addWithLabel(row, "SWAP MEDIA AFTER (SECS)", sss_media_sec);
	addSaveFunc([sss_media_sec] {
		int playNextTimeout = (int)Math::round(sss_media_sec->getValue()) * (1000);
		Settings::getInstance()->setInt("ScreenSaverSwapMediaTimeout", playNextTimeout);
		PowerSaver::updateTimeouts();
	});

	// stretch
	auto sss_stretch = std::make_shared<SwitchComponent>(mWindow);
	sss_stretch->setState(Settings::getInstance()->getBool("SlideshowScreenSaverStretch"));
	addWithLabel(row, "STRETCH MEDIA", sss_stretch);
	addSaveFunc([sss_stretch] {
		Settings::getInstance()->setBool("SlideshowScreenSaverStretch", sss_stretch->getState());
	});

	// background audio file
	auto sss_bg_audio_file = std::make_shared<TextComponent>(mWindow, "", Font::get(FONT_SIZE_SMALL), 0x777777FF);
	addEditableTextComponent(row, "BACKGROUND AUDIO", sss_bg_audio_file, Settings::getInstance()->getString("SlideshowScreenSaverBackgroundAudioFile"));
	addSaveFunc([sss_bg_audio_file] {
		Settings::getInstance()->setString("SlideshowScreenSaverBackgroundAudioFile", sss_bg_audio_file->getValue());
	});

	// media source
	auto sss_custom_source = std::make_shared<SwitchComponent>(mWindow);
	sss_custom_source->setState(Settings::getInstance()->getBool("SlideshowScreenSaverCustomMediaSource"));
	addWithLabel(row, "USE CUSTOM MEDIA", sss_custom_source);
	addSaveFunc([sss_custom_source] { Settings::getInstance()->setBool("SlideshowScreenSaverCustomMediaSource", sss_custom_source->getState()); });

	// custom media directory
	auto sss_media_dir = std::make_shared<TextComponent>(mWindow, "", Font::get(FONT_SIZE_SMALL), 0x777777FF);
	addEditableTextComponent(row, "CUSTOM MEDIA DIR", sss_media_dir, Settings::getInstance()->getString("SlideshowScreenSaverMediaDir"));
	addSaveFunc([sss_media_dir] {
		Settings::getInstance()->setString("SlideshowScreenSaverMediaDir", sss_media_dir->getValue());
	});

	// recurse custom media directory
	auto sss_recurse = std::make_shared<SwitchComponent>(mWindow);
	sss_recurse->setState(Settings::getInstance()->getBool("SlideshowScreenSaverRecurse"));
	addWithLabel(row, "CUSTOM MEDIA DIR RECURSIVE", sss_recurse);
	addSaveFunc([sss_recurse] {
		Settings::getInstance()->setBool("SlideshowScreenSaverRecurse", sss_recurse->getState());
	});

	// custom image filter
	auto sss_image_filter = std::make_shared<TextComponent>(mWindow, "", Font::get(FONT_SIZE_SMALL), 0x777777FF);
	addEditableTextComponent(row, "CUSTOM IMAGE FILTER", sss_image_filter, Settings::getInstance()->getString("SlideshowScreenSaverImageFilter"));
	addSaveFunc([sss_image_filter] {
		Settings::getInstance()->setString("SlideshowScreenSaverImageFilter", sss_image_filter->getValue());
	});

	// custom video filter
	auto sss_video_filter = std::make_shared<TextComponent>(mWindow, "", Font::get(FONT_SIZE_SMALL), 0x777777FF);
	// set y-size >0 on last TextComponent in menu to assure proper fit into available row height
	sss_video_filter->setSize(Vector2f(0, Font::get(FONT_SIZE_SMALL)->getLetterHeight()));
	addEditableTextComponent(row, "CUSTOM VIDEO FILTER", sss_video_filter, Settings::getInstance()->getString("SlideshowScreenSaverVideoFilter"));
	addSaveFunc([sss_video_filter] {
		Settings::getInstance()->setString("SlideshowScreenSaverVideoFilter", sss_video_filter->getValue());
	});
}

GuiSlideshowScreensaverOptions::~GuiSlideshowScreensaverOptions()
{
}

void GuiSlideshowScreensaverOptions::addWithLabel(ComponentListRow row, const std::string label, std::shared_ptr<GuiComponent> component)
{
	row.elements.clear();

	auto lbl = std::make_shared<TextComponent>(mWindow, Utils::String::toUpper(label), Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
	row.addElement(lbl, true); // label

	row.addElement(component, false, true);

	addRow(row);
}
