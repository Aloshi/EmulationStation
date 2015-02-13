#include "SystemData.h"
#include "Gamelist.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <stdlib.h>

SystemOption::SystemOption(const std::string& id, const std::string& replace, const std::string& desc, const std::string& defaultID)
{
    mID = id;
    mMetaDataID = "option_" + id;
    mReplace = replace;
    mDesc = desc;
    mDefaultID = defaultID;
    mDefault = NULL;
}

void SystemOption::addValue( SystemOptionValue* value )
{
    mValues.push_back( value );

    if( value->getID().compare( mDefaultID ) == 0 )
        mDefault = value;
}

SystemOptionValue* SystemOption::getValue( std::string id )
{
    for( auto value = mValues.begin(); value != mValues.end(); value++ )
        if( id.compare( (*value)->getID() ) == 0 )
            return (*value);

    return NULL;
}

SystemOption::~SystemOption()
{
}
