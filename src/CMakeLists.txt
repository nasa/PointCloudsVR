# Find OsgEarth
FIND_PACKAGE(OsgEarth)

# Give user a hint if OsgEarth was not found
IF(NOT OSGEARTH_FOUND)
    SET(OSGEARTH_DIR "" CACHE PATH "Set to base osgEarth install path")
    MESSAGE(FATAL_ERROR "OsgEarth NOT FOUND: Please set OSGEARTH_DIR variable to the osgEarth install path and re-configure.")
ENDIF()

# Specify that we should look for headers locally
# INCLUDE_DIRECTORIES(${OpenFrames_SOURCE_DIR}/include ${OPENSCENEGRAPH_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${NETCDF_DIR}/include ${BOOST_DIR} ${LIBLAS_DIR}/include $ENV{OPENFRAMES_DIR}/include $ENV{OPENVR_SDK_ROOT_DIR}/headers $ENV{QT_DIR}/include ${OPENSCENEGRAPH_INCLUDE_DIRS} ${OSGEARTH_INCLUDE_DIR})

# Create alias for current exe
SET(curr_exe PointCloudsVR)

# Find Qt
# There are two methods for using Qt Ui files as your QWidgets:
# 1) Run time processing, which loads the Ui file from the disk or Qt resources. (For example resource.qrc)
# 2) Compile time processing, which builds headers and classes for your Ui file. (For example )
# See: https://doc.qt.io/qt-5/designer-using-a-ui-file.html
# See: https://cmake.org/cmake/help/v3.0/manual/cmake-qt.7.html

FIND_PACKAGE(Qt5 COMPONENTS UiTools) # Library for building ui files at runtime using (1)

IF( NOT Qt5_FOUND )
  RETURN()
ENDIF()

IF(NOT NETCDF_DIR)
  SET(NETCDF_DIR "" CACHE PATH "Set to NetCDF base path")
  #MESSAGE(FATAL_ERROR "NetCDF NOT FOUND: Please set NETCDF_DIR variable to the NetCDF base path and re-generate.")
ENDIF()

IF(NOT BOOST_DIR)
  SET(BOOST_DIR "" CACHE PATH "Set to Boost base path")
  MESSAGE(FATAL_ERROR "Boost NOT FOUND: Please set BOOST_DIR variable to the Boost base path and re-generate.")
ENDIF()

IF(NOT LIBLAS_DIR)
  SET(LIBLAS_DIR "" CACHE PATH "Set to LibLAS base path")
  MESSAGE(FATAL_ERROR "LibLAS NOT FOUND: Please set LIBLAS_DIR variable to the LibLAS base path and re-generate.")
ENDIF()

set(CMAKE_AUTOMOC ON)

# For inclusion of resource.qrc containing .ui file using (1)
set(CMAKE_AUTORCC ON)

# For inclusion of .ui file at compile time using (2)
set(CMAKE_AUTOUIC ON)

# Specify which file should be compiled for current test
# ADD_EXECUTABLE(${curr_exe} ${curr_exe}.cpp)

# Specify headers for Qt example
FILE(GLOB osgqt_HEADERS *.hpp)
# LIST(APPEND osgqt_SOURCES
  # vlpointcloud.cpp)
FILE(GLOB osgqt_SOURCES *.cpp)

# Attach the ui files as a resource using (1)
# ADD_EXECUTABLE(${curr_exe} ${osgqt_SOURCES} ${osgqt_HEADERS} resources.qrc)
ADD_EXECUTABLE(${curr_exe} ${osgqt_SOURCES} ${osgqt_HEADERS} resources.qrc)

# Alternatively, you can prebuild .ui files

# Tell linker to link against OpenSceneGraph library
# TARGET_LINK_LIBRARIES(${curr_exe} OpenFrames ${OPENSCENEGRAPH_LIBRARIES} Qt5::Core Qt5::Widgets Qt5::UiTools)
GET_FILENAME_COMPONENT(OSGEARTH_LIB_DIR ${OSGEARTH_LIBRARY} DIRECTORY)
TARGET_LINK_LIBRARIES(${curr_exe} ${NETCDF_DIR}/lib/netcdf.lib ${BOOST_DIR}/lib64-msvc-14.1/libboost_thread-vc141-mt-x64-1_66.lib ${BOOST_DIR}/lib64-msvc-14.1/libboost_program_options-vc141-mt-x64-1_66.lib ${BOOST_DIR}/lib64-msvc-14.1/libboost_system-vc141-mt-x64-1_66.lib ${BOOST_DIR}/lib64-msvc-14.1/libboost_iostreams-vc141-mt-x64-1_66.lib ${BOOST_DIR}/lib64-msvc-14.1/libboost_filesystem-vc141-mt-s-x64-1_66.lib ${BOOST_DIR}/lib64-msvc-14.1/libboost_date_time-vc141-mt-x64-1_66.lib ${LIBLAS_DIR}/build/bin/Release/liblas.lib $ENV{OPENFRAMES_DIR}/lib/OpenFrames.lib ${OPENSCENEGRAPH_LIBRARIES} $ENV{OPENVR_SDK_ROOT_DIR}/lib/win64/openvr_api.lib Qt5::Core Qt5::Widgets Qt5::UiTools ${OSGEARTH_LIBRARY} ${OSGEARTHUTIL_LIBRARY} ${OSGEARTH_COLOR_RAMP_LIBRARY}
${OSGEARTH_LIB_DIR}/osgdb_osgearth_colorramp.lib)
#"D:/VR/OpenFrames/osgEarth/install/lib/osgdb_osgearth_colorramp.lib")
#"C:/Users/Public/Documents/AR-VR/PointCloudsVR_all/osgEarth/install/lib/osgdb_osgearth_colorramp.#lib")

# # Tell linker to link against OpenFrames library
# TARGET_LINK_LIBRARIES(${curr_exe} PRIVATE OpenFrames ${OPENSCENEGRAPH_LIBRARIES})

# # Enable OpenVR
# IF( ${OF_VR_TYPE} MATCHES ${VR_OpenVR} )
  # TARGET_INCLUDE_DIRECTORIES(${curr_exe} PUBLIC ${OPENVR_SDK_INCLUDE_DIRS})
  # TARGET_LINK_LIBRARIES(${curr_exe} PRIVATE OpenFrames ${OPENVR_SDK_LIBRARIES})
# ENDIF()

# Executable postfix needs to be explicitly specified
SET_TARGET_PROPERTIES(${curr_exe} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

# Install exe in CMAKE_INSTALL_PREFIX/bin directory
INSTALL(
  TARGETS ${curr_exe}
  RUNTIME DESTINATION bin
  )