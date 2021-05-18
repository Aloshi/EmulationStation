#pragma once
#ifndef ES_CORE_SCRIPTING_H
#define ES_CORE_SCRIPTING_H

#include <string>

namespace Scripting
{
	int fireEvent(const std::string& eventName, const std::string& arg1="", const std::string& arg2="", const std::string& arg3="", const std::string& arg4="");
} // Scripting::

#endif //ES_CORE_SCRIPTING_H
