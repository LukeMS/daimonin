##############################################################
# Daimonin cmake file
##############################################################

UNSET(OPENAL_LIBRARY CACHE)
#  IF ("${CMAKE_BUILD_TYPE}" MATCHES "Release")
    set(LIB_NAME openal)
#  ELSE ()
#   set(LIB_NAME openal_d)
#  ENDIF ()

IF (WIN32)
  # OpenAl comes with cAudio...
    RETURN()
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

IF    (OPENAL_LIBRARY)
  GET_FILENAME_COMPONENT(LIB_NAME ${OPENAL_LIBRARY} NAME)
  MESSAGE(STATUS "* OpenAL library was found: " ${LIB_NAME})
ELSE  (OPENAL_LIBRARY)
  MESSAGE(FATAL_ERROR " * ERROR: OpenAL library was not found!")
ENDIF (OPENAL_LIBRARY)

MARK_AS_ADVANCED(OPENAL_LIBRARY)
