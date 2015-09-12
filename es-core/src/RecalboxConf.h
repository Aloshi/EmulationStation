//
// Created by matthieu on 12/09/15.
//

#ifndef EMULATIONSTATION_ALL_RECALBOXCONF_H
#define EMULATIONSTATION_ALL_RECALBOXCONF_H


#include <string>
#include <map>

class RecalboxConf {

public:
    RecalboxConf();

    bool loadRecalboxConf();

    bool saveRecalboxConf();

    std::string get(const std::string &name);

    void set(const std::string &name, const std::string &value);

    static RecalboxConf *sInstance;

    static RecalboxConf *getInstance();
private:
    std::map<std::string, std::string> confMap;
};


#endif //EMULATIONSTATION_ALL_RECALBOXCONF_H
