#include "MetaData.h"
#include "components/TextComponent.h"
#include "Log.h"

#include "components/TextEditComponent.h"
#include "components/RatingComponent.h"
#include "components/DateTimeComponent.h"
#include <sstream>
#include <cstring>

MetaDataList::MetaDataList()
{
}

MetaDataList::MetaDataList(const std::vector<MetaDataDecl>& mdd)
{
	for(auto iter = mdd.begin(); iter != mdd.end(); iter++)
		set(iter->key, iter->defaultValue);
}

std::vector<MetaDataDecl> MetaDataList::getDefaultGameMDD()
{
	MetaDataDecl decls[] = { 
		{"name",		MD_STRING,				"", 		false,	false}, 
		{"desc",		MD_MULTILINE_STRING,	"", 		false,	false},
		{"image",		MD_IMAGE_PATH_LIST,			"", 		false,	false},
		{"thumbnail",	MD_IMAGE_PATH,			"", 		false,	false},
		{"rating",		MD_RATING,				"0.000000",	false,	false},
		{"releasedate", MD_DATE,				"0", 		false,	false},
		{"playcount",	MD_INT,					"0", 		true,	false},
		{"lastplayed",	MD_TIME,				"0", 		true,	false},
		{"selected",	MD_SELECTED,			"", 		true,	true}
	};

	std::vector<MetaDataDecl> mdd(decls, decls + sizeof(decls) / sizeof(decls[0]));
	return mdd;
}

unsigned int MetaDataList::getSize(const std::string &key)
{
    std::ostringstream ostr;
    ostr << key << "#";
    const std::string keyWithSep(ostr.str());
    unsigned int size = std::count_if(
            mMap.begin(),
            mMap.end(),
            [&keyWithSep](const std::pair<std::string, std::string> &key_value){
                return keyWithSep.size() < key_value.first.size() && std::strncmp(keyWithSep.c_str(), key_value.first.c_str(), keyWithSep.size()) == 0;
            }
        );
    return size;
}

const std::string &MetaDataList::getElemAt(const std::string &key, unsigned int npos)
{
    std::ostringstream ostr;
    ostr << key << "#" << npos;
    const std::string &result = mMap.at(ostr.str());
    return result;
}

void MetaDataList::clearList(const std::string &key)
{
    const std::string keyWithSep(key+"#");
    for (auto iter = mMap.begin(); iter != mMap.end();)
    {
        if (keyWithSep.compare(0, keyWithSep.size(), iter->first) == 0)
            mMap.erase(iter++);
        else
            ++iter;
    }
}

void MetaDataList::push_back(const std::string &key, const std::string &value)
{
    std::ostringstream ostr;
    ostr << key << "#" << getSize(key);
    mMap[ostr.str()] = value;
}

void MetaDataList::set(const std::string &key, unsigned int npos, const std::string &value)
{
    std::ostringstream ostr;
    ostr << key << "#" << npos;
    mMap[ostr.str()] = value;
}

MetaDataList MetaDataList::createFromXML(const std::vector<MetaDataDecl>& mdd, pugi::xml_node node)
{
	MetaDataList mdl;

        // set up defaults
	for(auto iter = mdd.begin(); iter != mdd.end(); iter++)
        {
                if (iter->type != MD_IMAGE_PATH_LIST)
                        mdl.set(iter->key, iter->defaultValue);
        }
        // overwrite settings from xml
        for (pugi::xml_node::iterator iter = node.begin(); iter != node.end(); ++iter)
        {
                if (std::string(iter->name()) == "image") 
                        mdl.push_back(iter->name(), iter->text().get());
                else
                        mdl.set(iter->name(), iter->text().get());
        }

	return mdl;
}

void MetaDataList::appendToXML(pugi::xml_node parent, const std::vector<MetaDataDecl>& ignoreDefaults) const
{
	for(auto iter = mMap.begin(); iter != mMap.end(); iter++)
	{
                std::string tagName;
		for(auto mddIter = ignoreDefaults.begin(); mddIter != ignoreDefaults.end(); mddIter++)
		{
			if(mddIter->key == iter->first)
			{
                                if(iter->second != mddIter->defaultValue)
                                        tagName = iter->first;
				break;
			} else if (mddIter->type == MD_IMAGE_PATH_LIST 
                                        && mddIter->key.size() < iter->first.size()
                                        && std::strncmp(mddIter->key.c_str(), iter->first.c_str(), mddIter->key.size()) == 0)
                        {
                                tagName = iter->first.substr(0, iter->first.find('#'));
                                break;
                        }
		}

		if(!tagName.empty())
			parent.append_child(tagName.c_str()).text().set(iter->second.c_str());
	}
}

void MetaDataList::set(const std::string& key, const std::string& value)
{
	mMap[key] = value;
}

void MetaDataList::setTime(const std::string& key, const boost::posix_time::ptime& time)
{
	mMap[key] = boost::posix_time::to_iso_string(time);
}

const std::string& MetaDataList::get(const std::string& key) const
{
	return mMap.at(key);
}

int MetaDataList::getInt(const std::string& key) const
{
	return atoi(get(key).c_str());
}

float MetaDataList::getFloat(const std::string& key) const
{
	return (float)atof(get(key).c_str());
}

boost::posix_time::ptime MetaDataList::getTime(const std::string& key) const
{
	return string_to_ptime(get(key), "%Y%m%dT%H%M%S%F%q");
}

GuiComponent* MetaDataList::makeDisplay(Window* window, MetaDataType as)
{
	switch(as)
	{
	case MD_RATING:
		{
			RatingComponent* comp = new RatingComponent(window);
			return comp;
		}
	default:
		TextComponent* comp = new TextComponent(window);
		return comp;
	}
}

GuiComponent* MetaDataList::makeEditor(Window* window, MetaDataType as)
{
	switch(as)
	{
	case MD_RATING:
		{
			RatingComponent* comp = new RatingComponent(window);
			return comp;
		}
	case MD_MULTILINE_STRING:
		{
			TextEditComponent* comp = new TextEditComponent(window);
			comp->setSize(comp->getSize().x(), comp->getSize().y() * 3);
			return comp;
		}
	case MD_DATE:
		{
			DateTimeComponent* comp = new DateTimeComponent(window);
			return comp;
		}
	case MD_TIME:
		{
			DateTimeComponent* comp = new DateTimeComponent(window);
			comp->setDisplayMode(DateTimeComponent::DISP_RELATIVE_TO_NOW);
			return comp;
		}
	case MD_SELECTED:
		{
			DateTimeComponent* comp = new DateTimeComponent(window);
			comp->setDisplayMode(DateTimeComponent::DISP_RELATIVE_TO_NOW);
			return comp;
		}
	default:
		{
			TextEditComponent* comp = new TextEditComponent(window);
			return comp;
		}
	}
}

//util function
boost::posix_time::ptime string_to_ptime(const std::string& str, const std::string& fmt)
{
	std::istringstream ss(str);
	ss.imbue(std::locale(std::locale::classic(), new boost::posix_time::time_input_facet(fmt))); //std::locale handles deleting the facet
	boost::posix_time::ptime time;
	ss >> time;

	return time;
}
