# set up the external libraries that aren't popular enough to be found on most 
# package managers are included with the project (in the 'external' folder)

# sqlite is included because although it is in package managers, it is a huge pain to set up on Windows
# (you have to generate the .lib file from the .def file for the downloaded binaries...)

add_subdirectory("nanosvg")
add_subdirectory("pugixml")
add_subdirectory("sqlite3")