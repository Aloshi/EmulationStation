#include "MetaData.h"
#include "components/TextComponent.h"
#include "Log.h"
#include "components/TextEditComponent.h"

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
		{"name",		MD_STRING,				""}, 
		{"desc",		MD_MULTILINE_STRING,	""},
		{"image",		MD_IMAGE_PATH,			""},
		{"rating",		MD_RATING,				"0"},
		{"userrating",	MD_RATING,				"0"},
		{"playcount",	MD_INT,					"0"},
		{"lastplayed",	MD_TIME,				"0"}
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

std::time_t MetaDataList::getTime(const std::string& key) const
{
	return (std::time_t) atoi(get(key).c_str());
}

GuiComponent* MetaDataList::makeDisplay(Window* window, MetaDataType as)
{
	switch(as)
	{
	default:
		TextComponent* comp = new TextComponent(window);
		return comp;
	}
}

GuiComponent* MetaDataList::makeEditor(Window* window, MetaDataType as)
{
	switch(as)
	{
	default:
		TextEditComponent* comp = new TextEditComponent(window);
		return comp;
	}
}
