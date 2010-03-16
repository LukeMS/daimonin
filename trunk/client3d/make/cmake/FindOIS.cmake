##############################################################
# Daimonin cmake file
##############################################################
IF (WIN32)
  #Todo
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

IF (OIS_INCLUDE AND OIS_LIBRARY) 
  SET(OIS_FOUND 1)
  MESSAGE(STATUS "* OIS was found.")
ELSE (OIS_INCLUDE AND OIS_LIBRARY)
  SET(OIS_FOUND 0)
  MESSAGE(STATUS "* Results for OIS:")
  Message(STATUS "include: " ${OGRE_INCLUDE})
  Message(STATUS "library: " ${OGRE_LIBRARY})
ENDIF (OIS_INCLUDE AND OIS_LIBRARY)

MARK_AS_ADVANCED(OIS_INCLUDE OIS_LIBRARY)
