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

int main(int argc, char *argv[])
{
    // Make a simple check to program arguments,
    if(argc != 3)
    {
        std::cout << "usage:\n" << argv[0] << " src_file dest_file\n";
        exit(0);
    }

    // init variables,
    const std::string name = argv[1];
    const std::string destName = argv[2];
    const char *pszFormat =  "GTiff";
    char **papszMetadata;
    char **papszOptions = NULL;
    double padfTransform[6], xStart, yStart, xEnd, yEnd;
    GDALDriver *poDriver;

    GDALAllRegister();
    GDALDataset *poDataset = (GDALDataset *) GDALOpen(name.c_str(), GA_ReadOnly );
    GDALRasterBand *data;
    float noDataValue;
    int size;
    float *buffer;
    double geot[6];
    const char *projection;

    if(poDataset == NULL)
    {
        std::cout << "Failed to open " << name << "\n";
    }
    else
    {
        poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);

        if(poDriver == NULL)
        {
            std::cout << "Cant open driver\n";
            exit(1);
        }

        papszMetadata = GDALGetMetadata(poDriver, NULL);

        if(!CSLFetchBoolean(papszMetadata, GDAL_DCAP_CREATE, FALSE))
        {
            std::cout << "Create Method not suported!\n";
        }

        if(!CSLFetchBoolean(papszMetadata, GDAL_DCAP_CREATECOPY, FALSE ))
        {
            std::cout << "CreateCopy() method not suported.\n";
        }

        // Get transform matrix, save it on padfTransform
        poDataset->GetGeoTransform(padfTransform);

        // calculate box coordinates,
        xStart = padfTransform[0] + 0*padfTransform[1] + 0*padfTransform[2];
        yStart = padfTransform[3] + 0*padfTransform[4] + 0*padfTransform[5];

        xEnd = padfTransform[0] + poDataset->GetRasterXSize()*padfTransform[1] + poDataset->GetRasterYSize()*padfTransform[2];
        yEnd = padfTransform[3] + poDataset->GetRasterXSize()*padfTransform[4] + poDataset->GetRasterYSize()*padfTransform[5];

        // Get raster band
        data = poDataset->GetRasterBand(1);

        // GDALDataType type = data->GetRasterDataType();
        int hasNoData;
        noDataValue = (float) data->GetNoDataValue(&hasNoData);
        
        if (!hasNoData) {
			noDataValue = 0;
		}

        size = data->GetXSize()*data->GetYSize();

        buffer = (float *) CPLMalloc(sizeof(float)*size);
        data->RasterIO(GF_Read, 0, 0, data->GetXSize(), data->GetYSize(), buffer, data->GetXSize(), data->GetYSize(), GDT_Float32, 0, 0 );

        // Metadata,
        poDataset->GetGeoTransform(geot);

        projection = poDataset->GetProjectionRef();

        OGRSpatialReference *ogr = new OGRSpatialReference(projection);

		ogr->importFromEPSGA(atoi(ogr->GetAttrValue("AUTHORITY", 1)));
		ogr->AutoIdentifyEPSG();
		ogr->Fixup();
	
		int xy = 0;
		if (ogr->IsProjected()) {
			xy = !ogr->EPSGTreatsAsNorthingEasting();
		}
		else {
			xy = !ogr->EPSGTreatsAsLatLong();
		}

		const std::string axisOrder = xy?"\"x\", \"y\"":"\"y\", \"x\"";// latLong?"\"y\", \"x\"":"\"x\", \"y\"";

        std::cout
                << "Reading file " << name << "\n"
                << "Axis Order = " << axisOrder << "\n"
                << "x= " << poDataset->GetRasterXSize()
                << ", h=" << poDataset->GetRasterYSize()
                << ", bands= " << poDataset->GetRasterCount() << "\n"
                << "xstart = " << xStart <<"\n"
                << "xend = " << xEnd <<"\n"
                << "ystart = " << yStart <<"\n"
                << "yend = " << yEnd <<"\n"
                << "nodatavalue = " << (hasNoData?"":"N/A, default = ") << noDataValue << "\n"
                << "Projection: " << ogr->GetAttrValue("AUTHORITY", 0) << ":" << ogr->GetAttrValue("AUTHORITY", 1) << "\n"
                << "size=" << size << " , w*h = " << poDataset->GetRasterXSize()*poDataset->GetRasterYSize() << "\n";
        ;

        std::fstream json(destName.c_str(), std::ios::trunc | std::ios::out);

        json << "{"
             "\"type\" : \"Coverage\","
             "\"domain\" : {"
             "\"type\": \"Domain\","
             "\"domainType\": \"Grid\","
             "\"axes\": {"
             "\"x\": { \"start\": " << xStart << ", \"stop\": " << xEnd << ", \"num\": " << poDataset->GetRasterXSize() << " },"
             "\"y\": { \"start\": " << yStart << ", \"stop\": " << yEnd << ", \"num\": " << poDataset->GetRasterYSize() << " }"
             "},"
             "\"referencing\": [{"
             "\"coordinates\": [" << axisOrder << "],"
             "\"system\": {"
             "\"type\": \"GeodeticCRS\","
             "\"id\": \"http://www.opengis.net/def/crs/" << ogr->GetAttrValue("AUTHORITY", 0) << "/0/" << ogr->GetAttrValue("AUTHORITY", 1) << "\""
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
             "\"axisNames\": [\"y\",\"x\"]," // << axisOrder << "],"
             "\"shape\": [" << poDataset->GetRasterYSize() <<", "<< poDataset->GetRasterXSize() << "],"
             "\"values\" : [ "
             ;

        unsigned int i;
        for(i=0; i<size; i++)
        {
            float value = buffer[i];

            if (value == noDataValue)
            {
                json << "null";
            }
            else
            {
                json << value;
            }

            if (i!=size-1)
            {
                json << ",";
            }
        }

        json << "]}}}";

        json.close();
        GDALClose(poDataset);

        std::cout << "\n\nGenerated file saved to: " << destName << "\n";

    }

    exit(0);
}
