#include <iostream>
#include <stdarg.h>
#include <stdlib.h>
#include <fstream>
#include <geos_c.h>
#include <cstring>
#include <vector>
#include <mpi.h>

using namespace std;

static void
geos_message_handler(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

int create_tree(const char *filename)
{
    initGEOS(geos_message_handler, geos_message_handler);

    GEOSSTRtree *tree = GEOSSTRtree_create(10);

    std::ifstream input(filename);
    for (std::string line; getline(input, line);)
    {
        if (line.length() > 5 && line.find("("))
        {
            GEOSGeometry *geom = GEOSGeomFromWKT(line.c_str());
            GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
        }
    }

    GEOSSTRtree_destroy(tree);

    finishGEOS();

    return 0;
}

int element_count = 0;
void iterate_tree_callback(void *geom, void *userdata)
{
    GEOSWKTWriter *writer = GEOSWKTWriter_create();
    GEOSWKTWriter_setTrim(writer, 1);
    GEOSWKTWriter_setRoundingPrecision(writer, 3);
    char *wkt_geom = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry *>(geom));

    GEOSWKTWriter_destroy(writer);
    cout << wkt_geom << endl;
    element_count++;
}

int iterate_tree(const char *filename)
{
    initGEOS(geos_message_handler, geos_message_handler);

    GEOSSTRtree *tree = GEOSSTRtree_create(10);

    std::ifstream input(filename);
    for (std::string line; getline(input, line);)
    {
        if (line.length() > 5 && line.find("("))
        {
            GEOSGeometry *geom = GEOSGeomFromWKT(line.c_str());
            GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
        }
    }

    GEOSSTRtree_iterate(tree, iterate_tree_callback, 0);
    cout << "# of Geometries: " << element_count << endl;

    GEOSSTRtree_destroy(tree);

    finishGEOS();

    return 0;
}

void intersect_callback(void *geom, void *base_geom)
{
    if (!GEOSIntersects(static_cast<const GEOSGeometry *>(geom), static_cast<const GEOSGeometry *>(base_geom)))
    {
        GEOSWKTWriter *writer = GEOSWKTWriter_create();
        GEOSWKTWriter_setTrim(writer, 1);
        GEOSWKTWriter_setRoundingPrecision(writer, 3);
        char *wkt_geom = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry *>(geom));
        char *wkt_base_geom = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry *>(base_geom));
        GEOSWKTWriter_destroy(writer);
        cout << "Intersect: " << wkt_base_geom << " - " << wkt_geom << endl;
    }
}

int intersect(const char *filename, const char *filename2)
{
    initGEOS(geos_message_handler, geos_message_handler);

    GEOSSTRtree *tree = GEOSSTRtree_create(10); // initialize tree

    std::ifstream input(filename);
    for (std::string line; getline(input, line);)
    {
        if (line.length() > 5 && line.find("(")) // insert if the line is in wkt polygon format
        {
            GEOSGeometry *geom = GEOSGeomFromWKT(line.c_str());
            GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
        }
    }

    vector<GEOSGeometry *> geoms2;
    std::ifstream input2(filename2);
    for (std::string line2; getline(input2, line2);) // get polygons from file to compare and create vector
    {
        if (line2.length() > 5 && line2.find("("))
        {
            GEOSGeometry *geom2 = GEOSGeomFromWKT(line2.c_str());
            geoms2.push_back(geom2);
        }
    }

    GEOSGeometry *geoms2_cur;
    for (auto cur = geoms2.begin(); cur != geoms2.end(); ++cur)
    {
        geoms2_cur = *cur;
        GEOSSTRtree_query(tree, geoms2_cur, intersect_callback, geoms2_cur); // perform query for each polygon in the vector (callback function will be called if the polygon is in the result set of the query)
    }

    // deallocating objects
    for (auto cur = geoms2.begin(); cur != geoms2.end(); ++cur)
    {
        geoms2_cur = *cur;
        GEOSGeom_destroy(geoms2_cur);
    }

    GEOSSTRtree_destroy(tree);

    finishGEOS();

    return 0;
}

void split_file(const char *filename)
{
    string line;
    int line_count = 0;
    ifstream file(filename);
    while (getline(file, line))
        line_count++;
    int line_count_per_file = (line_count / numProcs) + 1;
    system((string("split -l ") + to_string(line_count_per_file) + " " + argv[1] + string(" -d --suffix-length=3 file_part_")).c_str());
}

int main(int argc, char **argv)
{
    int rank;
    int numProcs;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    MPI_Status status;

    int n = 1;
    if (argc > 3)
        n = atoi(argv[3]);
    if (!n)
        n = 1;

    char *filenameTemp;
    if (numProcs > 1)
    {
        if (rank == 0)
        {
            split_file(argv[1])
        }
        MPI_Barrier(MPI_COMM_WORLD);

        string filenameTempSuffix = rank < 10 ? "00" : rank < 100 ? "0"
                                                                  : "";
        filenameTemp = (char *)(string("file_part_") + filenameTempSuffix + to_string(rank)).c_str();
    }
    else
    {
        filenameTemp = argv[1];
    }

    const char *filename = filenameTemp;
    const char *filename2 = argv[2];
    cout << "File: " << filename << endl;

    if (rank == 0)
    {
        if (numProcs > 1)
        {
            system("rm ./file_part_*");
        }
    }

    MPI_Finalize();
    return 0;
}