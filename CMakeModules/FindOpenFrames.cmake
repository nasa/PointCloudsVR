# This module defines

# OPENFRAMES_FOUND, if false, do not try to link to OpenFrames
# OPENFRAMES_LIBRARY, Main OpenFrames link library
# OPENFRAMES_C_LIBRARY, OpenFrames C-Interface link library
# OPENFRAMES_FORTRAN_LIBRARY, OpenFrames Fortran-Interface link library
# OPENFRAMES_LIBRARIES, All OpenFrames link libraries
# OPENFRAMES_INCLUDE_DIR, Main OpenFrames header path
# OPENFRAMES_INCLUDE_DIRS, All OpenFrames header paths

# to use this module, set variables to point to the osg build
# directory, and source directory, respectively
# OPENFRAMES_DIR: OpenFrames build directory, place in which you've installed OpenFrames via cmake

# Header files are presumed to be included like
# #include <OpenFrames/ReferenceFrame.hpp>

###### headers ######

MACRO( FIND_OPENFRAMES_INCLUDE THIS_OPENFRAMES_INCLUDE_DIR THIS_OPENFRAMES_INCLUDE_FILE )

FIND_PATH( ${THIS_OPENFRAMES_INCLUDE_DIR} ${THIS_OPENFRAMES_INCLUDE_FILE}
    PATHS
        ${OPENFRAMES_DIR}
        $ENV{OPENFRAMES_DIR}
        /usr/local/
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OPENFRAMES_ROOT]/
    PATH_SUFFIXES
        /include/
)

ENDMACRO( FIND_OPENFRAMES_INCLUDE THIS_OPENFRAMES_INCLUDE_DIR THIS_OPENFRAMES_INCLUDE_FILE )

FIND_OPENFRAMES_INCLUDE( OPENFRAMES_INCLUDE_DIR OpenFrames/ReferenceFrame.hpp )

###### libraries ######

MACRO( FIND_OPENFRAMES_LIBRARY MYLIBRARY MYLIBRARYNAME )

FIND_LIBRARY(${MYLIBRARY}
    NAMES
        ${MYLIBRARYNAME}
    PATHS
        ${OPENFRAMES_DIR}
        $ENV{OPENFRAMES_DIR}
        /usr/local
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OPENFRAMES_ROOT]/lib
    PATH_SUFFIXES
        /lib/
        /lib64/
     )

ENDMACRO(FIND_OPENFRAMES_LIBRARY LIBRARY LIBRARYNAME)

FIND_OPENFRAMES_LIBRARY( OPENFRAMES_LIBRARY OpenFrames)
FIND_OPENFRAMES_LIBRARY( OPENFRAMES_C_LIBRARY OpenFrames_Interface_C)
FIND_OPENFRAMES_LIBRARY( OPENFRAMES_FORTRAN_LIBRARY OpenFrames_Interface_IFORT)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OpenFrames DEFAULT_MSG OPENFRAMES_LIBRARY OPENFRAMES_INCLUDE_DIR)

IF(OPENFRAMES_FOUND)
  SET( OPENFRAMES_INCLUDE_DIRS ${OPENFRAMES_INCLUDE_DIR})
  SET( OPENFRAMES_LIBRARIES ${OPENFRAMES_LIBRARY} ${OPENFRAMES_C_LIBRARY} ${OPENFRAMES_FORTRAN_LIBRARY})
  GET_FILENAME_COMPONENT( OPENFRAMES_LIBRARIES_DIR ${OPENFRAMES_LIBRARY} PATH )
ENDIF( OPENFRAMES_FOUND )
