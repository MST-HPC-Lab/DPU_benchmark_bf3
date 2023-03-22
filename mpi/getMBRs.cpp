#include <iostream>
#include <stdarg.h>
#include <stdlib.h>
#include <fstream>

#include <geos_c.h>
#include <cstring>
#include <vector>
#include <sys/time.h>

using namespace std;

int processRank = -1;

static void
geos_message_handler(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

vector<GEOSGeometry *> *get_polygons(const char *filename)
{
    vector<GEOSGeometry *> *geoms = new vector<GEOSGeometry *>;
    std::ifstream input(filename);
    for (std::string line; getline(input, line);)
    {
        if (line.length() > 5 && line.find("("))
        {
            GEOSGeometry *geom = GEOSGeomFromWKT(line.c_str());
            geoms->push_back(geom);
        }
    }
    return geoms;
}

int main(int argc, char **argv)
{

    initGEOS(geos_message_handler, geos_message_handler);

    // vector<GEOSGeometry *> *geoms = get_polygons((string(argv[1]) + "/" + to_string(rank + i)).c_str());
    vector<GEOSGeometry *> *geoms = get_polygons(argv[1]);

    ofstream myfile;
    myfile.open ((string(argv[1]) + "_indexed").c_str());
    ofstream myfile2;
    myfile2.open ((string(argv[1]) + "_MBRs").c_str());

    int id = 1;
    GEOSWKTWriter *writer = GEOSWKTWriter_create();
    // GEOSWKTWriter_setTrim(writer, 1);
    // GEOSWKTWriter_setRoundingPrecision(writer, 3);
    for (auto cur = geoms->begin(); cur != geoms->end(); ++cur)
    {
        GEOSGeometry *geom = *cur;
        GEOSGeometry *geomMBR = GEOSEnvelope(*cur);
        char *wkt_geom = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry *>(geom));
        char *wkt_geomMBR = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry *>(geomMBR));
        myfile << id << " - " << wkt_geom << endl;
        myfile2 << id << " - " << wkt_geomMBR << endl;
        id++;
    }
    GEOSWKTWriter_destroy(writer);
    myfile.close();
    myfile2.close();

    finishGEOS();
    return 0;
}