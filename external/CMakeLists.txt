# set up the external libraries that aren't popular enough to be found on most
# package managers are included with the project (in the 'external' folder)

add_subdirectory("nanosvg")

find_package(pugixml)

if(NOT pugixml_FOUND)
  add_subdirectory("pugixml")
endif()
