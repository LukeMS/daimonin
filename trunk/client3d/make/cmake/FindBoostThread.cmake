##############################################################
# Daimonin cmake file
##############################################################
IF (WIN32)
  #Todo
ELSE (WIN32)
  FIND_PATH(BOOST_THREAD_INCLUDE thread.hpp
    PATHS
    /usr/include/boost/thread
    /opt/include/boost/thread
    /usr/local/include/boost/thread
    /opt/local/include/boost/thread
    )
  FIND_LIBRARY(BOOST_THREAD_LIBRARY boost_thread
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

IF (BOOST_THREAD_INCLUDE AND BOOST_THREAD_LIBRARY) 
  SET(BOOST_FOUND 1)
  MESSAGE(STATUS "* Boost was found.")
ELSE (BOOST_THREAD_INCLUDE AND BOOST_THREAD_LIBRARY)
  SET(BOOST_FOUND 0)
  MESSAGE(STATUS "* Results for Boost:")
  Message(STATUS "include: " ${BOOST_THREAD_INCLUDE})
  Message(STATUS "library: " ${BOOST_THREAD_LIBRARY})
ENDIF (BOOST_THREAD_INCLUDE AND BOOST_THREAD_LIBRARY)

MARK_AS_ADVANCED(BOOST_THREAD_INCLUDE BOOST_THREAD_LIBRARY)
