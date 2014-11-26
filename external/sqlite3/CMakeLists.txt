project("sqlite3")

set(SQLITE3_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/sqlite3.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite3ext.h
)

set(SQLITE3_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/src/sqlite3.c
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(sqlite3 STATIC ${SQLITE3_SOURCES} ${SQLITE3_HEADERS})
