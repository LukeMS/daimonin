############################################################
# Daimonin 3D client
############################################################
cmake_minimum_required(VERSION 2.8)
project(client3d CXX)
add_definitions(-Wall -W)

# Version number of the project.
set (client3D_VERSION_MAJOR_VERSION 0)
set (client3D_VERSION_MINOR_VERSION 2)
set (client3D_VERSION_PATCH_LEVEL   0)

MESSAGE(STATUS "**** Create build files for Client3d Version " ${client3D_VERSION_MAJOR_VERSION} 
               "." ${client3D_VERSION_MINOR_VERSION} "." ${client3D_VERSION_PATCH_LEVEL} " ****")

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
# Looking for package: Ogre.
INCLUDE(${MY_CMAKE_MODULE_PATH}/FindOGRE.cmake)
IF (NOT OGRE_FOUND)
  MESSAGE(FATAL_ERROR "Ogre was not found.")
ENDIF(NOT OGRE_FOUND)
set(LIBS ${LIBS} ${OGRE_LIBRARY})

# Looking for package: OIS.
INCLUDE(${MY_CMAKE_MODULE_PATH}/FindOIS.cmake)
IF (NOT OIS_FOUND)
  MESSAGE(FATAL_ERROR "OIS was not found.")
ENDIF(NOT OIS_FOUND)
set(LIBS ${LIBS} ${OIS_LIBRARY})

# Looking for package: boost-thread.
INCLUDE(${MY_CMAKE_MODULE_PATH}/FindBoostThread.cmake)
IF (NOT BOOST_FOUND)
  MESSAGE(FATAL_ERROR "Boost-thread was not found.")
ENDIF(NOT BOOST_FOUND)
set(LIBS ${LIBS} ${BOOST_THREAD_LIBRARY})

# Looking for package: boost-thread.
INCLUDE(${MY_CMAKE_MODULE_PATH}/FindFmod.cmake)
IF (NOT FMOD_FOUND)
  MESSAGE(FATAL_ERROR "fmod was not found.")
ENDIF(NOT FMOD_FOUND)
set(LIBS ${LIBS} ${FMOD_LIBRARY})

link_libraries(${LIBS})
MESSAGE(STATUS "-> done")

############################################################
# define all header and source files
############################################################
set (HEADER_FILES
  3rdParty/fmod/fmod.h
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
  include/define.h
  include/tile_chunk.h
  include/tile_decal.h  
  include/tile_manager.h  
  include/tile_map.h  
  include/events.h  
  include/light_manager.h  
  include/logger.h  
  include/network.h  
  include/network_cmd_interface.h  
  include/network_serverfile.h  
  include/object_equipment.h  
  include/object_animate.h  
  include/object_visuals.h  
  include/object_manager.h  
  include/object_npc.h  
  include/object_hero.h  
  include/object_missile.h  
  include/object_static.h  
  include/option.h  
  include/particle_manager.h  
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
  source/tile_chunk.cpp  
  source/tile_decal.cpp  
  source/tile_manager.cpp  
  source/tile_map.cpp  
  source/events.cpp  
  source/events_user.cpp  
  source/light_manager.cpp  
  source/logger.cpp  
  source/main.cpp  
  source/network.cpp  
  source/network_cmd.cpp  
  source/network_cmd_interface.cpp  
  source/network_serverfile.cpp  
  source/object_equipment.cpp  
  source/object_animate.cpp  
  source/object_visuals.cpp  
  source/object_manager.cpp  
  source/object_npc.cpp  
  source/object_hero.cpp  
  source/object_missile.cpp  
  source/object_static.cpp  
  source/option.cpp  
  source/option_init.cpp  
  source/particle_manager.cpp  
  source/sound.cpp  
  source/spell_manager.cpp  
  source/spell_range.cpp  
  source/item.cpp 
)

#Set all include direcetories here
INCLUDE_DIRECTORIES(
  "3rdParty/fmod"
  "3rdParty/tinyxml"
  "3rdParty/zlib"
  "include"
  "include/gui"
  ${OGRE_INCLUDE}
  ${OIS_INCLUDE}
)

#Build the executeable
add_executable(client3d ${SOURCE_FILES})

MESSAGE(STATUS "############################################################")
MESSAGE(STATUS "# Create the file: plugins.cfg")
MESSAGE(STATUS "############################################################")
if (UNIX)
  set(PLUGIN_FOLDER ${OGRE_LIB_PATH})
else(UNIX)
  set(PLUGIN_FOLDER "./")
endif(UNIX)

file(WRITE ./plugins.cfg 
  "# Defines plugins to load\n\n"
  "# Define plugin folder\n"
  "# Choose your Ogre library path:\n"
  "PluginFolder=${PLUGIN_FOLDER}\n"
  "\n"
  "# Select OpenGL rendering system\n"
  "Plugin=RenderSystem_GL\n"
  "Plugin=Plugin_ParticleFX\n"
  "Plugin=Plugin_CgProgramManager\n"
)
MESSAGE(STATUS "-> done")

MESSAGE(STATUS "############################################################")
MESSAGE(STATUS "# You can compile the client whith the 'make' command now.")
MESSAGE(STATUS "############################################################")
