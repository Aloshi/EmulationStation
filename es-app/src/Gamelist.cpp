#include "Gamelist.h"
#include "SystemData.h"
#include "pugixml/pugixml.hpp"
#include <boost/filesystem.hpp>
#include "Log.h"
#include "Settings.h"
#include "Util.h"

namespace fs = boost::filesystem;

FileData* findOrCreateFile(SystemData* system, const boost::filesystem::path& path, FileType type)
{
	// first, verify that path is within the system's root folder
	FileData* root = system->getRootFolder();
	
	bool contains = false;
	fs::path relative = removeCommonPath(path, root->getPath(), contains);
	if(!contains)
	{
		LOG(LogError) << "File path \"" << path << "\" is outside system path \"" << system->getStartPath() << "\"";
		return NULL;
	}

	auto path_it = relative.begin();
	FileData* treeNode = root;
	bool found = false;
	while(path_it != relative.end())
	{
		const std::vector<FileData*>& children = treeNode->getChildren();
		found = false;
		for(auto child_it = children.begin(); child_it != children.end(); child_it++)
		{
			if((*child_it)->getPath().filename() == *path_it)
			{
				treeNode = *child_it;
				found = true;
				break;
			}
		}

		// this is the end
		if(path_it == --relative.end())
		{
			if(found)
				return treeNode;

			if(type == FOLDER)
			{
				LOG(LogWarning) << "gameList: folder doesn't already exist, won't create";
				return NULL;
			}

			FileData* file = new FileData(type, path, system);
			treeNode->addChild(file);
			return file;
		}

		if(!found)
		{
			// don't create folders unless it's leading up to a game
			// if type is a folder it's gonna be empty, so don't bother
			if(type == FOLDER)
			{
				LOG(LogWarning) << "gameList: folder doesn't already exist, won't create";
				return NULL;
			}
			
			// create missing folder
			FileData* folder = new FileData(FOLDER, treeNode->getPath().stem() / *path_it, system);
			treeNode->addChild(folder);
			treeNode = folder;
		}

		path_it++;
	}

	return NULL;
}

void parseGamelist(SystemData* system)
{
	std::string xmlpath = system->getGamelistPath(false);

	if(!boost::filesystem::exists(xmlpath))
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

	fs::path relativeTo = system->getStartPath();

	const char* tagList[2] = { "game", "folder" };
	FileType typeList[2] = { GAME, FOLDER };
	for(int i = 0; i < 2; i++)
	{
		const char* tag = tagList[i];
		FileType type = typeList[i];
		for(pugi::xml_node fileNode = root.child(tag); fileNode; fileNode = fileNode.next_sibling(tag))
		{
			fs::path path = resolvePath(fileNode.child("path").text().get(), relativeTo, false);
			
			if(!boost::filesystem::exists(path))
			{
				LOG(LogWarning) << "File \"" << path << "\" does not exist! Ignoring.";
				continue;
			}

			FileData* file = findOrCreateFile(system, path, type);
			if(!file)
			{
				LOG(LogError) << "Error finding/creating FileData for \"" << path << "\", skipping.";
				continue;
			}

			//load the metadata
			std::string defaultName = file->metadata.get("name");
			file->metadata = MetaDataList::createFromXML(GAME_METADATA, fileNode, relativeTo);

			//make sure name gets set if one didn't exist
			if(file->metadata.get("name").empty())
				file->metadata.set("name", defaultName);
		}
	}
}

void addFileDataNode(pugi::xml_node& parent, const FileData* file, const char* tag, SystemData* system)
{
	//create game and add to parent node
	pugi::xml_node newNode = parent.append_child(tag);

	//write metadata
	file->metadata.appendToXML(newNode, true, system->getStartPath());
	
	if(newNode.children().begin() == newNode.child("name") //first element is name
		&& ++newNode.children().begin() == newNode.children().end() //theres only one element
		&& newNode.child("name").text().get() == file->getCleanName()) //the name is the default
	{
		//if the only info is the default name, don't bother with this node
		//delete it and ultimately do nothing
		parent.remove_child(newNode);
	}else{
		//there's something useful in there so we'll keep the node, add the path

		// try and make the path relative if we can so things still work if we change the rom folder location in the future
		newNode.prepend_child("path").text().set(makeRelativePath(file->getPath(), system->getStartPath(), false).generic_string().c_str());
	}
}

void updateGamelist(SystemData* system)
{
	//We do this by reading the XML again, adding changes and then writing it back,
	//because there might be information missing in our systemdata which would then miss in the new XML.
	//We have the complete information for every game though, so we can simply remove a game
	//we already have in the system from the XML, and then add it back from its GameData information...

	if(Settings::getInstance()->getBool("IgnoreGamelist"))
		return;

	pugi::xml_document doc;
	pugi::xml_node root;
	std::string xmlReadPath = system->getGamelistPath(false);

	if(boost::filesystem::exists(xmlReadPath))
	{
		//parse an existing file first
		pugi::xml_parse_result result = doc.load_file(xmlReadPath.c_str());
		
		if(!result)
		{
			LOG(LogError) << "Error parsing XML file \"" << xmlReadPath << "\"!\n	" << result.description();
			return;
		}

		root = doc.child("gameList");
		if(!root)
		{
			LOG(LogError) << "Could not find <gameList> node in gamelist \"" << xmlReadPath << "\"!";
			return;
		}
	}else{
		//set up an empty gamelist to append to
		root = doc.append_child("gameList");
	}


	//now we have all the information from the XML. now iterate through all our games and add information from there
	FileData* rootFolder = system->getRootFolder();
	if (rootFolder != nullptr)
	{
		//get only files, no folders
		std::vector<FileData*> files = rootFolder->getFilesRecursive(GAME | FOLDER);
		//iterate through all files, checking if they're already in the XML
		std::vector<FileData*>::const_iterator fit = files.cbegin();
		while(fit != files.cend())
		{
			const char* tag = ((*fit)->getType() == GAME) ? "game" : "folder";

			// check if the file already exists in the XML
			// if it does, remove it before adding
			for(pugi::xml_node fileNode = root.child(tag); fileNode; fileNode = fileNode.next_sibling(tag))
			{
				pugi::xml_node pathNode = fileNode.child("path");
				if(!pathNode)
				{
					LOG(LogError) << "<" << tag << "> node contains no <path> child!";
					continue;
				}

				fs::path nodePath = resolvePath(pathNode.text().get(), system->getStartPath(), true);
				fs::path gamePath((*fit)->getPath());
				if(nodePath == gamePath || (fs::exists(nodePath) && fs::exists(gamePath) && fs::equivalent(nodePath, gamePath)))
				{
					// found it
					root.remove_child(fileNode);
					break;
				}
			}

			// it was either removed or never existed to begin with; either way, we can add it now
			addFileDataNode(root, *fit, tag, system);

			++fit;
		}

		//now write the file

		//make sure the folders leading up to this path exist (or the write will fail)
		boost::filesystem::path xmlWritePath(system->getGamelistPath(true));
		boost::filesystem::create_directories(xmlWritePath.parent_path());

		if (!doc.save_file(xmlWritePath.c_str())) {
			LOG(LogError) << "Error saving gamelist.xml to \"" << xmlWritePath << "\" (for system " << system->getName() << ")!";
		}
	}else{
		LOG(LogError) << "Found no root folder for system \"" << system->getName() << "\"!";
	}
}
