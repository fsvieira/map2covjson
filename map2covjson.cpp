#define CPL_DEBUG ON
#include <gdal/gdal_priv.h>
#include <gdal/cpl_conv.h> // for CPLMalloc()
#include <gdal/cpl_string.h>
#include <iostream>
#include <ostream>
#include <istream>
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <stdlib.h>

int main(int argc, char *argv[]){
	GDALDataset  *poDataset;
	GDALAllRegister();
	
	if(argc != 3){
		std::cout << "usage:\n" << argv[0] << " src_file dest_file\n";
		exit(0);  
	}
	
	const std::string name = argv[1]; 
	const std::string destName = argv[2]; 

	poDataset = (GDALDataset *) GDALOpen(name.c_str(), GA_ReadOnly );

	if( poDataset == NULL ){
	   std::cout << "Failed to open " << name << "\n"; 
	}
	else{
		const char *pszFormat =  "GTiff";
		char **papszMetadata;

		GDALDriver *poDriver;

		poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);

		if( poDriver == NULL ){
			std::cout << "Cant open driver\n"; 
			exit(1);       
		}
		
		papszMetadata = GDALGetMetadata(poDriver, NULL);
		if( !CSLFetchBoolean(papszMetadata, GDAL_DCAP_CREATE, FALSE)){
			std::cout << "Create Method not suported!\n";
		}
		
		if( !CSLFetchBoolean( papszMetadata, GDAL_DCAP_CREATECOPY, FALSE ) ){
			std::cout << "CreateCopy() method not suported.\n";
		}  

		char **papszOptions = NULL;

	  std::cout << "Reading file " << name << "\n"; 
	  std::cout << "x= " << poDataset->GetRasterXSize()
							 << ", h=" << poDataset->GetRasterYSize() <<
							 ", bands= " << poDataset->GetRasterCount() << "\n";
	  
	  double padfTransform[6];
	  poDataset->GetGeoTransform(padfTransform);
	
	  double xStart = padfTransform[0] + 0*padfTransform[1] + 0*padfTransform[2];
	  double yStart = padfTransform[3] + 0*padfTransform[4] + 0*padfTransform[5];

	  double xEnd = padfTransform[0] + poDataset->GetRasterXSize()*padfTransform[1] + poDataset->GetRasterYSize()*padfTransform[2];
	  double yEnd = padfTransform[3] + poDataset->GetRasterXSize()*padfTransform[4] + poDataset->GetRasterYSize()*padfTransform[5];
	  
	  std::cout 
		<< "xstart = " << xStart <<"\n"
		<< "xend = " << xEnd <<"\n"
		<< "ystart = " << yStart <<"\n"
		<< "yend = " << yEnd <<"\n"
		;
	   
	
	  GDALRasterBand *data; 
	  data = poDataset->GetRasterBand(1);   
	    
       
    GDALDataType type = data->GetRasterDataType(); 
    
    int size = data->GetXSize()*data->GetYSize();
		
	  std::cout << "size=" << size << " , w*h = " << poDataset->GetRasterXSize()*poDataset->GetRasterYSize() << "\n";

    float *buffer;
    buffer = (float *) CPLMalloc(sizeof(float)*size);
    data->RasterIO(GF_Read, 0, 0, data->GetXSize(), data->GetYSize(), buffer, data->GetXSize(), data->GetYSize(), GDT_Float32, 0, 0 );
       
       // Metadata, 
     double geot[6]; 
     poDataset->GetGeoTransform(geot); 
       
     unsigned int i;
     float max=0, min=0; 
       
     for(i=0; i<size; i++){
		if(max < buffer[i]){
			max = buffer[i];
		}
				
		if(min > buffer[i]){
			min = buffer[i]; 
		}
	 }
       
     float range = max - min; 
     std::cout << "range=" << range << ", max=" << max << ", min=" << min << "\n";  
     std::map<float, unsigned int> counter;  

	 std::fstream json(destName.c_str(), std::ios::trunc | std::ios::out);
	 // json << "{\"w\":" << poDataset->GetRasterXSize() << ",\"h\":" << poDataset->GetRasterXSize() << ",\"data\":[";
	 json << "{"
		"\"type\" : \"Coverage\","
		"\"domain\" : {"
		"\"type\": \"Domain\","
		"\"domainType\": \"Grid\","
		"\"axes\": {"
		  "\"x\": { \"start\": "<< xStart << ", \"stop\": "<< xEnd << ", \"num\": "<< poDataset->GetRasterXSize() << " },"
		  "\"y\": { \"start\": "<< yStart << ", \"stop\": "<< yStart << ", \"num\": "<< poDataset->GetRasterYSize() << " }"
		"},"
		"\"referencing\": [{"
		  "\"coordinates\": [\"x\",\"y\"],"
		  "\"system\": {"
			"\"type\": \"GeodeticCRS\","
			"\"id\": \"http://www.opengis.net/def/crs/OGC/1.3/CRS84\""
		  "}"
		"}]"
	  "},"
	  "\"parameters\" : {"
		"\"height\": {"
		  "\"type\" : \"Parameter\","
		  "\"unit\" : {"
			"\"label\": {"
			  "\"en\": \"Meter\""
			"},"
			"\"symbol\": {"
			  "\"value\": \"m\","
			  "\"type\": \"http://www.opengis.net/def/uom/UCUM/\""
			"}"
		  "},"
		  "\"observedProperty\" : {"
			"\"id\" : \"http://vocab.nerc.ac.uk/standard_name/height/\","
			"\"label\" : {"
			  "\"en\": \"Height\""
			"}"
		  "}"
		"}"
	  "},"
	  "\"ranges\" : {"
		"\"height\" : {"
		  "\"type\" : \"NdArray\","
		  "\"dataType\": \"float\","
		  "\"axisNames\": [\"y\",\"x\"],"
		  "\"shape\": [180, 360],"
		  "\"values\" : [ "
	;
	
     for(i=0; i<size; i++){
		   float value = buffer[i];
			 json << value;
			 if (i!=size-1) {
				json << ",";
			 }
	   }

		 // json << "]}"; 
		json << "]}}}";
		
		 json.close();
		 GDALClose(poDataset);
	}
    
  exit(0); 
}
