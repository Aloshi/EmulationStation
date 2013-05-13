#include "Log.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "platform.h"

LogLevel Log::reportingLevel = LogInfo;
FILE* Log::file = NULL; //fopen(getLogPath().c_str(), "w");

LogLevel Log::getReportingLevel()
{
	return reportingLevel;
}

std::string Log::getLogPath()
{
	std::string home = getHomePath();
	return home + "/.emulationstation/es_log.txt";
}

void Log::setReportingLevel(LogLevel level)
{
	reportingLevel = level;
}

void Log::open()
{
	file = fopen(getLogPath().c_str(), "w");
}

std::ostringstream& Log::get(LogLevel level)
{
	os << "lvl" << level << ": \t";
	messageLevel = level;

	return os;
}

void Log::flush()
{
	fflush(getOutput());
}

void Log::close()
{
	fclose(file);
	file = NULL;
}

FILE* Log::getOutput()
{
	return file;
}

Log::~Log()
{
	os << std::endl;

	if(getOutput() == NULL)
	{
		std::cerr << "ERROR - tried to write to log file before it was open!\n";
		return;
	}

	fprintf(getOutput(), "%s", os.str().c_str());

	//if it's an error, also print to console
	if(messageLevel == LogError)
		fprintf(stderr, "%s", os.str().c_str());
}
