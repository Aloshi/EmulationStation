#ifndef _XMLREADER_H_
#define _XMLREADER_H_

#include <string>
class SystemData;

//Loads gamelist.xml data into a SystemData.
void parseGamelist(SystemData* system);

//Writes changes to SystemData back to a previously loaded gamelist.xml.
void updateGamelist(SystemData* system);

#endif
