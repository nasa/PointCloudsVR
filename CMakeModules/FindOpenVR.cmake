# - Find OpenVR SDK
# Find the OpenVR SDK headers and libraries.
#
#  OPENVR_SDK_INCLUDE_DIRS - where to find openvr.h, etc.
#  OPENVR_SDK_LIBRARIES    - List of libraries when using OpenVR SDK.
#  OPENVR_SDK_FOUND        - True if OpenVR SDK found.

IF (DEFINED ENV{OPENVR_SDK_ROOT_DIR})
    SET(OPENVR_SDK_ROOT_DIR "$ENV{OPENVR_SDK_ROOT_DIR}")
ENDIF()
SET(OPENVR_SDK_ROOT_DIR
    "${OPENVR_SDK_ROOT_DIR}"
    CACHE
    PATH
    "Root directory to search for OpenVR SDK")

# Look for the header file.
FIND_PATH(OPENVR_SDK_INCLUDE_DIRS NAMES openvr.h HINTS 
	${OPENVR_SDK_ROOT_DIR}/headers )

# Determine architecture
IF(CMAKE_SIZEOF_VOID_P MATCHES "8")
  IF(WIN32)
    SET(_OPENVR_SDK_LIB_ARCH "win64")
  ELSEIF(APPLE)
    # OpenVR only uses 32-bit on OSX
    SET(_OPENVR_SDK_LIB_ARCH "osx32")
  ENDIF()
ELSE()
  IF(WIN32)
    SET(_OPENVR_SDK_LIB_ARCH "win32")
  ELSEIF(APPLE)
    SET(_OPENVR_SDK_LIB_ARCH "osx32")
  ENDIF()
ENDIF()

MARK_AS_ADVANCED(_OPENVR_SDK_LIB_ARCH)

# Append "d" to debug libs on windows platform
IF (WIN32)
	SET(CMAKE_DEBUG_POSTFIX d)
ENDIF()

# Determine the compiler version for Visual Studio
IF (MSVC)
	# Visual Studio 2010
	IF(MSVC10)
		SET(_OPENVR_MSVC_DIR "VS2010")
	ENDIF()
	# Visual Studio 2012
	IF(MSVC11)
		SET(_OPENVR_MSVC_DIR "VS2012")
	ENDIF()
	# Visual Studio 2013
	IF(MSVC12)
		SET(_OPENVR_MSVC_DIR "VS2013")
	ENDIF()
ENDIF()


# Locate OpenVR license file
SET(_OPENVR_SDK_LICENSE_FILE "${OPENVR_SDK_ROOT_DIR}/LICENSE")
IF(EXISTS "${_OPENVR_SDK_LICENSE_FILE}") 
	SET(OPENVR_SDK_LICENSE_FILE "${_OPENVR_SDK_LICENSE_FILE}" CACHE INTERNAL "The location of the OpenVR SDK license file")
ENDIF()

# Look for the library.
FIND_LIBRARY(OPENVR_SDK_LIBRARY NAMES openvr_api HINTS ${OPENVR_SDK_ROOT_DIR} 
                                                      ${OPENVR_SDK_ROOT_DIR}/lib/${_OPENVR_SDK_LIB_ARCH}
                                                    )
    
MARK_AS_ADVANCED(OPENVR_SDK_LIBRARY)
MARK_AS_ADVANCED(OPENVR_SDK_LIBRARY_DEBUG)

# No debug library for OpenVR
SET(OPENVR_SDK_LIBRARY_DEBUG ${OPENVR_SDK_LIBRARY})

SET(OPENVR_SDK_LIBRARIES optimized ${OPENVR_SDK_LIBRARY} debug ${OPENVR_SDK_LIBRARY_DEBUG})

# handle the QUIETLY and REQUIRED arguments and set OPENVR_SDK_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OpenVR_SDK DEFAULT_MSG OPENVR_SDK_LIBRARIES OPENVR_SDK_INCLUDE_DIRS)

MARK_AS_ADVANCED(OPENVR_SDK_LIBRARIES OPENVR_SDK_INCLUDE_DIRS)
