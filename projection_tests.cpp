#define CPL_DEBUG ON
#include <gdal/gdal_priv.h>
#include <gdal/cpl_conv.h>
#include <gdal/cpl_string.h>
#include <gdal/ogr_core.h>
#include <gdal/ogr_spatialref.h>
#include <iostream>
#include <ostream>
#include <istream>
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <stdlib.h>

void getProjectionInfo (const char *projection) {

	OGRSpatialReference *ogr = new OGRSpatialReference(projection);

	ogr->AutoIdentifyEPSG();
	ogr->Fixup();
	int latLong = ogr->EPSGTreatsAsLatLong();
	
	const std::string axisOrder = latLong?"\"y\", \"x\"":"\"x\", \"y\"";
	
	std::cout 
		<< "Projection = " << projection << "\n"
		<< "AUTHORITY = " << ogr->GetAttrValue("AUTHORITY", 0) << ":" << ogr->GetAttrValue("AUTHORITY", 1) << "\n"
		<< "EPSGTreatsAsLatLong = " << (latLong?"True":"False") << "\n"
		<< "Axis Order = " << (latLong?"Lat, Long":"Long, Lat") << " (" << axisOrder << ")\n\n"
	;
}

int main(int argc, char *argv[])
{
	
	getProjectionInfo("PROJCS[\"WGS 84 / UTM zone 29N\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",-9],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AUTHORITY[\"EPSG\",\"32629\"]]");
	getProjectionInfo("PROJCS[\"WGS 84 / UTM zone 29N\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",-9],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],AUTHORITY[\"EPSG\",\"32629\"],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH]]");
	getProjectionInfo("GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]]");

}
