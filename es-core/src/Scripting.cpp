#include "Scripting.h"
#include "Log.h"
#include "platform.h"
#include "utils/FileSystemUtil.h"
#ifndef WIN32
#include <errno.h>
#include <string.h>
#endif

namespace Scripting
{
    int fireEvent(const std::string& eventName, const std::string& arg1, const std::string& arg2)
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
        int ret = 0;
        // loop over found script paths per event and over scripts found in eventName folder.
        for(std::list<std::string>::const_iterator dirIt = scriptDirList.cbegin(); dirIt != scriptDirList.cend(); ++dirIt) {
            std::list<std::string> scripts = Utils::FileSystem::getDirContent(*dirIt);
            for (std::list<std::string>::const_iterator it = scripts.cbegin(); it != scripts.cend(); ++it) {
#ifndef WIN32 // osx / linux
                if (!Utils::FileSystem::isExecutable(*it)) {
                    LOG(LogWarning) << *it << " is not executable. Review file permissions.";
                    continue;
                }
#endif
                std::string script = *it;
                if (arg1.length() > 0) {
                    script += " \"" + arg1 + "\"";
                    if (arg2.length() > 0) {
                        script += " \"" + arg2 + "\"";
                    }
                }
                LOG(LogDebug) << "executing: " << script;
                ret = runSystemCommand(script);
                if (ret != 0) {
                    LOG(LogWarning) << script << " failed with exit code != 0. Terminating processing for this event.";
#ifndef WIN32
                    if (ENOENT == errno) {
                        LOG(LogWarning) << "Exit code: " << errno << " (" << strerror(errno) << ")";
                        LOG(LogWarning) << "It is not executable by the current user (usually 'pi'). Review file permissions.";
                    }
#endif
                    return ret;
                }
            }
        }
        return ret;
    }

} // Scripting::
