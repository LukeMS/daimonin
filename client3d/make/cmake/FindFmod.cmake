##############################################################
# Daimonin cmake file
##############################################################
IF (WIN32)
  #Todo
ELSE (WIN32)
  FIND_LIBRARY(FMOD_LIBRARY fmodex
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

IF (FMOD_LIBRARY) 
  SET(FMOD_FOUND 1)
  MESSAGE(STATUS "* fmod was found.")
ELSE (FMOD_LIBRARY)
  SET(FMOD_FOUND 0)
  MESSAGE(STATUS "* Results for fmod:")
  Message(STATUS "library: " ${FMOD_LIBRARY})
ENDIF (FMOD_LIBRARY)

MARK_AS_ADVANCED(FMOD_LIBRARYY)
