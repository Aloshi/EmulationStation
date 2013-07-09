#include "XMLReader.h"
#include "SystemData.h"
#include "GameData.h"
#include "pugiXML/pugixml.hpp"
#include <boost/filesystem.hpp>
#include "Log.h"

//this is obviously an incredibly inefficient way to go about searching
//but I don't think it'll matter too much with the size of most collections
GameData* searchFolderByPath(FolderData* folder, std::string const& path)
{
	for(unsigned int i = 0; i < folder->getFileCount(); i++)
	{
		FileData* file = folder->getFile(i);

		if(file->isFolder())
		{
			GameData* result = searchFolderByPath((FolderData*)file, path);
			if(result)
				return (GameData*)result;
		}else{
			if(file->getPath() == path)
				return (GameData*)file;
		}
	}

	return NULL;
}

GameData* createGameFromPath(std::string gameAbsPath, SystemData* system)
{
	std::string gamePath = gameAbsPath;
	std::string sysPath = system->getStartPath();

	//strip out the system path stuff so it's relative to the system root folder
	unsigned int i = 0;
	while(i < gamePath.length() && i < sysPath.length() && gamePath[i] == sysPath[i])
		i++;

	gamePath = gamePath.substr(i, gamePath.length() - i);


	if(gamePath[0] != '/')
		gamePath.insert(0, "/");


	//make our way through the directory tree finding each folder in our path or creating it if it doesn't exist
	FolderData* folder = system->getRootFolder();

	size_t separator = 0;
	size_t nextSeparator = 0;
	unsigned int loops = 0;
	while(nextSeparator != std::string::npos)
	{
		//determine which chunk of the path we're testing right now
		nextSeparator = gamePath.find('/', separator + 1);
		if(nextSeparator == std::string::npos)
			break;

		std::string checkName = gamePath.substr(separator + 1, nextSeparator - separator - 1);
		separator = nextSeparator;

		//see if the folder already exists
		bool foundFolder = false;
		for(unsigned int i = 0; i < folder->getFileCount(); i++)
		{
			FileData* checkFolder = folder->getFile(i);
			if(checkFolder->isFolder() && checkFolder->getName() == checkName)
			{
				folder = (FolderData*)checkFolder;
				foundFolder = true;
				break;
			}
		}

		//the folder didn't already exist, so create it
		if(!foundFolder)
		{
			FolderData* newFolder = new FolderData(system, folder->getPath() + "/" + checkName, checkName);
			folder->pushFileData(newFolder);
			folder = newFolder;
		}

		//if for some reason this function is broken, break out of this while instead of freezing
		if(loops > gamePath.length() * 2)
		{
			LOG(LogError) << "createGameFromPath breaking out of loop for path \"" << gamePath << "\" to prevent infinite loop (please report this)";
			break;
		}
		loops++;
	}


	//find gameName
	std::string gameName = gamePath.substr(separator + 1, gamePath.find(".", separator) - separator - 1);

	GameData* game = new GameData(system, gameAbsPath, gameName);
	folder->pushFileData(game);
	return game;
}

void parseGamelist(SystemData* system)
{
	std::string xmlpath = system->getGamelistPath();

	if(xmlpath.empty())
		return;

	LOG(LogInfo) << "Parsing XML file \"" << xmlpath << "\"...";

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(xmlpath.c_str());

	if(!result)
	{
		LOG(LogError) << "Error parsing XML file \"" << xmlpath << "\"!\n	" << result.description();
		return;
	}

	pugi::xml_node root = doc.child(GameData::xmlTagGameList.c_str());
	if(!root)
	{
		LOG(LogError) << "Could not find <" << GameData::xmlTagGameList << "> node in gamelist \"" << xmlpath << "\"!";
		return;
	}

	for(pugi::xml_node gameNode = root.child(GameData::xmlTagGame.c_str()); gameNode; gameNode = gameNode.next_sibling(GameData::xmlTagGame.c_str()))
	{
		pugi::xml_node pathNode = gameNode.child(GameData::xmlTagPath.c_str());
		if(!pathNode)
		{
			LOG(LogError) << "<" << GameData::xmlTagGame << "> node contains no <" << GameData::xmlTagPath << "> child!";
			continue;
		}

		//convert path to generic directory seperators
		boost::filesystem::path gamePath(pathNode.text().get());
		std::string path = gamePath.generic_string();

		//expand "."
		if(path[0] == '.')
		{
			path.erase(0, 1);
			path.insert(0, system->getRootFolder()->getPath());
		}

		if(boost::filesystem::exists(path))
		{
			GameData* game = searchFolderByPath(system->getRootFolder(), path);

			if(game == NULL)
				game = createGameFromPath(path, system);

			//actually gather the information in the XML doc, then pass it to the game's set method
			std::string newName, newDesc, newImage;

			if(gameNode.child(GameData::xmlTagName.c_str()))
			{
				game->setName(gameNode.child(GameData::xmlTagName.c_str()).text().get());
			}
			if(gameNode.child(GameData::xmlTagDescription.c_str())) 
			{
				game->setDescription(gameNode.child(GameData::xmlTagDescription.c_str()).text().get());
			}
			if(gameNode.child(GameData::xmlTagImagePath.c_str()))
			{
				newImage = gameNode.child(GameData::xmlTagImagePath.c_str()).text().get();

				//expand "."
				if(newImage[0] == '.')
				{
					newImage.erase(0, 1);
					boost::filesystem::path pathname(xmlpath);
					newImage.insert(0, pathname.parent_path().generic_string() );
				}

				//if the image exist, set it
				if(boost::filesystem::exists(newImage))
				{
					game->setImagePath(newImage);
				}
			}

			//get rating and the times played from the XML doc
			if(gameNode.child(GameData::xmlTagRating.c_str()))
			{
				float rating;
				std::istringstream(gameNode.child(GameData::xmlTagRating.c_str()).text().get()) >> rating;
				game->setRating(rating);
			}
			if(gameNode.child(GameData::xmlTagUserRating.c_str()))
			{
				float userRating;
				std::istringstream(gameNode.child(GameData::xmlTagUserRating.c_str()).text().get()) >> userRating;
				game->setUserRating(userRating);
			}
			if(gameNode.child(GameData::xmlTagTimesPlayed.c_str()))
			{
				size_t timesPlayed;
				std::istringstream(gameNode.child(GameData::xmlTagTimesPlayed.c_str()).text().get()) >> timesPlayed;
				game->setTimesPlayed(timesPlayed);
			}
			if(gameNode.child(GameData::xmlTagLastPlayed.c_str()))
			{
				std::time_t lastPlayed;
				std::istringstream(gameNode.child(GameData::xmlTagLastPlayed.c_str()).text().get()) >> lastPlayed;
				game->setLastPlayed(lastPlayed);
			}
		}
		else{
			LOG(LogWarning) << "Game at \"" << path << "\" does not exist!";
		}
	}
}

void addGameDataNode(pugi::xml_node & parent, const GameData * game)
{
	//create game and add to parent node
	pugi::xml_node newGame = parent.append_child(GameData::xmlTagGame.c_str());
	//add values
	if (!game->getPath().empty()) {
		pugi::xml_node pathNode = newGame.append_child(GameData::xmlTagPath.c_str());
		//store path with generic directory seperators
		boost::filesystem::path gamePath(game->getPath());
		pathNode.text().set(gamePath.generic_string().c_str());
	}
	if (!game->getName().empty()) {
		pugi::xml_node nameNode = newGame.append_child(GameData::xmlTagName.c_str());
		nameNode.text().set(game->getName().c_str());
	}
	if (!game->getDescription().empty()) {
		pugi::xml_node descriptionNode = newGame.append_child(GameData::xmlTagDescription.c_str());
		descriptionNode.text().set(game->getDescription().c_str());
	}
	if (!game->getImagePath().empty()) {
		pugi::xml_node imagePathNode = newGame.append_child(GameData::xmlTagImagePath.c_str());
		imagePathNode.text().set(game->getImagePath().c_str());
	}
	//all other values are added regardless of their value
	pugi::xml_node ratingNode = newGame.append_child(GameData::xmlTagRating.c_str());
	ratingNode.text().set(std::to_string((long double)game->getRating()).c_str());

	pugi::xml_node userRatingNode = newGame.append_child(GameData::xmlTagUserRating.c_str());
	userRatingNode.text().set(std::to_string((long double)game->getUserRating()).c_str());

	pugi::xml_node timesPlayedNode = newGame.append_child(GameData::xmlTagTimesPlayed.c_str());
	timesPlayedNode.text().set(std::to_string((unsigned long long)game->getTimesPlayed()).c_str());

	pugi::xml_node lastPlayedNode = newGame.append_child(GameData::xmlTagLastPlayed.c_str());
	lastPlayedNode.text().set(std::to_string((unsigned long long)game->getLastPlayed()).c_str());
}

void updateGamelist(SystemData* system)
{
	//We do this by reading the XML again, adding changes and then writing it back,
	//because there might be information missing in our systemdata which would then miss in the new XML.
	//We have the complete information for every game though, so we can simply remove a game
	//we already have in the system from the XML, and then add it back from its GameData information...

	std::string xmlpath = system->getGamelistPath();
	if(xmlpath.empty()) {
		return;
	}

	LOG(LogInfo) << "Parsing XML file \"" << xmlpath << "\" before writing...";

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(xmlpath.c_str());

	if(!result) {
		LOG(LogError) << "Error parsing XML file \"" << xmlpath << "\"!\n	" << result.description();
		return;
	}

	pugi::xml_node root = doc.child(GameData::xmlTagGameList.c_str());
	if(!root) {
		LOG(LogError) << "Could not find <" << GameData::xmlTagGameList << "> node in gamelist \"" << xmlpath << "\"!";
		return;
	}

	//now we have all the information from the XML. now iterate through all our games and add information from there
	FolderData * rootFolder = system->getRootFolder();
	if (rootFolder != nullptr) {
		//get only files, no folders
		std::vector<FileData*> files = rootFolder->getFilesRecursive(true);
		//iterate through all files, checking if they're already in the XML
		std::vector<FileData*>::const_iterator fit = files.cbegin();
		while(fit != files.cend()) {
			//try to cast to gamedata
			const GameData * game = dynamic_cast<const GameData*>(*fit);
			if (game != nullptr) {
				//worked. check if this games' path can be found somewhere in the XML
				for(pugi::xml_node gameNode = root.child(GameData::xmlTagGame.c_str()); gameNode; gameNode = gameNode.next_sibling(GameData::xmlTagGame.c_str())) {
					//get path from game node
					pugi::xml_node pathNode = gameNode.child(GameData::xmlTagPath.c_str());
					if(!pathNode)
					{
						LOG(LogError) << "<" << GameData::xmlTagGame << "> node contains no <" << GameData::xmlTagPath << "> child!";
						continue;
					}
					//check paths. use the same directory separators
					boost::filesystem::path nodePath(pathNode.text().get());
					boost::filesystem::path gamePath(game->getPath());
					if (nodePath.generic_string() == gamePath.generic_string()) {
						//found the game. remove it. it will be added again later with updated values
						root.remove_child(gameNode);
						//break node search loop
						break;
					}
				}
				//either the game content was removed, because it needs to be updated,
				//or didn't exist in the first place, so just add it
				addGameDataNode(root, game);
			}
			++fit;
		}
		//now write the file
		if (!doc.save_file(xmlpath.c_str())) {
			LOG(LogError) << "Error saving XML file \"" << xmlpath << "\"!";
		}
	}
	else {
		LOG(LogError) << "Found no root folder for system \"" << system->getName() << "\"!";
	}
}
