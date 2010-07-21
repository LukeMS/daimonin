##############################################################
# Daimonin cmake file
##############################################################
IF (WIN32)
  #Todo
ELSE (WIN32)
  IF (CMAKE_SIZEOF_VOID_P MATCHES "8")
    FIND_LIBRARY(CAUDIO_LIBRARY cAudio
      PATHS
      /usr/lib64
      /opt/lib64
      /usr/local/lib64
      /opt/local/lib64
      ./make/linux/lib64
      )
  ELSE (CMAKE_SIZEOF_VOID_P MATCHES "8")
    FIND_LIBRARY(CAUDIO_LIBRARY cAudio
      PATHS
      /usr/lib
      /opt/lib
      /usr/local/lib
      /opt/local/lib
      ./make/linux/lib32
      )
  ENDIF (CMAKE_SIZEOF_VOID_P MATCHES "8")
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
