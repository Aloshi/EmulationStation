#include "XMLReader.h"
#include "SystemData.h"
#include "GameData.h"
#include "pugiXML/pugixml.hpp"
#include <boost/filesystem.hpp>

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
	for(unsigned int i = 0; i < gamePath.length(); i++)
	{
		if(gamePath[i] != sysPath[i])
		{
			gamePath = gamePath.substr(i, gamePath.length() - i);
			break;
		}
	}

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
			std::cerr << "breaking out of loop for path \"" << gamePath << "\"\n";
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
	if(!system->hasGamelist())
		return;

	std::string xmlpath = system->getRootFolder()->getPath() + "/gamelist.xml";

	std::cout << "Parsing XML file \"" << xmlpath << "\"...";

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(xmlpath.c_str());

	if(!result)
	{
		std::cerr << "Error parsing XML file \"" << xmlpath << "\"!\n";
		std::cerr << "	" << result.description() << "\n";
		return;
	}

	pugi::xml_node root = doc.child("gameList");
	if(!root)
	{
		std::cerr << "Error - could not find <gameList> node in XML document!\n";
		return;
	}

	for(pugi::xml_node gameNode = root.child("game"); gameNode; gameNode = gameNode.next_sibling("game"))
	{
		pugi::xml_node pathNode = gameNode.child("path");
		if(!pathNode)
		{
			std::cerr << "Error - <game> node contains no <path> child!\n";
			return;
		}

		std::string path = pathNode.text().get();

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

			if(gameNode.child("name"))
				newName = gameNode.child("name").text().get();
			if(gameNode.child("desc"))
				newDesc = gameNode.child("desc").text().get();
			if(gameNode.child("image"))
			{
				newImage = gameNode.child("image").text().get();

				//expand "."
				if(newImage[0] == '.')
				{
					newImage.erase(0, 1);
					newImage.insert(0, system->getRootFolder()->getPath());
				}

				//if the image doesn't exist, forget it
				if(!boost::filesystem::exists(newImage))
					newImage = "";
			}

			game->set(newName, newDesc, newImage);

		}else{
			std::cerr << "Game at \"" << path << "\" does not exist!\n";
		}
	}

	std::cout << "done.\n";
}
