#include "SystemData.h"
#include "Gamelist.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <stdlib.h>

SystemOptionValue::SystemOptionValue(const std::string& id, const std::string& desc, const std::string& code)
{
    mID = id;
    mDesc = desc;
    mCode = code;
}

SystemOptionValue::~SystemOptionValue()
{
}
