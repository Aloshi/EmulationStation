#pragma once

class SystemData;

// Loads gamelist.xml data into a SystemData.
void parseGamelist(SystemData* system);

// Writes currently loaded metadata for a SystemData to gamelist.xml.
void updateGamelist(SystemData* system);
