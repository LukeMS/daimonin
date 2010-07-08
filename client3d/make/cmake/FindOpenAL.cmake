##############################################################
# Daimonin cmake file
##############################################################
IF (WIN32)
  #Todo
ELSE (WIN32)
  FIND_LIBRARY(OPENAL_LIBRARY openal
    PATHS
    ./make/linux
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

IF (OPENAL_LIBRARY) 
  SET(OPENAL_FOUND 1)
  MESSAGE(STATUS "* openAL was found.")
ELSE (OPENAL_LIBRARY)
  SET(OPENAL_FOUND 0)
  MESSAGE(STATUS "* Results for openAL:")
  Message(STATUS "library: " ${OPENAL_LIBRARY})
ENDIF (OPENAL_LIBRARY)

MARK_AS_ADVANCED(OPENAL_LIBRARYY)
