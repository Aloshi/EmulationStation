#pragma once
#ifndef ES_CORE_MAMENAMES_H
#define ES_CORE_MAMENAMES_H

#include <string>
#include <vector>

class MameNames
{
public:

	static void       init       ();
	static void       deinit     ();
	static MameNames* getInstance();
	std::string       getRealName(const std::string& _mameName);

private:

	struct NamePair
	{
		std::string mameName;
		std::string realName;
	};

	typedef std::vector<NamePair> namePairVector;

	 MameNames();
	~MameNames();

	static MameNames* sInstance;

	namePairVector    mNamePairs;

}; // MameNames

#endif // ES_CORE_MAMENAMES_H
