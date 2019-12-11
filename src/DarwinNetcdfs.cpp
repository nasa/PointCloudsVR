//----------------------------------------------------------------------------------------------------------------------------
// Filename: DarwinNetcdfs.cpp
// Author: Nargess Memarsadeghi, Code 586, 2018
// Description: This class supports loading and storing data from netcdf files of MIT Dawrin project including Phtoplankton data.
// Data was obtained from Oliver Jahn (jahn@mit.edu). NASA POC for this project is Dr. Stephanie Uz
//---------------------------------------------------------------------------------------------------------------------------
#pragma once
#include "DarwinNetcdfs.hpp"
//*************************** function implementations *************************************
template <class T>
int DarwinNetcdfs::read_data_parameter(int ncid, const char* attribute_name, T* data_array)
{
	cout << "in first version" << endl;
	int attribute_varid = 0, retval = 0;

	cout << "attribute name is: " << attribute_name << endl;

	if ((retval = nc_inq_varid(ncid, attribute_name, &attribute_varid)))
		ERR(retval);
	
	if ((retval = nc_get_var_float(ncid, attribute_varid, data_array)))
		ERR(retval);

	return 0;
}

//------------------------------------------------------------------------------
template <class T>
int DarwinNetcdfs::read_data_parameter(int ncid, const char* attribute_name, T** data_value)
{
	cout << "in second version" << endl;
	return 0;
}
//--------------------------------------------------------------
template <class T>
int DarwinNetcdfs::read_data_parameter(int ncid, const char* attribute_name, T data_array[][NLAT][NLON])
{
	cout << "in third version" << endl;
	int attribute_varid = 0, retval = 0;

	cout << "attribute name is: " << attribute_name << endl;

	if ((retval = nc_inq_varid(ncid, attribute_name, &attribute_varid)))
		ERR(retval);

	/* Read the data. Since we know the contents of the file we know
	* that the data arrays in this program are the correct size to
	* hold one timestep. */
	size_t start[NZ], count[NZ];

	//cout << "size_t is: " << size_t << endl;

	count[0] = 1;
	count[1] = NZ;
	count[2] = NLAT;
	count[3] = NLON;
	start[0] = 0;
	start[1] = 0;
	start[2] = 0;
	start[3] = 0;

	if ((retval = nc_get_vara_float(ncid, attribute_varid, start, count, &data_array[0][0][0])))
	{
		cout << "retval: " << retval << endl;
		ERR(retval);
	}
	//test values
	/*for (int row = 0; row < 20; row++)
	{
	cout << ", data_array[0][0][" << row << "]: " << data_array[0][0][row] << endl;
	}*/

	return 0;
}
//--------------------------------------------------------------
template <class T>
int DarwinNetcdfs::read_data_parameter(int ncid, const char* attribute_name, T*** data_array)
{
	cout << "in fourth version" << endl;
	int attribute_varid = 0, retval = 0;

	cout << "attribute name is: " << attribute_name << endl;

	if ((retval = nc_inq_varid(ncid, attribute_name, &attribute_varid)))
		ERR(retval);

	/* Read the data. Since we know the contents of the file we know
	* that the data arrays in this program are the correct size to
	* hold one timestep. */
	size_t start[NZ], count[NZ];

	//cout << "size_t is: " << size_t << endl;

	count[0] = 1;
	count[1] = NZ;
	count[2] = NLAT;
	count[3] = NLON;
	start[0] = 0;
	start[1] = 0;
	start[2] = 0;
	start[3] = 0;

	if ((retval = nc_get_vara_float(ncid, attribute_varid, start, count, &data_array[0][0][0])))
	{
		cout << "retval: " << retval << endl;
		ERR(retval);
	}
	return 0;
}
//--------------------------------------------------------------------------------------------------------
int DarwinNetcdfs::readDarwinData(const char* filename)
{

	float TimeStep = -1;	// time step
	int retval = 0;
	int ncid;


	// print possible error values
	//	cout << "FILE_NAME: " << FILE_NAME << endl;
	cout << "NC_NOERR: " << NC_NOERR << endl;
	cout << "NC_ENOMEM:" << NC_ENOMEM << endl;
	cout << "NC_EHDFERR: " << NC_EHDFERR << endl;
	cout << "NC_EDIMMETA:" << NC_EDIMMETA << endl;
	cout << "NC_ENOCOMPOUND " << NC_ENOCOMPOUND << endl;

	retval = nc_open(filename, NC_NOWRITE, &ncid);	if (retval) ERR(retval);


	retval = read_data_parameter(ncid, "T", &TimeStep);	if (retval) ERR(retval);
	retval = read_data_parameter(ncid, "lat", lats);	if (retval) ERR(retval);
	retval = read_data_parameter(ncid, "lon", lons);	if (retval) ERR(retval);
	retval = read_data_parameter(ncid, "Z", Zs);		if (retval) ERR(retval);
	retval = read_data_parameter(ncid, "ZF", Zfs);		if (retval) ERR(retval);


	retval = read_data_parameter(ncid, "velE", VelE);   if (retval) ERR(retval);
	retval = read_data_parameter(ncid, "velN", VelN);   if (retval) ERR(retval);
	retval = read_data_parameter(ncid, "velUP", VelUp);	if (retval) ERR(retval);


	if (_diatom) retval = read_data_parameter(ncid, "Diatom", diatom); if (retval) ERR(retval);
	if (_coco)   retval = read_data_parameter(ncid, "Cocco", coco);	  if (retval) ERR(retval);
	if (_dino)   retval = read_data_parameter(ncid, "Dino", dino);	  if (retval) ERR(retval);
	if (_prok)   retval = read_data_parameter(ncid, "Prok", prok);	  if (retval) ERR(retval);

	//---------------------------------------------------------------
	/* Close the file, freeing all resources. */
	retval = nc_close(ncid); if (retval) ERR(retval);

	return retval;
}
//-------------------------------------------------------------------
void DarwinNetcdfs::convertLatLongHeightToXYZ(double latitude, double longitude, double height, double& X, double& Y, double& Z)
{
	// for details on maths see http://www.colorado.edu/geography/gcraft/notes/datum/gif/llhxyz.gif
	double sin_latitude = sin(latitude*PI / 180.0);
	double cos_latitude = cos(latitude*PI / 180.0);
	double _radiusEquator = 6371 * 10 ^ 3;
	double _eccentricitySquared = 0.0167 *0.0167;

	double N = _radiusEquator / sqrt(1.0 - _eccentricitySquared * sin_latitude*sin_latitude);
	X = (N + height)*cos_latitude*cos(longitude*PI / 180);
	Y = (N + height)*cos_latitude*sin(longitude*PI / 180);
	Z = (N*(1 - _eccentricitySquared) + height)*sin_latitude;
}
