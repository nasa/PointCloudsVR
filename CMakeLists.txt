CMAKE_MINIMUM_REQUIRED(VERSION 3.5.2)
PROJECT(PointCloudsVR)

# Set where to find extra CMake modules
SET(CMAKE_MODULE_PATH "{CMAKE_MODULE_PATH}" ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)

# Make the makefile output verbose (for compiler error checking)
#SET(CMAKE_VERBOSE_MAKEFILE on)

# Default to Release configuration on single-config generators
# e.g. make, but not VisualStudio or XCode
# Note that CMAKE_BUILD_TYPE is initialized to "" on the initial cmake
# run, so we check for that and set the default appropriately
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING "Build Configuration (Release, Debug, RelWithDebInfo, or MinSizeRel" FORCE)
endif()

# Set debug postfix
SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "Postfix for debug libraries and executables")

# Enable multiprocessor builds on Visual Studio
if(MSVC)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

# Enforce C++11
SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)

# Enable RPATH on Mac/Linux so that shared libraries can be found
# relative to each other.
if(UNIX)
  if(APPLE)
    SET(CMAKE_MACOSX_RPATH TRUE) # Enable Mac RPATH
  else()
    # Linux searches the RPATH before the system LD_LIBRARY_PATH, so enable
    # the use of RUNPATH which is searched after LD_LIBRARY_PATH. This makes
    # the Linux search order similar to Mac/Windows.
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--enable-new-dtags")
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--enable-new-dtags")
  endif()
endif()

# Set debug preprocessor define
SET_PROPERTY(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:OF_DEBUG>)

# Set default installation location
# Can be overridden by using "-DCMAKE_INSTALL_PREFIX=/foo/bar" 
# at command line 
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/PointCloudsVR-${CMAKE_SYSTEM_NAME}-install" CACHE PATH "PointCloudsVR installation location" FORCE)
ENDIF()

# Find OpenFrames
FIND_PACKAGE(OpenFrames)

# Give user a hint if OpenFrames was not found
IF(NOT OPENFRAMES_FOUND)
  SET(OPENFRAMES_DIR "" CACHE PATH "Set to base OpenFrames install path")
  MESSAGE(FATAL_ERROR "OpenFrames NOT FOUND: Please set OPENFRAMES_DIR variable to the OpenFrames install path and re-generate.")
ENDIF()

# Find OpenSceneGraph
FIND_PACKAGE(OpenSceneGraph 3.5.6 COMPONENTS osg osgText osgGA osgDB osgUtil osgViewer osgParticle)

# Give user a hint if OSG was not found
IF(NOT OPENSCENEGRAPH_FOUND)
  SET(OSG_DIR "" CACHE PATH "Set to base OpenSceneGraph install path")
  MESSAGE(FATAL_ERROR "OpenSceneGraph NOT FOUND: Please set OSG_DIR variable to the OpenSceneGraph install path and re-generate.")
ENDIF()

# Find OpenVR
FIND_PACKAGE(OpenVR)
IF(NOT OPENVR_SDK_FOUND)
  MESSAGE(WARNING "OpenVR NOT FOUND. If OpenVR HMD support is needed, then please set OPENVR_SDK_ROOT_DIR to the OpenVR SDK path and re-generate.")
ENDIF()

# Find Qt
# FIND_PACKAGE(Qt5 COMPONENTS Core Widgets OpenGL)
# IF( Qt5_FOUND )
  # SET(OF_USE_QT TRUE)
# ELSE()
  # SET(OF_USE_QT FALSE)
  # MESSAGE("Qt5 not found: will not build Qt widget panel or Qt demos.")
# ENDIF()

# Find nanomsg
#FIND_PACKAGE(nanomsg)
#IF(NOT NANOMSG_FOUND)
#  SET(NANOMSG_DIR "" CACHE PATH "Set to base nanomsg install path")
#  MESSAGE(FATAL_ERROR "nanomsg not found. Please set NANOMSG_DIR to the nanomsg install path and re-generate.")
#ENDIF()

# Go to the src directory and look for CMake instructions there
ADD_SUBDIRECTORY(src)

# Install models
#INSTALL(
#   DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Models
#   DESTINATION ${CMAKE_INSTALL_PREFIX}
#  )
