#include "InputConfig.h"
#include <string>
#include <algorithm>
#include <SDL.h>
#include <iostream>

std::string toLower(std::string str)
{
	for(unsigned int i = 0; i < str.length(); i++)
	{
		str[i] = tolower(str[i]);
	}

	return str;
}

InputConfig::InputConfig(int deviceId) : mDeviceId(deviceId)
{
	mPlayerNum = -1;
}

void InputConfig::clear()
{
	mNameMap.clear();
}

void InputConfig::mapInput(const std::string& name, Input input)
{
	mNameMap[toLower(name)] = input;
}

Input InputConfig::getInputByName(const std::string& name)
{
	return mNameMap[toLower(name)];
}

bool InputConfig::isMappedTo(const std::string& name, Input input)
{
	Input comp = getInputByName(name);

	if(comp.configured && comp.type == input.type && comp.id == input.id)
	{
		if(comp.type == TYPE_HAT)
		{
			return (input.value == 0 || input.value & comp.value);
		}

		if(comp.type == TYPE_AXIS)
		{
			return input.value == 0 || comp.value == input.value;
		}else{
			return true;
		}
	}
	return false;
}

std::vector<std::string> InputConfig::getMappedTo(Input input)
{
	std::vector<std::string> maps;

	typedef std::map<std::string, Input>::iterator it_type;
	for(it_type iterator = mNameMap.begin(); iterator != mNameMap.end(); iterator++)
	{
		Input chk = iterator->second;

		if(!chk.configured)
			continue;

		if(chk.device == input.device && chk.type == input.type && chk.id == input.id)
		{
			if(chk.type == TYPE_HAT)
			{
				if(input.value == 0 || input.value & chk.value)
				{
					maps.push_back(iterator->first);
				}
				continue;
			}

			if(input.type == TYPE_AXIS)
			{
				if(input.value == 0 || chk.value == input.value)
					maps.push_back(iterator->first);
			}else{
				maps.push_back(iterator->first);
			}
		}
	}

	return maps;
}


void InputConfig::setPlayerNum(int num) { mPlayerNum = num; }
int InputConfig::getPlayerNum() { return mPlayerNum; }
int InputConfig::getDeviceId() { return mDeviceId; }
