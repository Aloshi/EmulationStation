#pragma once

//This is a shabby, sort of OS-independent resource "system"
//Use bin2h: http://code.google.com/p/bin2h/
//to convert the binary files to C code,
//adjust this file with their declarations and
//then compile the files into the project

//These point to the actual PNG file data:

//from ES_logo_16.cpp
extern const size_t es_logo_16_data_len;
extern const unsigned char es_logo_16_data[];

//from ES_logo_32.cpp
extern const size_t es_logo_32_data_len;
extern const unsigned char es_logo_32_data[];
