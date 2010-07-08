##############################################################
# Daimonin cmake file
##############################################################
IF (WIN32)
  #Todo
ELSE (WIN32)
  FIND_LIBRARY(CAUDIO_LIBRARY cAudio
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

IF (CAUDIO_LIBRARY) 
  SET(CAUDIO_FOUND 1)
  MESSAGE(STATUS "* cAudio was found.")
ELSE (CAUDIO_LIBRARY)
  SET(CAUDIO_FOUND 0)
  MESSAGE(STATUS "* Results for cAudio:")
  Message(STATUS "library: " ${CAUDIO_LIBRARY})
ENDIF (CAUDIO_LIBRARY)

MARK_AS_ADVANCED(CAUDIO_LIBRARYY)
