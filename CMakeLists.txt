cmake_minimum_required(VERSION 2.4)

PROJECT(game_client)

ADD_DEFINITIONS(-ggdb)

find_package(Entc)

INCLUDE_DIRECTORIES( ${ENTC_INCLUDE_DIRS} ${Q5_INCLUDE_DIRS} )

SET (GAMECLIENT_SOURCES
  main.cpp
)

SET (CMAKE_DISABLE_FIND_PACKAGE_ENET TRUE)
find_package(enet)

IF( ENET_LIBRARY )


ELSE(ENET_LIBRARY)

SUBDIRS(thirdparty/enet)
INCLUDE_DIRECTORIES(thirdparty/enet/include)

ENDIF(ENET_LIBRARY)

SET (CMAKE_DISABLE_FIND_PACKAGE_CURSES TRUE)

find_package(Curses)

IF( CURSES_LIBRARY )

  include_directories(${CURSES_INCLUDE_DIR})

  ADD_EXECUTABLE( game_client ${GAMECLIENT_SOURCES} )
  TARGET_LINK_LIBRARIES( game_client ${ENTC_LIBRARIES} ${CURSES_LIBRARIES} enet) 

ENDIF( CURSES_LIBRARY )
