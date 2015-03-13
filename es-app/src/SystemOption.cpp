#include "SystemData.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <stdlib.h>

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

std::map<std::string, std::string> SystemOption::getInheritedOptions( std::vector<FileData> files, FileOptionType type )
{
    std::map<std::string, std::string> values;
    
    for( auto it = files.begin(); it != files.end(); it++ )
    {
        std::map<std::string, std::string> options = (*it).get_options( type );

        for( auto option = options.begin(); option != options.end(); option++ )
        {
            std::string id    = option->first;
            std::string value = option->second;

            if( values.count(id) == 0 )
                values[id] = value;
        }
    }

    return values;
}
