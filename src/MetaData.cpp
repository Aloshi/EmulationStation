#include "MetaData.h"
#include "components/TextComponent.h"
#include "Log.h"

#include "components/TextEditComponent.h"
#include "components/RatingComponent.h"
#include "components/DateTimeComponent.h"

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
		{"name",		MD_STRING,				"", 	false}, 
		{"desc",		MD_MULTILINE_STRING,	"", 	false},
		{"image",		MD_IMAGE_PATH,			"", 	false},
		{"thumbnail",	MD_IMAGE_PATH,			"", 	false},
		{"rating",		MD_RATING,				"0.000000", 	false},
		{"releasedate", MD_DATE,				"0", 	false},
		{"playcount",	MD_INT,					"0", 	true},
		{"lastplayed",	MD_TIME,				"0", 	true},
		{"selected",	MD_SELECTED,				"", 	true}
	};

	std::vector<MetaDataDecl> mdd(decls, decls + sizeof(decls) / sizeof(decls[0]));
	return mdd;
}

MetaDataList MetaDataList::createFromXML(const std::vector<MetaDataDecl>& mdd, pugi::xml_node node)
{
	MetaDataList mdl;

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

void MetaDataList::appendToXML(pugi::xml_node parent, const std::vector<MetaDataDecl>& ignoreDefaults) const
{
	for(auto iter = mMap.begin(); iter != mMap.end(); iter++)
	{
		bool write = true;
		for(auto mddIter = ignoreDefaults.begin(); mddIter != ignoreDefaults.end(); mddIter++)
		{
			if(mddIter->key == iter->first)
			{
				if(iter->second == mddIter->defaultValue)
					write = false;

				break;
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
