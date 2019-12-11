//----------------------------------------------------------------------------------------------------------------------------
// Filename: DarwinNetcdfs.hpp
// Author: Nargess Memarsadeghi, Code 586, 2018
// Description: This class supports loading and storing data from netcdf files of MIT Dawrin project including Phtoplankton data.
// Data was obtained from Oliver Jahn (jahn@mit.edu). NASA POC for this project is Dr. Stephanie Uz
//---------------------------------------------------------------------------------------------------------------------------
#pragma once

//#include <netcdf_mem.h>
//#include <netcdf_meta.h>

#include <iostream>
#include <osg\Array>
#include <Algorithm>

#include <string>
#include <netcdf.h>

#define PI  3.14159265358

//#define FILE_NAME "..\\..\\Data\\00000.nc"
#define ERR(e) { cerr<<"Error: "<<nc_strerror(e)<<endl; return 2; }	  

using namespace std;
using namespace osg;

class DarwinNetcdfs {

public:

	// Reading 3 dimensional grids of NZ planes of size NLAT x NLON

	static const int NZ = 20; //this is referred to as Z in .nc files
	static const int NZF = 21;
	static const int NLAT = 900;
	static const int NLON = 1800;

	float   lats[NLAT];
	float   lons[NLON];
	float   Zs[NZ];
	float   Zfs[NZF];

	float VelE[NZ][NLAT][NLON];
	float VelN[NZ][NLAT][NLON];
	float VelUp[NZF][NLAT][NLON];

	float  diatom[NZ][NLAT][NLON];
	float    dino[NZ][NLAT][NLON];
	float    coco[NZ][NLAT][NLON];
	float    prok[NZ][NLAT][NLON];

	bool _coco = 0;
	bool _prok = 0;
	bool _diatom = 0;
	bool _dino = 0;

	osg::Vec4 cocoColor= Vec4(166.0f / 255.0f, 206.0f / 255.0f, 227.0f / 255.0f, 1.0f);	// pale blue for cocos																				
	osg::Vec4 prokColor= Vec4(31.0f / 255.0f, 120.0f / 255.0f, 180.0f / 255.0f, 1.0f);	// dark blue  for proks
																				//osg::Vec4 darkBlue(0.0f, 0.0f,1.0f, 1.0f);	// blue  for proks
	osg::Vec4 dinoColor = Vec4(178.0f / 255.0f, 223.0f / 255.0f, 138.0f / 255.0f, 1.0f);	// pale green for dinos
	osg::Vec4 diatomColor = Vec4(51.0f / 255.0f, 160.0f / 255.0f, 44.0f / 255.0f, 1.0f);	// dark green for diatoms

	double cocoScale = 1.5;
    double diatomScale = 3.8;
	double dinoScale   = 2.0;
	double prokScale = 1.0;

	//*************************** function protoytypes *************************************
	// read a data array from netcdf file
	template <class T>
	int read_data_parameter(int ncid, const char* attribute_name, T* data_array);

	//read a two-dimensional data array from netcdf file
	template <class T>
	int read_data_parameter(int ncid, const char* attribute_name, T** data_value);

	// read three dimensional data array from netcdf file
	template <class T>
	int read_data_parameter(int ncid, const char* attribute_name, T data_value[][NLAT][NLON]);

	template <class T>
	int read_data_parameter(int ncid, const char* attribute_name, T*** data_value);

	int readDarwinData(const char* filename);

	// source: 	https://github.com/openscenegraph/OpenSceneGraph/blob/72ab22e539de6cc1084799bf0936a24c578f342f/include/osg/CoordinateSystemNode
	void convertLatLongHeightToXYZ(double latitude, double longitude, double height, double& X, double& Y, double& Z);


private:
	

};


