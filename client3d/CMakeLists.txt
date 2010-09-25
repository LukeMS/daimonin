############################################################
# Daimonin 3D client
############################################################
cmake_minimum_required(VERSION 2.6)

# Project info
project(client3d CXX)
set (client3D_MAJOR_VERSION 0)
set (client3D_MINOR_VERSION 2)
set (client3D_PATCH_VERSION 0)

# Build type can be set by "cmake -DCMAKE_BUILD_TYPE=<Debug|Release|RelWithDebInfo|MinSizeRel>"
# If the build type is != "Release", "Debug" will be selected.
IF("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
  SET(CMAKE_CXX_FLAGS       "-O0 -W -Wall -g0 -DNDEBUG" CACHE STRING "Release build." FORCE)
  # Linker flags work different from the compiler flags!
  # CMAKE_EXE_LINKER_FLAGS is ALWAYS used and CMAKE_EXE_LINKER_FLAGS_DEBUG is ADDED on debug version.
  SET(CMAKE_EXE_LINKER_FLAGS "-s")
ELSE()
  SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Default build type is debug." FORCE)
  SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -W -Wall -g  -DDEBUG -D_DEBUG"  CACHE STRING "Debug build."   FORCE)
  SET(CMAKE_EXE_LINKER_FLAGS "")
ENDIF()

MESSAGE(STATUS "############################################################")
MESSAGE(STATUS "# Client3d Version: " ${client3D_MAJOR_VERSION} "." ${client3D_MINOR_VERSION})
MESSAGE(STATUS "############################################################")
MESSAGE(STATUS "* " ${CMAKE_BUILD_TYPE} " version will be build." )
MESSAGE(STATUS "* You can set the build type by:")
MESSAGE(STATUS "  'cmake -DCMAKE_BUILD_TYPE=<Debug|Release>' .")

# Set the folder for own cmake stuff.
set(MY_CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/make/cmake)

# Header file to pass some of the cmake settings
#configure_file(
#  "${PROJECT_SOURCE_DIR}/include/config.h.in"
#  "${PROJECT_BINARY_DIR}/include/config.h"
#)
#include_directories("${PROJECT_BINARY_DIR}")

MESSAGE(STATUS "############################################################")
MESSAGE(STATUS "# Looking for all external packages")
MESSAGE(STATUS "############################################################")
# Looking for package: boost-thread.
find_package(Boost 1.38 REQUIRED COMPONENTS thread)
IF   (NOT Boost_THREAD_FOUND)
  MESSAGE(FATAL_ERROR "Boost-thread was not found.")
ELSE ()
  MESSAGE(STATUS "* Boost-thread was found.")
ENDIF()
set(LIBS ${LIBS} ${Boost_THREAD_LIBRARY})

# Looking for package: Ogre.
INCLUDE(${MY_CMAKE_MODULE_PATH}/FindOGRE.cmake)
IF (NOT OGRE_FOUND)
  MESSAGE(FATAL_ERROR "Ogre was not found.")
ENDIF()
set(LIBS ${LIBS} ${OGRE_LIBRARY})

# Looking for package: OIS.
INCLUDE(${MY_CMAKE_MODULE_PATH}/FindOIS.cmake)
IF (NOT OIS_FOUND)
  MESSAGE(FATAL_ERROR "OIS was not found.")
ENDIF()
set(LIBS ${LIBS} ${OIS_LIBRARY})

# Looking for package: openAL
INCLUDE(${MY_CMAKE_MODULE_PATH}/FindOpenAL.cmake)
IF (NOT OPENAL_FOUND)
  MESSAGE(FATAL_ERROR "openAL was not found.")
ENDIF()
set(LIBS ${LIBS} ${OPENAL_LIBRARY})

# Looking for package: cAudio
INCLUDE(${MY_CMAKE_MODULE_PATH}/FindCAudio.cmake)
IF (NOT CAUDIO_FOUND)
  MESSAGE(FATAL_ERROR "cAudio was not found.")
ENDIF()
set(LIBS ${LIBS} ${CAUDIO_LIBRARY})

link_libraries(${LIBS})
MESSAGE(STATUS "-> done")

############################################################
# define all header and source files
############################################################
set (HEADER_FILES
  3rdParty/caudio/include/cAudio.h
  3rdParty/caudio/include/IAudioDecoder.h
  3rdParty/caudio/include/IAudioDecoderFactory.h
  3rdParty/caudio/include/IAudioManager.h
  3rdParty/caudio/include/IDataSource.h
  3rdParty/caudio/include/IListener.h
  3rdParty/tinyxml/tinystr.h
  3rdParty/tinyxml/tinyxml.h
  3rdParty/zlib/crc32.h
  3rdParty/zlib/inffast.h
  3rdParty/zlib/inflate.h
  3rdParty/zlib/inftrees.h
  3rdParty/zlib/zconf.h
  3rdParty/zlib/zlib.h
  3rdParty/zlib/zutil.h
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
set (SOURCE_FILES
  3rdParty/tinyxml/tinystr.cpp
  3rdParty/tinyxml/tinyxml.cpp
  3rdParty/tinyxml/tinyxmlerror.cpp
  3rdParty/tinyxml/tinyxmlparser.cpp
  3rdParty/zlib/adler32.c
  3rdParty/zlib/crc32.c
  3rdParty/zlib/inffast.c
  3rdParty/zlib/inflate.c
  3rdParty/zlib/inftrees.c
  3rdParty/zlib/uncompr.c
  3rdParty/zlib/zutil.c
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

#Set all include direcetories here
INCLUDE_DIRECTORIES(
  "3rdParty/caudio/include"
  "3rdParty/tinyxml"
  "3rdParty/zlib"
  "include"
  ${OGRE_INCLUDE}
  ${OIS_INCLUDE}
)

#Build the executeable
IF("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
  add_executable(client3d ${SOURCE_FILES} ${HEADER_FILES})
ELSE()
  add_executable(client3d_d ${SOURCE_FILES} ${HEADER_FILES})
ENDIF()

MESSAGE(STATUS "############################################################")
MESSAGE(STATUS "# Create the file: plugins.cfg")
MESSAGE(STATUS "############################################################")
IF (UNIX)
  set(PLUGIN_FOLDER ${OGRE_LIB_PATH})
ELSE()
  set(PLUGIN_FOLDER "./")
ENDIF()

IF("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
  file(WRITE ./plugins.cfg
  "# Defines plugins to load\n"
  "# Don't edit this file it will be overwritten by cmake!\n\n"
  "# Define plugin folder\n"
  "# Choose your Ogre library path:\n"
  "PluginFolder=${PLUGIN_FOLDER}\n"
  "\n"
  "# Select OpenGL rendering system\n"
  "Plugin=RenderSystem_GL\n"
  "Plugin=Plugin_ParticleFX\n"
  "Plugin=Plugin_CgProgramManager\n"
)
ELSE()
  file(WRITE ./plugins_d.cfg
  "# Defines plugins to load\n"
  "# Don't edit this file it will be overwritten by cmake!\n\n"
  "# Define plugin folder\n"
  "# Choose your Ogre library path:\n"
  "PluginFolder=${PLUGIN_FOLDER}\n"
  "\n"
  "# Select OpenGL rendering system\n"
  "Plugin=RenderSystem_GL\n"
  "Plugin=Plugin_ParticleFX\n"
  "Plugin=Plugin_CgProgramManager\n"
)
ENDIF()

MESSAGE(STATUS "-> done")

MESSAGE(STATUS "############################################################")
MESSAGE(STATUS "# You can compile the client with the 'make' command now.")
MESSAGE(STATUS "############################################################")

MESSAGE(STATUS ${CMAKE_BUILD_TYPE})

