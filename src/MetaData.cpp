#include "MetaData.h"
#include "components/TextComponent.h"
#include "Log.h"

#include "components/TextEditComponent.h"
#include "components/RatingComponent.h"
#include "components/DateTimeComponent.h"


MetaDataDecl gameDecls[] = { 
	{"name",		MD_STRING,				"", 		false}, 
	{"desc",		MD_MULTILINE_STRING,	"", 		false},
	{"image",		MD_IMAGE_PATH,			"", 		false},
	{"thumbnail",	MD_IMAGE_PATH,			"", 		false},
	{"rating",		MD_RATING,				"0", 		false},
	{"releasedate", MD_DATE,				"0", 		false},
	{"developer",	MD_STRING,				"unknown",	false},
	{"publisher",	MD_STRING,				"unknown",	false},
	{"genre",		MD_STRING,				"unknown",	false},
	{"players",		MD_INT,					"1",		false},
	{"playcount",	MD_INT,					"0",		true},
	{"lastplayed",	MD_TIME,				"0", 		true}
};
const std::vector<MetaDataDecl> gameMDD(gameDecls, gameDecls + sizeof(gameDecls) / sizeof(gameDecls[0]));

MetaDataDecl folderDecls[] = { 
	{"name",		MD_STRING,				"", 	false}, 
	{"desc",		MD_MULTILINE_STRING,	"", 	false},
	{"image",		MD_IMAGE_PATH,			"", 	false},
	{"thumbnail",	MD_IMAGE_PATH,			"", 	false},
};
const std::vector<MetaDataDecl> folderMDD(folderDecls, folderDecls + sizeof(folderDecls) / sizeof(folderDecls[0]));

const std::vector<MetaDataDecl>& getMDDByType(MetaDataListType type)
{
	switch(type)
	{
	case GAME_METADATA:
		return gameMDD;
	case FOLDER_METADATA:
		return folderMDD;
	}

	LOG(LogError) << "Invalid MDD type";
	return gameMDD;
}



MetaDataList::MetaDataList(MetaDataListType type)
	: mType(type)
{
	const std::vector<MetaDataDecl>& mdd = getMDD();
	for(auto iter = mdd.begin(); iter != mdd.end(); iter++)
		set(iter->key, iter->defaultValue);
}


MetaDataList MetaDataList::createFromXML(MetaDataListType type, pugi::xml_node node)
{
	MetaDataList mdl(type);

	const std::vector<MetaDataDecl>& mdd = mdl.getMDD();

	for(auto iter = mdd.begin(); iter != mdd.end(); iter++)
	{
		pugi::xml_node md = node.child(iter->key.c_str());
		if(md)
		{
			mdl.set(iter->key, md.text().get());
		}else{
			mdl.set(iter->key, iter->defaultValue);
		}
	}

	return mdl;
}

void MetaDataList::appendToXML(pugi::xml_node parent, bool ignoreDefaults) const
{
	const std::vector<MetaDataDecl>& mdd = getMDD();

	for(auto iter = mMap.begin(); iter != mMap.end(); iter++)
	{
		bool write = true;

		if(ignoreDefaults)
		{
			for(auto mddIter = mdd.begin(); mddIter != mdd.end(); mddIter++)
			{
				if(mddIter->key == iter->first)
				{
					if(iter->second == mddIter->defaultValue)
						write = false;

					break;
				}
			}
		}

		if(write)
			parent.append_child(iter->first.c_str()).text().set(iter->second.c_str());
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

std::shared_ptr<GuiComponent> MetaDataList::makeEditor(Window* window, MetaDataType as)
{
	switch(as)
	{
	case MD_RATING:
		{
			return std::make_shared<RatingComponent>(window);
		}
	case MD_MULTILINE_STRING:
		{
			auto comp = std::make_shared<TextEditComponent>(window);
			comp->setSize(comp->getSize().x(), comp->getSize().y() * 3);
			return comp;
		}
	case MD_DATE:
		{
			return std::make_shared<DateTimeComponent>(window);
		}
	case MD_TIME:
		{
			auto comp = std::make_shared<DateTimeComponent>(window);
			comp->setDisplayMode(DateTimeComponent::DISP_RELATIVE_TO_NOW);
			return comp;
		}
	default:
		{
			return std::make_shared<TextEditComponent>(window);
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
