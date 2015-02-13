#pragma once

#include <vector>
#include <string>
#include "FileData.h"
#include "Window.h"
#include "MetaData.h"
#include "PlatformId.h"
#include "ThemeData.h"
#include "SystemOptionValue.h"

class SystemOption
{
public:
    SystemOption(const std::string& id, const std::string& replace, const std::string& desc, const std::string& defaultValue);
    ~SystemOption();

    inline const std::string& getID() const { return mID; }
    inline const std::string& getMetaDataID() const { return mMetaDataID; }
    inline const std::string& getReplace() const { return mReplace; }
    inline const std::string& getDesc() const { return mDesc; }
    inline const std::string& getDefaultID() const { return mDefaultID; }
    inline SystemOptionValue* getDefault() const { return mDefault; }

    void addValue( SystemOptionValue* value );

    bool hasValues() const { return mValues.size() > 0; }
    bool hasDefault() const { return mDefaultID.size() > 0; }
    const std::vector<SystemOptionValue*>& getValues() const { return mValues; }
    SystemOptionValue* getValue( std::string id );

//    inline const std::vector<std::string>& getExtensions() const { return mSearchExtensions; }

private:
    std::string mID;
    std::string mMetaDataID;
    std::string mReplace;
    std::string mDesc;
    std::string mDefaultID;
    std::vector<SystemOptionValue*> mValues;
    SystemOptionValue* mDefault;
};
