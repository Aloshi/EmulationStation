#include "Log.h"

#include "platform.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>

LogLevel Log::reportingLevel = LogInfo;
FILE* Log::file = NULL; //fopen(getLogPath().c_str(), "w");

LogLevel Log::getReportingLevel()
{
	return reportingLevel;
}

std::string Log::getLogPath()
{
	std::string home = getConfigDirectory();
	return home + "/es_log.txt";
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
		// not open yet, print to stdout
		std::cerr << "ERROR - tried to write to log file before it was open! The following won't be logged:\n";
		std::cerr << os.str();
		return;
	}

	fprintf(getOutput(), "%s", os.str().c_str());

	//if it's an error, also print to console
	//print all messages if using --debug
	if(messageLevel == LogError || reportingLevel >= LogDebug)
		fprintf(stderr, "%s", os.str().c_str());
}
