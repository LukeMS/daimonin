##############################################################
# Daimonin cmake file
##############################################################

UNSET(OGRE_INCLUDE CACHE)
UNSET(OGRE_LIBRARY CACHE)
UNSET(OGRE_PLUGINS CACHE)

IF (WIN32)
  IF (MINGW)
    SET(IDE_FOLDER CodeBlocks)
    FIND_LIBRARY(OGRE_LIBRARY libOgreMain${POSTFIX_DEBUG}.dll
      PATHS
      ./make/win32/${IDE_FOLDER}/OgreSDK/lib/debug
      ./make/win32/${IDE_FOLDER}/OgreSDK/lib/release
      )
    FIND_PATH(OGRE_PLUGINS libRenderSystem_GL${POSTFIX_DEBUG}.dll.a
      PATHS
      ./make/win32/${IDE_FOLDER}/OgreSDK/lib/debug/opt
      ./make/win32/${IDE_FOLDER}/OgreSDK/lib/release/opt
      )

  ELSE (MINGW)
    SET(IDE_FOLDER VisualC)
    FIND_LIBRARY(OGRE_LIBRARY OgreMain${POSTFIX_DEBUG}.lib
      PATHS
      ./make/win32/${IDE_FOLDER}/OgreSDK/lib/debug
      ./make/win32/${IDE_FOLDER}/OgreSDK/lib/release
      )
    FIND_PATH(OGRE_PLUGINS RenderSystem_GL${POSTFIX_DEBUG}.lib
      PATHS
      ./make/win32/${IDE_FOLDER}/OgreSDK/lib/debug/opt
      ./make/win32/${IDE_FOLDER}/OgreSDK/lib/release/opt
      )
  ENDIF (MINGW)

  FIND_PATH(OGRE_INCLUDE Ogre.h
    PATHS
    ./make/win32/${IDE_FOLDER}/OgreSDK/include/Ogre
    )

ELSE (WIN32)
  FIND_PATH(OGRE_INCLUDE Ogre.h
    PATHS
    /usr/include/OGRE
    /opt/include/OGRE
    /usr/local/include/OGRE
    /opt/local/include/OGRE
    )

  FIND_PATH(OGRE_PLUGINS RenderSystem_GL.so
    PATHS
    /usr/lib/OGRE
    /opt/lib/OGRE
    /usr/lib64/OGRE
    /opt/lib64/OGRE
    /usr/local/lib/OGRE
    /opt/local/lib/OGRE
    /usr/local/lib64/OGRE
    /opt/local/lib64/OGRE
    )

  FIND_LIBRARY(OGRE_LIBRARY OgreMain
    PATHS
    /usr/lib
    /opt/lib
    /usr/lib64
    /opt/lib64
    /usr/local/lib
    /opt/local/lib
    /usr/local/lib64
    /opt/local/lib64
    )
ENDIF (WIN32)

IF    (OGRE_INCLUDE)
  MESSAGE(STATUS "* Ogre include was found.")
ELSE  (OGRE_INCLUDE)
  MESSAGE(FATAL_ERROR " * ERROR: Ogre include was not found!")
ENDIF (OGRE_INCLUDE)

IF    (OGRE_PLUGINS)
  MESSAGE(STATUS "* Ogre plugins was found.")
ELSE  (OGRE_PLUGINS)
  MESSAGE(FATAL_ERROR " * ERROR: Ogre plugins was not found!")
ENDIF (OGRE_PLUGINS)

IF    (OGRE_LIBRARY)
  GET_FILENAME_COMPONENT(LIB_NAME ${OGRE_LIBRARY} NAME)
  MESSAGE(STATUS "* Ogre library was found: " ${LIB_NAME})
ELSE  (OGRE_LIBRARY)
  MESSAGE(FATAL_ERROR " * ERROR: Ogre library was not found!")
ENDIF (OGRE_LIBRARY)

MARK_AS_ADVANCED(OGRE_INCLUDE OGRE_LIBRARY OGRE_PLUGINS)
