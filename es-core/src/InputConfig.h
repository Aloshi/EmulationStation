#pragma once
#ifndef ES_CORE_INPUT_CONFIG_H
#define ES_CORE_INPUT_CONFIG_H

#ifdef HAVE_LIBCEC
#include <libcec/cectypes.h>
#endif // HAVE_LIBCEC
#include <SDL_joystick.h>
#include <SDL_keyboard.h>
#include <map>
#include <sstream>
#include <vector>

namespace pugi { class xml_node; }

#define DEVICE_KEYBOARD -1
#define DEVICE_CEC      -2

enum InputType
{
	TYPE_AXIS,
	TYPE_BUTTON,
	TYPE_HAT,
	TYPE_KEY,
	TYPE_CEC_BUTTON,
	TYPE_COUNT
};

struct Input
{
public:
	int device;
	InputType type;
	int id;
	int value;
	bool configured;

	Input()
	{
		device = DEVICE_KEYBOARD;
		configured = false;
		id = -1;
		value = -999;
		type = TYPE_COUNT;
	}

	Input(int dev, InputType t, int i, int val, bool conf) : device(dev), type(t), id(i), value(val), configured(conf)
	{
	}

	std::string getHatDir(int val)
	{
		if(val & SDL_HAT_UP)
			return "up";
		else if(val & SDL_HAT_DOWN)
			return "down";
		else if(val & SDL_HAT_LEFT)
			return "left";
		else if(val & SDL_HAT_RIGHT)
			return "right";
		return "neutral?";
	}

	std::string getCECButtonName(int keycode)
	{

#ifdef HAVE_LIBCEC
		switch(keycode)
		{
			case CEC::CEC_USER_CONTROL_CODE_SELECT:                      { return "Select";                      } break;
			case CEC::CEC_USER_CONTROL_CODE_UP:                          { return "Up";                          } break;
			case CEC::CEC_USER_CONTROL_CODE_DOWN:                        { return "Down";                        } break;
			case CEC::CEC_USER_CONTROL_CODE_LEFT:                        { return "Left";                        } break;
			case CEC::CEC_USER_CONTROL_CODE_RIGHT:                       { return "Right";                       } break;
			case CEC::CEC_USER_CONTROL_CODE_RIGHT_UP:                    { return "Right-Up";                    } break;
			case CEC::CEC_USER_CONTROL_CODE_RIGHT_DOWN:                  { return "Left-Down";                   } break;
			case CEC::CEC_USER_CONTROL_CODE_LEFT_UP:                     { return "Left-Up";                     } break;
			case CEC::CEC_USER_CONTROL_CODE_LEFT_DOWN:                   { return "Left-Down";                   } break;
			case CEC::CEC_USER_CONTROL_CODE_ROOT_MENU:                   { return "Root-Menu";                   } break;
			case CEC::CEC_USER_CONTROL_CODE_SETUP_MENU:                  { return "Setup-Menu";                  } break;
			case CEC::CEC_USER_CONTROL_CODE_CONTENTS_MENU:               { return "Contents-Menu";               } break;
			case CEC::CEC_USER_CONTROL_CODE_FAVORITE_MENU:               { return "Favorite-Menu";               } break;
			case CEC::CEC_USER_CONTROL_CODE_EXIT:                        { return "Exit";                        } break;
			case CEC::CEC_USER_CONTROL_CODE_TOP_MENU:                    { return "Top-Menu";                    } break;
			case CEC::CEC_USER_CONTROL_CODE_DVD_MENU:                    { return "DVD-Menu";                    } break;
			case CEC::CEC_USER_CONTROL_CODE_NUMBER_ENTRY_MODE:           { return "Number-Entry-Mode";           } break;
			case CEC::CEC_USER_CONTROL_CODE_NUMBER11:                    { return "Number 11";                   } break;
			case CEC::CEC_USER_CONTROL_CODE_NUMBER12:                    { return "Number 12";                   } break;
			case CEC::CEC_USER_CONTROL_CODE_NUMBER0:                     { return "Number 0";                    } break;
			case CEC::CEC_USER_CONTROL_CODE_NUMBER1:                     { return "Number 1";                    } break;
			case CEC::CEC_USER_CONTROL_CODE_NUMBER2:                     { return "Number 2";                    } break;
			case CEC::CEC_USER_CONTROL_CODE_NUMBER3:                     { return "Number 3";                    } break;
			case CEC::CEC_USER_CONTROL_CODE_NUMBER4:                     { return "Number 4";                    } break;
			case CEC::CEC_USER_CONTROL_CODE_NUMBER5:                     { return "Number 5";                    } break;
			case CEC::CEC_USER_CONTROL_CODE_NUMBER6:                     { return "Number 6";                    } break;
			case CEC::CEC_USER_CONTROL_CODE_NUMBER7:                     { return "Number 7";                    } break;
			case CEC::CEC_USER_CONTROL_CODE_NUMBER8:                     { return "Number 8";                    } break;
			case CEC::CEC_USER_CONTROL_CODE_NUMBER9:                     { return "Number 9";                    } break;
			case CEC::CEC_USER_CONTROL_CODE_DOT:                         { return "Dot";                         } break;
			case CEC::CEC_USER_CONTROL_CODE_ENTER:                       { return "Enter";                       } break;
			case CEC::CEC_USER_CONTROL_CODE_CLEAR:                       { return "Clear";                       } break;
			case CEC::CEC_USER_CONTROL_CODE_NEXT_FAVORITE:               { return "Next-Favorite";               } break;
			case CEC::CEC_USER_CONTROL_CODE_CHANNEL_UP:                  { return "Channel-Up";                  } break;
			case CEC::CEC_USER_CONTROL_CODE_CHANNEL_DOWN:                { return "Channel-Down";                } break;
			case CEC::CEC_USER_CONTROL_CODE_PREVIOUS_CHANNEL:            { return "Previous-Channel";            } break;
			case CEC::CEC_USER_CONTROL_CODE_SOUND_SELECT:                { return "Sound-Select";                } break;
			case CEC::CEC_USER_CONTROL_CODE_INPUT_SELECT:                { return "Input-Select";                } break;
			case CEC::CEC_USER_CONTROL_CODE_DISPLAY_INFORMATION:         { return "Display-Information";         } break;
			case CEC::CEC_USER_CONTROL_CODE_HELP:                        { return "Help";                        } break;
			case CEC::CEC_USER_CONTROL_CODE_PAGE_UP:                     { return "Page-Up";                     } break;
			case CEC::CEC_USER_CONTROL_CODE_PAGE_DOWN:                   { return "Page-Down";                   } break;
			case CEC::CEC_USER_CONTROL_CODE_POWER:                       { return "Power";                       } break;
			case CEC::CEC_USER_CONTROL_CODE_VOLUME_UP:                   { return "Volume-Up";                   } break;
			case CEC::CEC_USER_CONTROL_CODE_VOLUME_DOWN:                 { return "Volume-Down";                 } break;
			case CEC::CEC_USER_CONTROL_CODE_MUTE:                        { return "Mute";                        } break;
			case CEC::CEC_USER_CONTROL_CODE_PLAY:                        { return "Play";                        } break;
			case CEC::CEC_USER_CONTROL_CODE_STOP:                        { return "Stop";                        } break;
			case CEC::CEC_USER_CONTROL_CODE_PAUSE:                       { return "Pause";                       } break;
			case CEC::CEC_USER_CONTROL_CODE_RECORD:                      { return "Record";                      } break;
			case CEC::CEC_USER_CONTROL_CODE_REWIND:                      { return "Rewind";                      } break;
			case CEC::CEC_USER_CONTROL_CODE_FAST_FORWARD:                { return "Fast-Forward";                } break;
			case CEC::CEC_USER_CONTROL_CODE_EJECT:                       { return "Eject";                       } break;
			case CEC::CEC_USER_CONTROL_CODE_FORWARD:                     { return "Forward";                     } break;
			case CEC::CEC_USER_CONTROL_CODE_BACKWARD:                    { return "Backward";                    } break;
			case CEC::CEC_USER_CONTROL_CODE_STOP_RECORD:                 { return "Stop-Record";                 } break;
			case CEC::CEC_USER_CONTROL_CODE_PAUSE_RECORD:                { return "Pause-Record";                } break;
			case CEC::CEC_USER_CONTROL_CODE_ANGLE:                       { return "Angle";                       } break;
			case CEC::CEC_USER_CONTROL_CODE_SUB_PICTURE:                 { return "Sub-Picture";                 } break;
			case CEC::CEC_USER_CONTROL_CODE_VIDEO_ON_DEMAND:             { return "Video-On-Demand";             } break;
			case CEC::CEC_USER_CONTROL_CODE_ELECTRONIC_PROGRAM_GUIDE:    { return "Electronic-Program-Guide";    } break;
			case CEC::CEC_USER_CONTROL_CODE_TIMER_PROGRAMMING:           { return "Timer-Programming";           } break;
			case CEC::CEC_USER_CONTROL_CODE_INITIAL_CONFIGURATION:       { return "Initial-Configuration";       } break;
			case CEC::CEC_USER_CONTROL_CODE_SELECT_BROADCAST_TYPE:       { return "Select-Broadcast-Type";       } break;
			case CEC::CEC_USER_CONTROL_CODE_SELECT_SOUND_PRESENTATION:   { return "Select-Sound-Presentation";   } break;
			case CEC::CEC_USER_CONTROL_CODE_PLAY_FUNCTION:               { return "Play-Function";               } break;
			case CEC::CEC_USER_CONTROL_CODE_PAUSE_PLAY_FUNCTION:         { return "Pause-Play-Function";         } break;
			case CEC::CEC_USER_CONTROL_CODE_RECORD_FUNCTION:             { return "Record-Function";             } break;
			case CEC::CEC_USER_CONTROL_CODE_PAUSE_RECORD_FUNCTION:       { return "Pause-Record-Function";       } break;
			case CEC::CEC_USER_CONTROL_CODE_STOP_FUNCTION:               { return "Stop-Function";               } break;
			case CEC::CEC_USER_CONTROL_CODE_MUTE_FUNCTION:               { return "Mute-Function";               } break;
			case CEC::CEC_USER_CONTROL_CODE_RESTORE_VOLUME_FUNCTION:     { return "Restore-Volume-Function";     } break;
			case CEC::CEC_USER_CONTROL_CODE_TUNE_FUNCTION:               { return "Tune-Function";               } break;
			case CEC::CEC_USER_CONTROL_CODE_SELECT_MEDIA_FUNCTION:       { return "Select-Media-Function";       } break;
			case CEC::CEC_USER_CONTROL_CODE_SELECT_AV_INPUT_FUNCTION:    { return "Select-AV-Input-function";    } break;
			case CEC::CEC_USER_CONTROL_CODE_SELECT_AUDIO_INPUT_FUNCTION: { return "Select-Audio-Input-Function"; } break;
			case CEC::CEC_USER_CONTROL_CODE_POWER_TOGGLE_FUNCTION:       { return "Power-Toggle-Function";       } break;
			case CEC::CEC_USER_CONTROL_CODE_POWER_OFF_FUNCTION:          { return "Power-Off-Function";          } break;
			case CEC::CEC_USER_CONTROL_CODE_POWER_ON_FUNCTION:           { return "Power-On-Function";           } break;
			case CEC::CEC_USER_CONTROL_CODE_F1_BLUE:                     { return "F1-Blue";                     } break;
			case CEC::CEC_USER_CONTROL_CODE_F2_RED:                      { return "F2-Red";                      } break;
			case CEC::CEC_USER_CONTROL_CODE_F3_GREEN:                    { return "F3-Green";                    } break;
			case CEC::CEC_USER_CONTROL_CODE_F4_YELLOW:                   { return "F4-Yellow";                   } break;
			case CEC::CEC_USER_CONTROL_CODE_F5:                          { return "F5";                          } break;
			case CEC::CEC_USER_CONTROL_CODE_DATA:                        { return "Data";                        } break;
			case CEC::CEC_USER_CONTROL_CODE_AN_RETURN:                   { return "AN-Return";                   } break;
			case CEC::CEC_USER_CONTROL_CODE_AN_CHANNELS_LIST:            { return "AN-Channels-List";            } break;
			default:                                                     { return "UNKNOWN";                     }
		}
#else // HAVE_LIBCEC
		(void)keycode;
#endif // HAVE_LIBCEC

		return "UNKNOWN";
	}

	std::string string()
	{
		std::stringstream stream;
		switch(type)
		{
			case TYPE_BUTTON:
				stream << "Button " << id;
				break;
			case TYPE_AXIS:
				stream << "Axis " << id << (value > 0 ? "+" : "-");
				break;
			case TYPE_HAT:
				stream << "Hat " << id << " " << getHatDir(value);
				break;
			case TYPE_KEY:
				stream << "Key " << SDL_GetKeyName((SDL_Keycode)id);
				break;
			case TYPE_CEC_BUTTON:
				stream << "CEC-Button " << getCECButtonName(id);
				break;
			default:
				stream << "Input to string error";
				break;
		}

		return stream.str();
	}
};

class InputConfig
{
public:
	InputConfig(int deviceId, const std::string& deviceName, const std::string& deviceGUID);

	void clear();
	void mapInput(const std::string& name, Input input);
	void unmapInput(const std::string& name); // unmap all Inputs mapped to this name

	inline int getDeviceId() const { return mDeviceId; };
	inline const std::string& getDeviceName() { return mDeviceName; }
	inline const std::string& getDeviceGUIDString() { return mDeviceGUID; }

	//Returns true if Input is mapped to this name, false otherwise.
	bool isMappedTo(const std::string& name, Input input);

	//Returns a list of names this input is mapped to.
	std::vector<std::string> getMappedTo(Input input);

	// Returns true if there is an Input mapped to this name, false otherwise.
	// Writes Input mapped to this name to result if true.
	bool getInputByName(const std::string& name, Input* result);

	void loadFromXML(pugi::xml_node& root);
	void writeToXML(pugi::xml_node& parent);

	bool isConfigured();

private:
	std::map<std::string, Input> mNameMap;
	const int mDeviceId;
	const std::string mDeviceName;
	const std::string mDeviceGUID;
};

#endif // ES_CORE_INPUT_CONFIG_H
