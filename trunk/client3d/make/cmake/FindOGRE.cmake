##############################################################
# Daimonin cmake file
##############################################################
IF (WIN32)
  #Todo
ELSE (WIN32)
  FIND_PATH(OGRE_INCLUDE Ogre.h
    PATHS
    /usr/include/OGRE
    /opt/include/OGRE
    /usr/local/include/OGRE
    /opt/local/include/OGRE
    )

  FIND_PATH(OGRE_LIB_PATH RenderSystem_GL.so
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

IF (OGRE_INCLUDE AND OGRE_LIBRARY) 
  SET(OGRE_FOUND 1)
  MESSAGE(STATUS "* Ogre was found.")
ELSE (OGRE_INCLUDE AND OGRE_LIBRARY)
  SET(OGRE_FOUND 0)
  MESSAGE(STATUS "* Results for Ogre:")
  Message(STATUS "include: " ${OGRE_INCLUDE})
  Message(STATUS "library: " ${OGRE_LIBRARY})
ENDIF (OGRE_INCLUDE AND OGRE_LIBRARY)

MARK_AS_ADVANCED(OGRE_INCLUDE OGRE_LIBRARY OHRE_LIB_PATH)
