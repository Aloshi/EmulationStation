#include "XMLReader.h"
#include "SystemData.h"
#include "GameData.h"
#include "pugiXML/pugixml.hpp"
#include <boost/filesystem.hpp>

//this is obviously an incredibly inefficient way to go about searching
//some day I may change this to use hash tables or something
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

void parseXMLFile(std::string xmlpath)
{
	std::cout << "Parsing XML file \"" << xmlpath << "\"...\n";

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

		GameData* game = NULL;
		for(unsigned int i = 0; i < SystemData::sSystemVector.size(); i++)
		{
			game = searchFolderByPath(SystemData::sSystemVector.at(i)->getRootFolder(), path);
			if(game != NULL)
				break;
		}

		if(game == NULL)
		{
			std::cerr << "Error - game of path \"" << path << "\" was not found by the system's search. Ignoring.\n";
		}else{
			//actually gather the information in the XML doc, then pass it to the game's set method
			std::string newName, newDesc, newImage;

			if(gameNode.child("name"))
				newName = gameNode.child("name").text().get();
			if(gameNode.child("desc"))
				newDesc = gameNode.child("desc").text().get();
			if(gameNode.child("image"))
				newImage = gameNode.child("image").text().get();

			game->set(newName, newDesc, newImage);
		}
	}

	std::cout << "XML parsing complete.\n";
}
