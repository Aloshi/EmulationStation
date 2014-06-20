project("pugixml")

set(PUGI_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/pugiconfig.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pugixml.hpp
)

set(PUGI_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/src/pugixml.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(pugixml STATIC ${PUGI_SOURCES} ${PUG_HEADERS})
