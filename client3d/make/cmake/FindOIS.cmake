##############################################################
# Daimonin cmake file
##############################################################

UNSET(OIS_LIBRARY CACHE)
UNSET(OIS_INCLUDE CACHE)

IF (WIN32)
  IF (MINGW)
    SET(IDE_FOLDER CodeBlocks)
  ELSE (MINGW)
    SET(IDE_FOLDER VisualC)
  ENDIF (MINGW)

  FIND_PATH(OIS_INCLUDE OIS.h
    PATHS
    ./make/win32/${IDE_FOLDER}/OgreSDK/include/OIS
    )

  FIND_LIBRARY(OIS_LIBRARY OIS${POSTFIX_DEBUG}
    PATHS
    ./make/win32/${IDE_FOLDER}/OgreSDK/lib/debug
    ./make/win32/${IDE_FOLDER}/OgreSDK/lib/release
    )

ELSE (WIN32)
  FIND_PATH(OIS_INCLUDE OIS.h
    PATHS
    /usr/include/OIS
    /opt/include/OIS
    /usr/local/include/OIS
    /opt/local/include/OIS
    )
  FIND_LIBRARY(OIS_LIBRARY OIS
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

IF    (OIS_INCLUDE)
  MESSAGE(STATUS "* OIS include was found.")
ELSE  (OIS_INCLUDE)
  MESSAGE(FATAL_ERROR " * ERROR: OIS plugins was not found!")
ENDIF (OIS_INCLUDE)

IF    (OIS_LIBRARY)
  GET_FILENAME_COMPONENT(LIB_NAME ${OIS_LIBRARY} NAME)
  MESSAGE(STATUS "* OIS library was found: " ${LIB_NAME})
ELSE  (OIS_LIBRARY)
  MESSAGE(FATAL_ERROR " * ERROR: OIS library was not found!")
ENDIF (OIS_LIBRARY)

MARK_AS_ADVANCED(OIS_INCLUDE OIS_LIBRARY)
