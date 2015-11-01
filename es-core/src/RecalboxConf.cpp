#include "RecalboxConf.h"
#include <iostream>
#include <fstream>
#include <boost/regex.hpp>
#include "Log.h"
#include <boost/algorithm/string/predicate.hpp>

RecalboxConf *RecalboxConf::sInstance = NULL;
boost::regex validLine("^(?<key>[^;|#].*?)=(?<val>.*?)$");
boost::regex commentLine("^;(?<key>.*?)=(?<val>.*?)$");

std::string recalboxConfFile = "/recalbox/share/system/recalbox.conf";
std::string recalboxConfFileTmp = "/recalbox/share/system/recalbox.conf.tmp";

RecalboxConf::RecalboxConf() {
    loadRecalboxConf();
}

RecalboxConf *RecalboxConf::getInstance() {
    if (sInstance == NULL)
        sInstance = new RecalboxConf();

    return sInstance;
}

bool RecalboxConf::loadRecalboxConf() {
    std::string line;
    std::ifstream recalboxConf(recalboxConfFile);
    if (recalboxConf && recalboxConf.is_open()) {
        while (std::getline(recalboxConf, line)) {
            boost::smatch lineInfo;
            if (boost::regex_match(line, lineInfo, validLine)) {
                confMap[std::string(lineInfo["key"])] = std::string(lineInfo["val"]);
            }
        }
        recalboxConf.close();
    } else {
        LOG(LogError) << "Unable to open " << recalboxConfFile;
        return false;
    }
    return true;
}


bool RecalboxConf::saveRecalboxConf() {
    std::ifstream filein(recalboxConfFile); //File to read from
    if (!filein) {
        LOG(LogError) << "Unable to open for saving :  " << recalboxConfFile << "\n";
        return false;
    }
    /* Read all lines in a vector */
    std::vector<std::string> fileLines;
    std::string line;
    while (std::getline(filein, line)) {
        fileLines.push_back(line);
    }
    filein.close();


    /* Save new value if exists */
    for (std::map<std::string, std::string>::iterator it = confMap.begin(); it != confMap.end(); ++it) {
        std::string key = it->first;
        std::string val = it->second;
        bool lineFound = false;
        for (int i = 0; i < fileLines.size(); i++) {
            std::string currentLine = fileLines[i];

            if (boost::starts_with(currentLine, key+"=") || boost::starts_with(currentLine, ";"+key+"=")){
                fileLines[i] = key + "=" + val;
                lineFound = true;
            }
        }
        if(!lineFound){
            fileLines.push_back(key + "=" + val);
        }
    }
    std::ofstream fileout(recalboxConfFileTmp); //Temporary file
    if (!fileout) {
        LOG(LogError) << "Unable to open for saving :  " << recalboxConfFileTmp << "\n";
        return false;
    }
    for (int i = 0; i < fileLines.size(); i++) {
        fileout << fileLines[i] << "\n";
    }

    fileout.close();


    /* Copy back the tmp to recalbox.conf */
    std::ifstream  src(recalboxConfFileTmp, std::ios::binary);
    std::ofstream  dst(recalboxConfFile,   std::ios::binary);
    dst << src.rdbuf();

    remove(recalboxConfFileTmp.c_str());

    return true;
}

std::string RecalboxConf::get(const std::string &name) {
    if (confMap.count(name)) {
        return confMap[name];
    }
    return "";
}

void RecalboxConf::set(const std::string &name, const std::string &value) {
    confMap[name] = value;
}