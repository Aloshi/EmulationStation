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
    SystemOptionValue(const std::string& id, const std::string& desc, const std::string& code) : 
        mID(id), mDesc(desc), mCode(code) { }

    ~SystemOptionValue();

    inline const std::string& getID() const { return mID; }
    inline const std::string& getDesc() const { return mDesc; }
    inline const std::string& getCode() const { return mCode; }

private:
    std::string mID;
    std::string mDesc;
    std::string mCode;
};

class SystemOption
{
public:
    SystemOption(const std::string& id, const std::string& replace, const std::string& desc, const std::string& defaultID) :
        mID(id), mReplace(replace), mDesc(desc), mDefaultID(defaultID) { }

    ~SystemOption();

    inline const std::string& getID() const { return mID; }
    inline const std::string& getReplace() const { return mReplace; }
    inline const std::string& getDesc() const { return mDesc; }
    inline const std::string& getDefaultID() const { return mDefaultID; }
    inline SystemOptionValue* getDefault() const { return mDefault; }

    void addValue( SystemOptionValue* value );

    bool hasValues() const { return mValues.size() > 0; }
    bool hasDefault() const { return mDefaultID.size() > 0; }
    const std::vector<SystemOptionValue*>& getValues() const { return mValues; }
    SystemOptionValue* getValue( std::string id );

    static std::map<std::string, std::string> getInheritedOptions( std::vector<FileData> files, FileOptionType type );

private:
    std::string mID;
    std::string mMetaDataID;
    std::string mReplace;
    std::string mDesc;
    std::string mDefaultID;
    std::vector<SystemOptionValue*> mValues;
    SystemOptionValue* mDefault;
};

