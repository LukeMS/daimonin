##############################################################
# Daimonin cmake file
##############################################################

UNSET(CAUDIO_LIBRARY CACHE)
#  IF ("${CMAKE_BUILD_TYPE}" MATCHES "Release")
    set(LIB_NAME cAudio)
#  ELSE ()
#   set(LIB_NAME cAudio_d)
#  ENDIF ()

IF (WIN32)
  IF (MINGW)
    set(IDE_FOLDER CodeBlocks)
  ELSE (MINGW)
    set(IDE_FOLDER VisualC)
  ENDIF (MINGW)
  FIND_LIBRARY(CAUDIO_LIBRARY ${LIB_NAME}
    PATHS
    ./make/win32/${IDE_FOLDER}/Sound
    )
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

IF    (CAUDIO_LIBRARY)
  GET_FILENAME_COMPONENT(LIB_NAME ${CAUDIO_LIBRARY} NAME)
  MESSAGE(STATUS "* cAudio library was found: " ${LIB_NAME})
ELSE  (CAUDIO_LIBRARY)
  MESSAGE(FATAL_ERROR " * ERROR: cAudio library was not found!")
ENDIF (CAUDIO_LIBRARY)

MARK_AS_ADVANCED(CAUDIO_LIBRARY)
