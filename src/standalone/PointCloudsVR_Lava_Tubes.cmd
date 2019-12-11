:: .cmd Script to launch Point Clouds VR in Standalone Mode -- 5/22/18, M. Brandt
::
:: Set Environment Variables
SET GDAL_DATA=%~dp0OSGeo4W64\share\gdal
SET PDAL_DRIVER_PATH=%~dp0Juniper\install\bin
SET QT_PLUGIN_PATH=%~dp0Qt\5.10.1\msvc2017_64\plugins
SET QT_QPA_PLATFORM_PLUGIN_PATH=%~dp0Qt\5.10.1\msvc2017_64\plugins\platforms
SET OSG_FILE_PATH=%~dp0OpenFrames\install

:: Set Executable Paths
SET PATH=%~dp0OSGeo4W64\bin;%PATH%;%~dp0OSG-3.5.6\install\bin;%~dp0osgEarth\install\bin;%~dp0Juniper\install\bin;%~dp0OpenVR\bin;%~dp0Qt\5.10.1\msvc2017_64\bin;%~dp0OpenFrames\install\bin

:: Launch Point Clouds VR
PUSHD %~dp0PointCloudsVR\install\bin
%~dp0PointCloudsVR\install\bin\PointCloudsVR.exe --scene lavatube
POPD
