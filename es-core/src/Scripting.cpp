#include "Scripting.h"
#include "Log.h"
#include "platform.h"
#include "utils/FileSystemUtil.h"

namespace Scripting
{
	void fireEvent(const std::string& eventName, const std::string& arg1, const std::string& arg2)
	{
		LOG(LogDebug) << "fireEvent: " << eventName << " " << arg1 << " " << arg2;

        std::list<std::string> scriptDirList;
        std::string test;

        // check in exepath
        test = Utils::FileSystem::getExePath() + "/scripts/" + eventName;
        if(Utils::FileSystem::exists(test))
            scriptDirList.push_back(test);

        // check in homepath
        test = Utils::FileSystem::getHomePath() + "/.emulationstation/scripts/" + eventName;
        if(Utils::FileSystem::exists(test))
            scriptDirList.push_back(test);

        for(std::list<std::string>::const_iterator dirIt = scriptDirList.cbegin(); dirIt != scriptDirList.cend(); ++dirIt) {
            std::list<std::string> scripts = Utils::FileSystem::getDirContent(*dirIt);
            for (std::list<std::string>::const_iterator it = scripts.cbegin(); it != scripts.cend(); ++it) {
                // append folder to path
                std::string script = *it + " \"" + arg1 + "\" \"" + arg2 + "\"";
                LOG(LogDebug) << "  executing: " << script;
                runSystemCommand(script);
            }
        }
	}

} // Scripting::
