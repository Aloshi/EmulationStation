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

	GameData* game = new GameData(system, gameAbsPath);
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

	pugi::xml_node root = doc.child("gameList");
	if(!root)
	{
		LOG(LogError) << "Could not find <gameList> node in gamelist \"" << xmlpath << "\"!";
		return;
	}

	for(pugi::xml_node gameNode = root.child("game"); gameNode; gameNode = gameNode.next_sibling("game"))
	{
		pugi::xml_node pathNode = gameNode.child("path");
		if(!pathNode)
		{
			LOG(LogError) << "<game> node contains no <path> child!";
			continue;
		}

		//convert path to generic directory seperators
		boost::filesystem::path gamePath(pathNode.text().get());
		std::string path = gamePath.generic_string();

		//expand '.'
		if(path[0] == '.')
		{
			path.erase(0, 1);
			path.insert(0, boost::filesystem::path(xmlpath).parent_path().generic_string());
		}

		//expand '~'
		if(path[0] == '~')
		{
			path.erase(0, 1);
			path.insert(0, getHomePath());
		}

		if(boost::filesystem::exists(path))
		{
			GameData* game = searchFolderByPath(system->getRootFolder(), path);

			if(game == NULL)
				game = createGameFromPath(path, system);

			//load the metadata
			*(game->metadata()) = MetaDataList::createFromXML(MetaDataList::getDefaultGameMDD(), gameNode);

			//make sure name gets set if one didn't exist
			if(game->metadata()->get("name").empty())
				game->metadata()->set("name", game->getBaseName());
		}else{
			LOG(LogWarning) << "Game at \"" << path << "\" does not exist!";
		}
	}
}

void addGameDataNode(pugi::xml_node& parent, const GameData* game)
{
	//create game and add to parent node
	pugi::xml_node newGame = parent.append_child("game");

	//write metadata
	const_cast<GameData*>(game)->metadata()->appendToXML(newGame, MetaDataList::getDefaultGameMDD());
	
	if(newGame.children().begin() == newGame.child("name") //first element is name
		&& ++newGame.children().begin() == newGame.children().end() //theres only one element
		&& newGame.child("name").text().get() == game->getBaseName()) //the name is the default
	{
		//if the only info is the default name, don't bother with this node
		parent.remove_child(newGame);
	}else{
		//there's something useful in there so we'll keep the node, add the path
		newGame.prepend_child("path").text().set(game->getPath().c_str());
	}
}

void updateGamelist(SystemData* system)
{
	//We do this by reading the XML again, adding changes and then writing it back,
	//because there might be information missing in our systemdata which would then miss in the new XML.
	//We have the complete information for every game though, so we can simply remove a game
	//we already have in the system from the XML, and then add it back from its GameData information...

	std::string xmlpath = system->getGamelistPath();
	if(xmlpath.empty())
		return;

	LOG(LogInfo) << "Parsing XML file \"" << xmlpath << "\" before writing...";

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(xmlpath.c_str());

	if(!result)
	{
		LOG(LogError) << "Error parsing XML file \"" << xmlpath << "\"!\n	" << result.description();
		return;
	}

	pugi::xml_node root = doc.child("gameList");
	if(!root)
	{
		LOG(LogError) << "Could not find <gameList> node in gamelist \"" << xmlpath << "\"!";
		return;
	}

	//now we have all the information from the XML. now iterate through all our games and add information from there
	FolderData * rootFolder = system->getRootFolder();
	if (rootFolder != nullptr)
	{
		//get only files, no folders
		std::vector<FileData*> files = rootFolder->getFilesRecursive(true);
		//iterate through all files, checking if they're already in the XML
		std::vector<FileData*>::const_iterator fit = files.cbegin();
		while(fit != files.cend())
		{
			//try to cast to gamedata
			const GameData * game = dynamic_cast<const GameData*>(*fit);
			if (game != nullptr)
			{
				//worked. check if this games' path can be found somewhere in the XML
				for(pugi::xml_node gameNode = root.child("game"); gameNode; gameNode = gameNode.next_sibling("game"))
				{
					//get path from game node
					pugi::xml_node pathNode = gameNode.child("path");
					if(!pathNode)
					{
						LOG(LogError) << "<game> node contains no <path> child!";
						continue;
					}

					//check paths. use the same directory separators
					boost::filesystem::path nodePath(pathNode.text().get());
					boost::filesystem::path gamePath(game->getPath());
					if (nodePath.generic_string() == gamePath.generic_string())
					{
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
			LOG(LogError) << "Error saving gamelist.xml file \"" << xmlpath << "\"!";
		}
	}else{
		LOG(LogError) << "Found no root folder for system \"" << system->getName() << "\"!";
	}
}
