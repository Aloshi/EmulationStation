#pragma once

#include <vector>
#include <string>
#include "FileData.h"
#include "Window.h"
#include "MetaData.h"
#include "PlatformId.h"
#include "ThemeData.h"

class SystemOptionValue
{
public:
    SystemOptionValue(const std::string& id, const std::string& desc, const std::string& code);
    ~SystemOptionValue();

    inline const std::string& getID() const { return mID; }
    inline const std::string& getDesc() const { return mDesc; }
    inline const std::string& getCode() const { return mCode; }

private:
    std::string mID;
    std::string mDesc;
    std::string mCode;
};
