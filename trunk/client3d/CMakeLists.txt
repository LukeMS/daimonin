############################################################
# Daimonin 3D client
############################################################
cmake_minimum_required(VERSION 2.6)

PROJECT(client3d C CXX) ### C switch is needed for the zlib files.
SET(client3D_MAJOR_VERSION  0)
SET(client3D_MINOR_VERSION  2)
SET(client3D_PATCH_VERSION  2)
ADD_DEFINITIONS(-DCLIENT3D_VERSION="${client3D_MAJOR_VERSION}.${client3D_MINOR_VERSION}.${client3D_PATCH_VERSION}")

IF   ("${CMAKE_BUILD_TYPE}" MATCHES "Debug")
  SET(POSTFIX_DEBUG "_d") ### For using in Find*.cmake
ENDIF("${CMAKE_BUILD_TYPE}" MATCHES "Debug")

# Set the folder for own cmake stuff.
SET(MY_CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/make/cmake)

SET(CMAKE_C_FLAGS "")
SET(CMAKE_C_FLAGS_DEBUG "")
SET(CMAKE_C_FLAGS_RELEASE "")
SET(CMAKE_EXE_LINKER_FLAGS "")
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "")

ADD_DEFINITIONS(-DFASTEST -DNO_GZCOMPRESS -DNO_GZIP -DBUILDFIXED) ### Used for Zlib
SET(CMAKE_CXX_STANDARD_LIBRARIES "") ### Don't add all the cmake default libraries.
if(WIN32)
  SET(LIBS ${LIBS} Ws2_32.lib)
  ADD_DEFINITIONS(-DWIN32)
  #ADD_DEFINITIONS(-Wall -W)
  Set(CMAKE_USE_RELATIVE_PATHS true) ### Without this switch it doesn't work on win32.
  if(MINGW)
    ADD_DEFINITIONS(-DMINGW)
    ADD_DEFINITIONS(-DNO_VIZ)  ### Zlib: Don't use -fvisibility switch.
    ADD_DEFINITIONS(-mthreads) ### Support thread-safe exception handling on `Mingw32'.
    SET(CMAKE_CXX_FLAGS_DEBUG    "-O0 -D_DEBUG -DDEBUG  " CACHE STRING "Debug build."   FORCE)
    SET(CMAKE_CXX_FLAGS_RELEASE  "-O2 -DNDEBUG          " CACHE STRING "Release build." FORCE)
    SET(CMAKE_EXE_LINKER_FLAGS " -Wl,--enable-auto-import -Wl,--enable-auto-image-base -Wl,--add-stdcall-alias")
    SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "-s")
  else(MINGW)
    ADD_DEFINITIONS(-DBOOST_ALL_NO_LIB)
    ADD_DEFINITIONS(-D_CONSOLE)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
    SET(CMAKE_CXX_FLAGS_DEBUG    "/MDd /D_DEBUG /DDEBUG " CACHE STRING "Debug build."   FORCE)
    SET(CMAKE_CXX_FLAGS_RELEASE  "/MD  /DNDEBUG /O2     " CACHE STRING "Release build." FORCE)
  endif(MINGW)
endif(WIN32)

#SET(CMAKE_CONFIGURATION_TYPES "DEBUG;RELEASE" CACHE STRING "Configs" FORCE)

MESSAGE(STATUS "############################################################")
MESSAGE(STATUS "# Looking for all external packages")
MESSAGE(STATUS "############################################################")
# Looking for package: boost-thread.
INCLUDE(${MY_CMAKE_MODULE_PATH}/FindBoostThread.cmake)
SET(LIBS ${LIBS} ${BOOST_THREAD_LIBRARY})

# Looking for package: Ogre3d.
INCLUDE(${MY_CMAKE_MODULE_PATH}/FindOGRE.cmake)
SET(LIBS ${LIBS} ${OGRE_LIBRARY})

# Looking for package: OIS.
INCLUDE(${MY_CMAKE_MODULE_PATH}/FindOIS.cmake)
SET(LIBS ${LIBS} ${OIS_LIBRARY})

# Looking for package: openAL
INCLUDE(${MY_CMAKE_MODULE_PATH}/FindOpenAL.cmake)
SET(LIBS ${LIBS} ${OPENAL_LIBRARY})

# Looking for package: cAudio
INCLUDE(${MY_CMAKE_MODULE_PATH}/FindCAudio.cmake)
SET(LIBS ${LIBS} ${CAUDIO_LIBRARY})

# Set all include direcetories here
INCLUDE_DIRECTORIES("3rdParty/caudio/include")
INCLUDE_DIRECTORIES("3rdParty/tinyxml")
INCLUDE_DIRECTORIES("3rdParty/zlib")
INCLUDE_DIRECTORIES("include")
INCLUDE_DIRECTORIES("${OIS_INCLUDE}")
INCLUDE_DIRECTORIES("${OGRE_INCLUDE}")
INCLUDE_DIRECTORIES("${BOOST_THREAD_INCLUDE}")

############################################################
# define all header and source files
############################################################
SET (HEADER_FILES
  3rdParty/zlib/crc32.h
  3rdParty/zlib/inffast.h
  3rdParty/zlib/inflate.h
  3rdParty/zlib/inftrees.h
  3rdParty/zlib/zconf.h
  3rdParty/zlib/zlib.h
  3rdParty/zlib/zutil.h
  3rdParty/caudio/include/cAudio.h
  3rdParty/caudio/include/IAudioDecoder.h
  3rdParty/caudio/include/IAudioDecoderFactory.h
  3rdParty/caudio/include/IAudioManager.h
  3rdParty/caudio/include/IDataSource.h
  3rdParty/caudio/include/IListener.h
  3rdParty/tinyxml/tinystr.h
  3rdParty/tinyxml/tinyxml.h
  include/gui/gui_animation.h
  include/gui/gui_cursor.h
  include/gui/gui_element.h
  include/gui/gui_element_slot.h
  include/gui/gui_element_combobox.h
  include/gui/gui_element_button.h
  include/gui/gui_element_scrollbar.h
  include/gui/gui_element_table.h
  include/gui/gui_element_listbox.h
  include/gui/gui_element_textbox.h
  include/gui/gui_element_statusbar.h
  include/gui/gui_graphic.h
  include/gui/gui_imageset.h
  include/gui/gui_manager.h
  include/gui/gui_textout.h
  include/gui/gui_window.h
  include/tile/tile_chunk.h
  include/tile/tile_decal.h
  include/tile/tile_atlas.h
  include/tile/tile_manager.h
  include/tile/tile_map.h
  include/object/object.h
  include/object/object_element.h
  include/object/object_element_animate3d.h
  include/object/object_element_avatar.h
  include/object/object_element_equip3d.h
  include/object/object_element_missile.h
  include/object/object_element_physical.h
  include/object/object_element_visual2d.h
  include/object/object_element_visual3d.h
  include/object/object_manager.h
  include/events.h
  include/logger.h
  include/assert_manager.h
  include/network.h
  include/network_account.h
  include/network_cmd_interface.h
  include/network_serverfile.h
  include/option.h
  include/sound.h
  include/spell_manager.h
  include/spell_range.h
  include/item.h
)

SET (SOURCE_FILES
  3rdParty/zlib/adler32.c
  3rdParty/zlib/crc32.c
  3rdParty/zlib/inffast.c
  3rdParty/zlib/inflate.c
  3rdParty/zlib/inftrees.c
  3rdParty/zlib/uncompr.c
  3rdParty/zlib/zutil.c
  3rdParty/tinyxml/tinystr.cpp
  3rdParty/tinyxml/tinyxml.cpp
  3rdParty/tinyxml/tinyxmlerror.cpp
  3rdParty/tinyxml/tinyxmlparser.cpp
  source/gui/gui_animation.cpp
  source/gui/gui_cursor.cpp
  source/gui/gui_element.cpp
  source/gui/gui_element_slot.cpp
  source/gui/gui_element_combobox.cpp
  source/gui/gui_element_button.cpp
  source/gui/gui_element_scrollbar.cpp
  source/gui/gui_element_table.cpp
  source/gui/gui_element_listbox.cpp
  source/gui/gui_element_textbox.cpp
  source/gui/gui_element_statusbar.cpp
  source/gui/gui_graphic.cpp
  source/gui/gui_imageset.cpp
  source/gui/gui_manager.cpp
  source/gui/gui_textout.cpp
  source/gui/gui_window.cpp
  source/tile/tile_chunk.cpp
  source/tile/tile_decal.cpp
  source/tile/tile_atlas.cpp
  source/tile/tile_manager.cpp
  source/tile/tile_map.cpp
  source/object/object.cpp
  source/object/object_element.cpp
  source/object/object_element_animate3d.cpp
  source/object/object_element_avatar.cpp
  source/object/object_element_equip3d.cpp
  source/object/object_element_missile.cpp
  source/object/object_element_physical.cpp
  source/object/object_element_visual2d.cpp
  source/object/object_element_visual3d.cpp
  source/object/object_manager.cpp
  source/events.cpp
  source/events_user.cpp
  source/logger.cpp
  source/main.cpp
  source/network.cpp
  source/network_account.cpp
  source/network_cmd.cpp
  source/network_cmd_interface.cpp
  source/network_serverfile.cpp
  source/option.cpp
  source/option_init.cpp
  source/sound.cpp
  source/spell_manager.cpp
  source/spell_range.cpp
  source/item.cpp
)
#SET_SOURCE_FILES_PROPERTIES(${SOURCE_FILES} LANGUAGE CXX) ### For compiling *.c files as *.cpp

# None build files (used to show up in the IDE only).
SET (OTHER_FILES
  media/materials/NPC.cg
  media/materials/NPC.material
  media/materials/Terrain.cg
  media/materials/Terrain.material
)
SET_SOURCE_FILES_PROPERTIES(${OTHER_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)

IF (UNIX)
  add_definitions(-DPLUGIN_FOLDER="${OGRE_PLUGINS}")
ELSE()
  add_definitions(-DPLUGIN_FOLDER=".")
ENDIF()

add_executable(client3d ${SOURCE_FILES} ${CFILES} ${HEADER_FILES} ${OTHER_FILES})
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
SET_TARGET_PROPERTIES(client3d PROPERTIES DEBUG_POSTFIX _d)
TARGET_LINK_LIBRARIES(client3d ${LIBS})

MESSAGE(STATUS "############################################################")
MESSAGE(STATUS "# Found all needed libraries")
MESSAGE(STATUS "############################################################")
