##############################################################
# Daimonin cmake file
##############################################################

UNSET(BOOST_THREAD_INCLUDE CACHE)
UNSET(BOOST_THREAD_LIBRARY CACHE)
IF (WIN32)
  IF (MINGW)
    SET(IDE_FOLDER CodeBlocks)
  ELSE (MINGW)
    SET(IDE_FOLDER VisualC)
  ENDIF (MINGW)
  SET(BOOST_THREAD_INCLUDE "./make/win32/${IDE_FOLDER}/OgreSDK/boost")
  FIND_LIBRARY(BOOST_THREAD_LIBRARY libboost_thread${POSTFIX_DEBUG}
    PATHS
    ${BOOST_THREAD_INCLUDE}/lib
    )

ELSE (WIN32)
  FIND_PATH(BOOST_THREAD_INCLUDE mutex.hpp
    PATHS
    /usr/include/boost/thread
    /opt/include/boost/thread
    /usr/local/include/boost/thread
    /opt/local/include/boost/thread
    )
  FIND_LIBRARY(BOOST_THREAD_LIBRARY libboost_thread.so
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

IF    (BOOST_THREAD_INCLUDE)
  MESSAGE(STATUS "* Boost include was found.")
ELSE  (BOOST_THREAD_INCLUDE)
  MESSAGE(FATAL_ERROR " * ERROR: Boost include was not found!")
ENDIF (BOOST_THREAD_INCLUDE)

IF    (BOOST_THREAD_LIBRARY)
  GET_FILENAME_COMPONENT(LIB_NAME ${BOOST_THREAD_LIBRARY} NAME)
  MESSAGE(STATUS "* Boost Thread library was found: " ${LIB_NAME})
ELSE  (BOOST_THREAD_LIBRARY)
  MESSAGE(FATAL_ERROR " * ERROR: Boost Thread library was not found!")
ENDIF (BOOST_THREAD_LIBRARY)
MARK_AS_ADVANCED(BOOST_THREAD_LIBRARY BOOST_THREAD_INCLUDE)
