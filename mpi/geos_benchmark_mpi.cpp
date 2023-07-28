// This program performs various geospatial operations using 
// the two given geospatial datasets and outputs the elapsed time. 
// This program uses the MPI library. The first file should be partitioned 
// and the directory name of the data should be given as the first parameter.
//
// USAGE
// -----
// mpic++ geos_benchmark.cpp
// mpirun -np <# of processes>  ./a.out <directorypath1> <filepath2> <# of repetition of operations>


#include <iostream>
#include <stdarg.h>
#include <stdlib.h>

#include <geos_c.h>
#include <cstring>
//#include <vector>
#include <sys/time.h>

#include <mpi.h>

#define MAX_LINE_LENGTH 30000
#define MAX_GEOM_NUMBER 500000

using namespace std;

static void
geos_message_handler(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

int create_tree(const char *filename, const char *filename2)
{
    initGEOS(geos_message_handler, geos_message_handler);

    GEOSSTRtree *tree = GEOSSTRtree_create(10);

    FILE *input;
    input = fopen(filename, "r");
    char line[MAX_LINE_LENGTH];
    while (fgets(line, MAX_LINE_LENGTH, input))
    {
        if (strlen(line) > 5 && string(line).find("("))
        {
            GEOSGeometry *geom = GEOSGeomFromWKT(line);
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
    // GEOSWKTWriter *writer = GEOSWKTWriter_create();
    // GEOSWKTWriter_setTrim(writer, 1);
    // GEOSWKTWriter_setRoundingPrecision(writer, 3);
    // char *wkt_geom = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry *>(geom));

    // GEOSWKTWriter_destroy(writer);
    // cout << wkt_geom << endl;
    element_count++;
}

int iterate_tree(const char *filename, const char *filename2)
{
    initGEOS(geos_message_handler, geos_message_handler);

    GEOSGeometry *geoms[MAX_GEOM_NUMBER];

    GEOSSTRtree *tree = GEOSSTRtree_create(10);

    FILE *input;
    input = fopen(filename, "r");
    char line[MAX_LINE_LENGTH];
    int index = 0;
    while (fgets(line, MAX_LINE_LENGTH, input))
    {
        if (strlen(line) > 5 && string(line).find("("))
        {
            GEOSGeometry *geom = GEOSGeomFromWKT(line);
            geoms[index] = geom;
            GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
            index++;
        }
    }

    GEOSSTRtree_iterate(tree, iterate_tree_callback, 0);
    cout << "# of Geometries: " << element_count << endl;

    GEOSSTRtree_destroy(tree);
    for (size_t i = 0; i < index; i++)
    {
        GEOSGeom_destroy(geoms[i]);
    }
    finishGEOS();

    return 0;
}

void query_callback(void *geom, void *base_geom)
{
    // GEOSWKTWriter *writer = GEOSWKTWriter_create();
    // GEOSWKTWriter_setTrim(writer, 1);
    // GEOSWKTWriter_setRoundingPrecision(writer, 3);
    // char *wkt_geom = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry *>(geom));
    // char *wkt_base_geom = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry *>(base_geom));
    // GEOSWKTWriter_destroy(writer);
    // cout <<"MBR Intersection: " << wkt_base_geom << " - " << wkt_geom << endl;
}

int query(const char *filename, const char *filename2)
{
    initGEOS(geos_message_handler, geos_message_handler);

    GEOSGeometry *geoms[MAX_GEOM_NUMBER];

    GEOSSTRtree *tree = GEOSSTRtree_create(10);

    FILE *input;
    input = fopen(filename, "r");
    char line[MAX_LINE_LENGTH];
    int index = 0;
    while (fgets(line, MAX_LINE_LENGTH, input))
    {
        if (strlen(line) > 5 && string(line).find("("))
        {
            GEOSGeometry *geom = GEOSGeomFromWKT(line);
            geoms[index] = geom;
            GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
            index++;
        }
    }

    GEOSGeometry *geoms2[MAX_GEOM_NUMBER];
    FILE *input2;
    input2 = fopen(filename2, "r");
    char line2[MAX_LINE_LENGTH];
    int index2 = 0;
    while (fgets(line2, MAX_LINE_LENGTH, input2))
    {
        if (strlen(line2) > 5)
        {
            GEOSGeometry *geom2 = GEOSGeomFromWKT(line2);
            geoms2[index2] = geom2;
            index2++;
        }
    }

    for (int i = 0; i < index2; i++)
    {
        GEOSSTRtree_query(tree, geoms2[i], query_callback, geoms2[i]);
    }

    for (size_t i = 0; i < index2; i++)
    {
        GEOSGeom_destroy(geoms2[i]);
    }

    // vector<GEOSGeometry *> geoms2;
    // FILE *input2;
    // input2 = fopen(filename2, "r");
    // char line2[MAX_LINE_LENGTH];
    // while (fgets(line2, MAX_LINE_LENGTH, input2))
    // {
    //     if (strlen(line2) > 5)
    //     {
    //         GEOSGeometry *geom2 = GEOSGeomFromWKT(line2);
    //         geoms2.push_back(geom2);
    //     }
    // }

    // GEOSGeometry *geoms2_cur;
    // for (auto cur = geoms2.begin(); cur != geoms2.end(); ++cur)
    // {
    //     geoms2_cur = *cur;
    //     GEOSSTRtree_query(tree, geoms2_cur, query_callback, geoms2_cur);
    // }

    // for (auto cur = geoms2.begin(); cur != geoms2.end(); ++cur)
    // {
    //     geoms2_cur = *cur;
    //     GEOSGeom_destroy(geoms2_cur);
    // }

    GEOSSTRtree_destroy(tree);
    for (size_t i = 0; i < index; i++)
    {
        GEOSGeom_destroy(geoms[i]);
    }
    finishGEOS();

    return 0;
}

void intersect_callback(void *geom, void *base_geom)
{
    if (!GEOSIntersects(static_cast<const GEOSGeometry *>(geom), static_cast<const GEOSGeometry *>(base_geom)))
    {
        // GEOSWKTWriter *writer = GEOSWKTWriter_create();
        // GEOSWKTWriter_setTrim(writer, 1);
        // GEOSWKTWriter_setRoundingPrecision(writer, 3);
        // char *wkt_geom = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry *>(geom));
        // char *wkt_base_geom = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry *>(base_geom));
        // GEOSWKTWriter_destroy(writer);
        // cout <<"Intersect: " << wkt_base_geom << " - " << wkt_geom << endl;
    }
}

int intersect(const char *filename, const char *filename2)
{
    initGEOS(geos_message_handler, geos_message_handler);

    GEOSGeometry *geoms[MAX_GEOM_NUMBER];

    GEOSSTRtree *tree = GEOSSTRtree_create(10);

    FILE *input;
    input = fopen(filename, "r");
    char line[MAX_LINE_LENGTH];
    int index = 0;
    while (fgets(line, MAX_LINE_LENGTH, input))
    {
        if (strlen(line) > 5 && string(line).find("("))
        {
            GEOSGeometry *geom = GEOSGeomFromWKT(line);
            geoms[index] = geom;
            GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
            index++;
        }
    }

    GEOSGeometry *geoms2[MAX_GEOM_NUMBER];
    FILE *input2;
    input2 = fopen(filename2, "r");
    char line2[MAX_LINE_LENGTH];
    int index2 = 0;
    while (fgets(line2, MAX_LINE_LENGTH, input2))
    {
        if (strlen(line2) > 5)
        {
            GEOSGeometry *geom2 = GEOSGeomFromWKT(line2);
            geoms2[index2] = geom2;
            index2++;
        }
    }

    for (int i = 0; i < index2; i++)
    {
        GEOSSTRtree_query(tree, geoms2[i], intersect_callback, geoms2[i]);
    }

    for (size_t i = 0; i < index2; i++)
    {
        GEOSGeom_destroy(geoms2[i]);
    }

    GEOSSTRtree_destroy(tree);
    for (size_t i = 0; i < index; i++)
    {
        GEOSGeom_destroy(geoms[i]);
    }
    finishGEOS();

    return 0;
}

void overlap_callback(void *geom, void *base_geom)
{
    if (!GEOSOverlaps(static_cast<const GEOSGeometry *>(geom), static_cast<const GEOSGeometry *>(base_geom)))
    {
        // GEOSWKTWriter *writer = GEOSWKTWriter_create();
        // GEOSWKTWriter_setTrim(writer, 1);
        // GEOSWKTWriter_setRoundingPrecision(writer, 3);
        // char *wkt_geom = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry *>(geom));
        // char *wkt_base_geom = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry *>(base_geom));
        // GEOSWKTWriter_destroy(writer);
        // cout <<"Overlap: " << wkt_base_geom << " - " << wkt_geom << endl;
    }
}

int overlap(const char *filename, const char *filename2)
{
    initGEOS(geos_message_handler, geos_message_handler);

    GEOSGeometry *geoms[MAX_GEOM_NUMBER];

    GEOSSTRtree *tree = GEOSSTRtree_create(10);

    FILE *input;
    input = fopen(filename, "r");
    char line[MAX_LINE_LENGTH];
    int index = 0;
    while (fgets(line, MAX_LINE_LENGTH, input))
    {
        if (strlen(line) > 5 && string(line).find("("))
        {
            GEOSGeometry *geom = GEOSGeomFromWKT(line);
            geoms[index] = geom;
            GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
            index++;
        }
    }

    GEOSGeometry *geoms2[MAX_GEOM_NUMBER];
    FILE *input2;
    input2 = fopen(filename2, "r");
    char line2[MAX_LINE_LENGTH];
    int index2 = 0;
    while (fgets(line2, MAX_LINE_LENGTH, input2))
    {
        if (strlen(line2) > 5)
        {
            GEOSGeometry *geom2 = GEOSGeomFromWKT(line2);
            geoms2[index2] = geom2;
            index2++;
        }
    }

    for (int i = 0; i < index2; i++)
    {
        GEOSSTRtree_query(tree, geoms2[i], overlap_callback, geoms2[i]);
    }

    for (size_t i = 0; i < index2; i++)
    {
        GEOSGeom_destroy(geoms2[i]);
    }

    GEOSSTRtree_destroy(tree);
    for (size_t i = 0; i < index; i++)
    {
        GEOSGeom_destroy(geoms[i]);
    }
    finishGEOS();

    return 0;
}

void touch_callback(void *geom, void *base_geom)
{
    if (!GEOSTouches(static_cast<const GEOSGeometry *>(geom), static_cast<const GEOSGeometry *>(base_geom)))
    {
        // GEOSWKTWriter *writer = GEOSWKTWriter_create();
        // GEOSWKTWriter_setTrim(writer, 1);
        // GEOSWKTWriter_setRoundingPrecision(writer, 3);
        // char *wkt_geom = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry *>(geom));
        // char *wkt_base_geom = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry *>(base_geom));
        // GEOSWKTWriter_destroy(writer);
        // cout <<"Touch: " << wkt_base_geom << " - " << wkt_geom << endl;
    }
}

int touch(const char *filename, const char *filename2)
{
    initGEOS(geos_message_handler, geos_message_handler);

    GEOSGeometry *geoms[MAX_GEOM_NUMBER];

    GEOSSTRtree *tree = GEOSSTRtree_create(10);

    FILE *input;
    input = fopen(filename, "r");
    char line[MAX_LINE_LENGTH];
    int index = 0;
    while (fgets(line, MAX_LINE_LENGTH, input))
    {
        if (strlen(line) > 5 && string(line).find("("))
        {
            GEOSGeometry *geom = GEOSGeomFromWKT(line);
            geoms[index] = geom;
            GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
            index++;
        }
    }

    GEOSGeometry *geoms2[MAX_GEOM_NUMBER];
    FILE *input2;
    input2 = fopen(filename2, "r");
    char line2[MAX_LINE_LENGTH];
    int index2 = 0;
    while (fgets(line2, MAX_LINE_LENGTH, input2))
    {
        if (strlen(line2) > 5)
        {
            GEOSGeometry *geom2 = GEOSGeomFromWKT(line2);
            geoms2[index2] = geom2;
            index2++;
        }
    }

    for (int i = 0; i < index2; i++)
    {
        GEOSSTRtree_query(tree, geoms2[i], touch_callback, geoms2[i]);
    }

    for (size_t i = 0; i < index2; i++)
    {
        GEOSGeom_destroy(geoms2[i]);
    }

    GEOSSTRtree_destroy(tree);
    for (size_t i = 0; i < index; i++)
    {
        GEOSGeom_destroy(geoms[i]);
    }
    finishGEOS();

    return 0;
}

void cross_callback(void *geom, void *base_geom)
{
    if (!GEOSCrosses(static_cast<const GEOSGeometry *>(geom), static_cast<const GEOSGeometry *>(base_geom)))
    {
        // GEOSWKTWriter *writer = GEOSWKTWriter_create();
        // GEOSWKTWriter_setTrim(writer, 1);
        // GEOSWKTWriter_setRoundingPrecision(writer, 3);
        // char *wkt_geom = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry *>(geom));
        // char *wkt_base_geom = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry *>(base_geom));
        // GEOSWKTWriter_destroy(writer);
        // cout <<"Cross: " << wkt_base_geom << " - " << wkt_geom << endl;
    }
}

int cross(const char *filename, const char *filename2)
{
    initGEOS(geos_message_handler, geos_message_handler);

    GEOSGeometry *geoms[MAX_GEOM_NUMBER];

    GEOSSTRtree *tree = GEOSSTRtree_create(10);

    FILE *input;
    input = fopen(filename, "r");
    char line[MAX_LINE_LENGTH];
    int index = 0;
    while (fgets(line, MAX_LINE_LENGTH, input))
    {
        if (strlen(line) > 5 && string(line).find("("))
        {
            GEOSGeometry *geom = GEOSGeomFromWKT(line);
            geoms[index] = geom;
            GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
            index++;
        }
    }

    GEOSGeometry *geoms2[MAX_GEOM_NUMBER];
    FILE *input2;
    input2 = fopen(filename2, "r");
    char line2[MAX_LINE_LENGTH];
    int index2 = 0;
    while (fgets(line2, MAX_LINE_LENGTH, input2))
    {
        if (strlen(line2) > 5)
        {
            GEOSGeometry *geom2 = GEOSGeomFromWKT(line2);
            geoms2[index2] = geom2;
            index2++;
        }
    }

    for (int i = 0; i < index2; i++)
    {
        GEOSSTRtree_query(tree, geoms2[i], cross_callback, geoms2[i]);
    }

    for (size_t i = 0; i < index2; i++)
    {
        GEOSGeom_destroy(geoms2[i]);
    }

    GEOSSTRtree_destroy(tree);
    for (size_t i = 0; i < index; i++)
    {
        GEOSGeom_destroy(geoms[i]);
    }
    finishGEOS();

    return 0;
}

void contain_callback(void *geom, void *base_geom)
{
    if (!GEOSContains(static_cast<const GEOSGeometry *>(geom), static_cast<const GEOSGeometry *>(base_geom)))
    {
        // GEOSWKTWriter *writer = GEOSWKTWriter_create();
        // GEOSWKTWriter_setTrim(writer, 1);
        // GEOSWKTWriter_setRoundingPrecision(writer, 3);
        // char *wkt_geom = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry *>(geom));
        // char *wkt_base_geom = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry *>(base_geom));
        // GEOSWKTWriter_destroy(writer);
        // cout <<"Contain: " << wkt_base_geom << " - " << wkt_geom << endl;
    }
}

int contain(const char *filename, const char *filename2)
{
    initGEOS(geos_message_handler, geos_message_handler);

    GEOSGeometry *geoms[MAX_GEOM_NUMBER];

    GEOSSTRtree *tree = GEOSSTRtree_create(10);

    FILE *input;
    input = fopen(filename, "r");
    char line[MAX_LINE_LENGTH];
    int index = 0;
    while (fgets(line, MAX_LINE_LENGTH, input))
    {
        if (strlen(line) > 5 && string(line).find("("))
        {
            GEOSGeometry *geom = GEOSGeomFromWKT(line);
            geoms[index] = geom;
            GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
            index++;
        }
    }

    GEOSGeometry *geoms2[MAX_GEOM_NUMBER];
    FILE *input2;
    input2 = fopen(filename2, "r");
    char line2[MAX_LINE_LENGTH];
    int index2 = 0;
    while (fgets(line2, MAX_LINE_LENGTH, input2))
    {
        if (strlen(line2) > 5)
        {
            GEOSGeometry *geom2 = GEOSGeomFromWKT(line2);
            geoms2[index2] = geom2;
            index2++;
        }
    }

    for (int i = 0; i < index2; i++)
    {
        GEOSSTRtree_query(tree, geoms2[i], contain_callback, geoms2[i]);
    }

    for (size_t i = 0; i < index2; i++)
    {
        GEOSGeom_destroy(geoms2[i]);
    }

    GEOSSTRtree_destroy(tree);
    for (size_t i = 0; i < index; i++)
    {
        GEOSGeom_destroy(geoms[i]);
    }
    finishGEOS();

    return 0;
}

void equal_callback(void *geom, void *base_geom)
{
    if (!GEOSEquals(static_cast<const GEOSGeometry *>(geom), static_cast<const GEOSGeometry *>(base_geom)))
    {
        // GEOSWKTWriter *writer = GEOSWKTWriter_create();
        // GEOSWKTWriter_setTrim(writer, 1);
        // GEOSWKTWriter_setRoundingPrecision(writer, 3);
        // char *wkt_geom = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry *>(geom));
        // char *wkt_base_geom = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry *>(base_geom));
        // GEOSWKTWriter_destroy(writer);
        // cout <<"Equal: " << wkt_base_geom << " - " << wkt_geom << endl;
    }
}

int equal(const char *filename, const char *filename2)
{
    initGEOS(geos_message_handler, geos_message_handler);

    GEOSGeometry *geoms[MAX_GEOM_NUMBER];

    GEOSSTRtree *tree = GEOSSTRtree_create(10);

    FILE *input;
    input = fopen(filename, "r");
    char line[MAX_LINE_LENGTH];
    int index = 0;
    while (fgets(line, MAX_LINE_LENGTH, input))
    {
        if (strlen(line) > 5 && string(line).find("("))
        {
            GEOSGeometry *geom = GEOSGeomFromWKT(line);
            geoms[index] = geom;
            GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
            index++;
        }
    }

    GEOSGeometry *geoms2[MAX_GEOM_NUMBER];
    FILE *input2;
    input2 = fopen(filename2, "r");
    char line2[MAX_LINE_LENGTH];
    int index2 = 0;
    while (fgets(line2, MAX_LINE_LENGTH, input2))
    {
        if (strlen(line2) > 5)
        {
            GEOSGeometry *geom2 = GEOSGeomFromWKT(line2);
            geoms2[index2] = geom2;
            index2++;
        }
    }

    for (int i = 0; i < index2; i++)
    {
        GEOSSTRtree_query(tree, geoms2[i], equal_callback, geoms2[i]);
    }

    for (size_t i = 0; i < index2; i++)
    {
        GEOSGeom_destroy(geoms2[i]);
    }

    GEOSSTRtree_destroy(tree);
    for (size_t i = 0; i < index; i++)
    {
        GEOSGeom_destroy(geoms[i]);
    }
    finishGEOS();

    return 0;
}

void equal_exact_callback(void *geom, void *base_geom)
{
    if (!GEOSEqualsExact(static_cast<const GEOSGeometry *>(geom), static_cast<const GEOSGeometry *>(base_geom), 0.3))
    {
        // GEOSWKTWriter *writer = GEOSWKTWriter_create();
        // GEOSWKTWriter_setTrim(writer, 1);
        // GEOSWKTWriter_setRoundingPrecision(writer, 3);
        // char *wkt_geom = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry *>(geom));
        // char *wkt_base_geom = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry *>(base_geom));
        // GEOSWKTWriter_destroy(writer);
        // cout <<"Equal Exact (0.3): " << wkt_base_geom << " - " << wkt_geom << endl;
    }
}

int equal_exact(const char *filename, const char *filename2)
{
    initGEOS(geos_message_handler, geos_message_handler);

    GEOSGeometry *geoms[MAX_GEOM_NUMBER];

    GEOSSTRtree *tree = GEOSSTRtree_create(10);

    FILE *input;
    input = fopen(filename, "r");
    char line[MAX_LINE_LENGTH];
    int index = 0;
    while (fgets(line, MAX_LINE_LENGTH, input))
    {
        if (strlen(line) > 5 && string(line).find("("))
        {
            GEOSGeometry *geom = GEOSGeomFromWKT(line);
            geoms[index] = geom;
            GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
            index++;
        }
    }

    GEOSGeometry *geoms2[MAX_GEOM_NUMBER];
    FILE *input2;
    input2 = fopen(filename2, "r");
    char line2[MAX_LINE_LENGTH];
    int index2 = 0;
    while (fgets(line2, MAX_LINE_LENGTH, input2))
    {
        if (strlen(line2) > 5)
        {
            GEOSGeometry *geom2 = GEOSGeomFromWKT(line2);
            geoms2[index2] = geom2;
            index2++;
        }
    }

    for (int i = 0; i < index2; i++)
    {
        GEOSSTRtree_query(tree, geoms2[i], equal_exact_callback, geoms2[i]);
    }

    for (size_t i = 0; i < index2; i++)
    {
        GEOSGeom_destroy(geoms2[i]);
    }

    GEOSSTRtree_destroy(tree);
    for (size_t i = 0; i < index; i++)
    {
        GEOSGeom_destroy(geoms[i]);
    }
    finishGEOS();

    return 0;
}

void cover_callback(void *geom, void *base_geom)
{
    if (!GEOSCovers(static_cast<const GEOSGeometry *>(geom), static_cast<const GEOSGeometry *>(base_geom)))
    {
        // GEOSWKTWriter *writer = GEOSWKTWriter_create();
        // GEOSWKTWriter_setTrim(writer, 1);
        // GEOSWKTWriter_setRoundingPrecision(writer, 3);
        // char *wkt_geom = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry *>(geom));
        // char *wkt_base_geom = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry *>(base_geom));
        // GEOSWKTWriter_destroy(writer);
        // cout <<"Cover: " << wkt_base_geom << " - " << wkt_geom << endl;
    }
}

int cover(const char *filename, const char *filename2)
{
    initGEOS(geos_message_handler, geos_message_handler);

    GEOSGeometry *geoms[MAX_GEOM_NUMBER];

    GEOSSTRtree *tree = GEOSSTRtree_create(10);

    FILE *input;
    input = fopen(filename, "r");
    char line[MAX_LINE_LENGTH];
    int index = 0;
    while (fgets(line, MAX_LINE_LENGTH, input))
    {
        if (strlen(line) > 5 && string(line).find("("))
        {
            GEOSGeometry *geom = GEOSGeomFromWKT(line);
            geoms[index] = geom;
            GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
            index++;
        }
    }

    GEOSGeometry *geoms2[MAX_GEOM_NUMBER];
    FILE *input2;
    input2 = fopen(filename2, "r");
    char line2[MAX_LINE_LENGTH];
    int index2 = 0;
    while (fgets(line2, MAX_LINE_LENGTH, input2))
    {
        if (strlen(line2) > 5)
        {
            GEOSGeometry *geom2 = GEOSGeomFromWKT(line2);
            geoms2[index2] = geom2;
            index2++;
        }
    }

    for (int i = 0; i < index2; i++)
    {
        GEOSSTRtree_query(tree, geoms2[i], cover_callback, geoms2[i]);
    }

    for (size_t i = 0; i < index2; i++)
    {
        GEOSGeom_destroy(geoms2[i]);
    }

    GEOSSTRtree_destroy(tree);
    for (size_t i = 0; i < index; i++)
    {
        GEOSGeom_destroy(geoms[i]);
    }
    finishGEOS();

    return 0;
}

void covered_by_callback(void *geom, void *base_geom)
{
    if (!GEOSCoveredBy(static_cast<const GEOSGeometry *>(geom), static_cast<const GEOSGeometry *>(base_geom)))
    {
        // GEOSWKTWriter *writer = GEOSWKTWriter_create();
        // GEOSWKTWriter_setTrim(writer, 1);
        // GEOSWKTWriter_setRoundingPrecision(writer, 3);
        // char *wkt_geom = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry *>(geom));
        // char *wkt_base_geom = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry *>(base_geom));
        // GEOSWKTWriter_destroy(writer);
        // cout << "Covered By: " << wkt_base_geom << " - " << wkt_geom << endl;
    }
}

int covered_by(const char *filename, const char *filename2)
{
    initGEOS(geos_message_handler, geos_message_handler);

    GEOSGeometry *geoms[MAX_GEOM_NUMBER];

    GEOSSTRtree *tree = GEOSSTRtree_create(10);

    FILE *input;
    input = fopen(filename, "r");
    char line[MAX_LINE_LENGTH];
    int index = 0;
    while (fgets(line, MAX_LINE_LENGTH, input))
    {
        if (strlen(line) > 5 && string(line).find("("))
        {
            GEOSGeometry *geom = GEOSGeomFromWKT(line);
            geoms[index] = geom;
            GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
            index++;
        }
    }

    GEOSGeometry *geoms2[MAX_GEOM_NUMBER];
    FILE *input2;
    input2 = fopen(filename2, "r");
    char line2[MAX_LINE_LENGTH];
    int index2 = 0;
    while (fgets(line2, MAX_LINE_LENGTH, input2))
    {
        if (strlen(line2) > 5)
        {
            GEOSGeometry *geom2 = GEOSGeomFromWKT(line2);
            geoms2[index2] = geom2;
            index2++;
        }
    }

    for (int i = 0; i < index2; i++)
    {
        GEOSSTRtree_query(tree, geoms2[i], covered_by_callback, geoms2[i]);
    }

    for (size_t i = 0; i < index2; i++)
    {
        GEOSGeom_destroy(geoms2[i]);
    }

    GEOSSTRtree_destroy(tree);
    for (size_t i = 0; i < index; i++)
    {
        GEOSGeom_destroy(geoms[i]);
    }
    finishGEOS();

    return 0;
}

double select_test(const char *name, int (*test_function)(const char *, const char *), const char *filename, const char *filename2, int n)
{
    double time_arr[n];
    for (int i = 0; i < n; i++)
    {
        // clock_t t;
        // t = clock();

        struct timeval tv1, tv2;
        gettimeofday(&tv1, NULL);

        test_function(filename, filename2);

        gettimeofday(&tv2, NULL);
        double time_spent = (double)(tv2.tv_usec - tv1.tv_usec) / 1000000 + (double)(tv2.tv_sec - tv1.tv_sec);
        time_arr[i] = time_spent;

        // t = clock() - t;
        // double time_taken = ((double)t) / CLOCKS_PER_SEC;
        // time_arr[i] = time_taken;
    }

    cout << "------------------- " << name << " -------------------" << endl;
    for (int i = 0; i < n; i++)
    {
        cout << "The program " << i << " took " << time_arr[i] << " seconds to execute" << endl;
    }
    cout << "---------------------------------------------" << endl
         << endl;

    if (n == 1)
    {
        return time_arr[0];
    }
    else
    {
        double avg_time = 0;
        for (int i = 1; i < n; i++)
        {
            avg_time += time_arr[i];
        }
        avg_time /= (n - 1);
        return avg_time;
    }
}

// int main(int argc, char **argv)
// {
//     int rank;
//     int numProcs;

//     MPI_Init(&argc, &argv);

//     MPI_Comm_rank(MPI_COMM_WORLD, &rank); // rank is process id

//     MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

//     // send side
//     int sendCode = 123456;
//     int destination = 0;
//     int tag = 100;

//     MPI_Status status;

//     if (rank == 0)
//     {

//         double test_time_arr_sum[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
//         for (size_t i = 1; i < numProcs; i++)
//         {
//             double test_time_arr_of_process[12];
//             MPI_Recv(&test_time_arr_of_process, 12, MPI_DOUBLE, i, tag, MPI_COMM_WORLD, &status);
//             printf("Received %d \n", i);
//             for (size_t j = 0; j < 12; j++)
//             {
//                 test_time_arr_sum[j] += test_time_arr_of_process[j];
//             }
//         }

//         cout << endl
//              << endl;
//         cout << "------------------------------------------------------------------" << endl;
//         cout << "-------------------- BENCHMARK RESULT (TOTAL) --------------------" << endl;
//         cout << "------------------------------------------------------------------" << endl;
//         cout << "Total Create Time: " << test_time_arr_sum[0] << endl;
//         cout << "Total Iterate Time: " << test_time_arr_sum[1] << endl;
//         cout << "Total Query Time: " << test_time_arr_sum[2] << endl;
//         cout << "Total Intersect Time: " << test_time_arr_sum[3] << endl;
//         cout << "Total Overlap Time: " << test_time_arr_sum[4] << endl;
//         cout << "Total Touch Time: " << test_time_arr_sum[5] << endl;
//         cout << "Total Cross Time: " << test_time_arr_sum[6] << endl;
//         cout << "Total Contain Time: " << test_time_arr_sum[7] << endl;
//         cout << "Total Equal Time: " << test_time_arr_sum[8] << endl;
//         cout << "Total Equal Exact (0.3) Time: " << test_time_arr_sum[9] << endl;
//         cout << "Total Cover Time: " << test_time_arr_sum[10] << endl;
//         cout << "Total Covered By Time: " << test_time_arr_sum[11] << endl;
//         cout << "------------------------------------------------------------------" << endl;
//         cout << "------------------------------------------------------------------" << endl;
//         cout << endl
//              << endl;
//     }
//     else
//     {
//         int n = 1;
//         if (argc > 3)
//             n = atoi(argv[3]);
//         if (!n)
//             n = 1;
//         // const char *filename = (string(argv[1]) + "/x" + to_string(rank)).c_str();
//         const char *filename = argv[1];
//         const char *filename2 = argv[2];

//         double create_time = select_test("Create", &create_tree, filename, filename2, n);
//         double iterate_time = select_test("Iterate", &iterate_tree, filename, filename2, n);
//         double query_time = select_test("Query", &query, filename, filename2, n);
//         double intersect_time = select_test("Intersect", &intersect, filename, filename2, n);
//         double overlap_time = select_test("Overlap", &overlap, filename, filename2, n);
//         double touch_time = select_test("Touch", &touch, filename, filename2, n);
//         double cross_time = select_test("Cross", &cross, filename, filename2, n);
//         double contain_time = select_test("Contain", &contain, filename, filename2, n);
//         double equal_time = select_test("Equal", &equal, filename, filename2, n);
//         double equal_exact_time = select_test("Equal Exact (0.3)", &equal_exact, filename, filename2, n);
//         double cover_time = select_test("Cover", &cover, filename, filename2, n);
//         double covered_by_time = select_test("Covered By", &covered_by, filename, filename2, n);

//         cout << endl
//              << endl;
//         cout << "------------------------------------------------------------------" << endl;
//         cout << "--------------- BENCHMARK RESULT (Process rank:" << rank << ") ---------------" << endl;
//         cout << "------------------------------------------------------------------" << endl;
//         cout << "Average Create Time: " << create_time << endl;
//         cout << "Average Iterate Time: " << iterate_time << endl;
//         cout << "Average Query Time: " << query_time << endl;
//         cout << "Average Intersect Time: " << intersect_time << endl;
//         cout << "Average Overlap Time: " << overlap_time << endl;
//         cout << "Average Touch Time: " << touch_time << endl;
//         cout << "Average Cross Time: " << cross_time << endl;
//         cout << "Average Contain Time: " << contain_time << endl;
//         cout << "Average Equal Time: " << equal_time << endl;
//         cout << "Average Equal Exact (0.3) Time: " << equal_exact_time << endl;
//         cout << "Average Cover Time: " << cover_time << endl;
//         cout << "Average Covered By Time: " << covered_by_time << endl;
//         cout << "------------------------------------------------------------------" << endl;
//         cout << "------------------------------------------------------------------" << endl;
//         cout << endl
//              << endl;

//         double test_time_arr[12] = {create_time, iterate_time, query_time, intersect_time, overlap_time, touch_time,
//                                     cross_time, contain_time, equal_time, equal_exact_time, cover_time, covered_by_time};

//         MPI_Send(test_time_arr, 12, MPI_DOUBLE, destination, tag, MPI_COMM_WORLD);
//     }

//     MPI_Finalize();
//     return 0;
// }

int main(int argc, char **argv)
{
    int rank;
    int numProcs;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // rank is process id

    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    // send side
    int sendCode = 123456;
    int destination = 0;
    int tag = 100;

    MPI_Status status;

    int n = 1;
    if (argc > 3)
        n = atoi(argv[3]);
    if (!n)
        n = 1;
    // const char *filename = (string(argv[1]) + "/x" + to_string(rank)).c_str();
    // const char *filename = argv[1];
    char filenameTemp[200];
    strcpy(filenameTemp, argv[1]);
    strcat(filenameTemp, to_string(rank).c_str());
    const char *filename = filenameTemp;
    cout << "File: " << filename << endl;
    const char *filename2 = argv[2];

    double create_time = select_test("Create", &create_tree, filename, filename2, n);
    double iterate_time = select_test("Iterate", &iterate_tree, filename, filename2, n);
    double query_time = select_test("Query", &query, filename, filename2, n);
    double intersect_time = select_test("Intersect", &intersect, filename, filename2, n);
    double overlap_time = select_test("Overlap", &overlap, filename, filename2, n);
    double touch_time = select_test("Touch", &touch, filename, filename2, n);
    double cross_time = select_test("Cross", &cross, filename, filename2, n);
    double contain_time = select_test("Contain", &contain, filename, filename2, n);
    double equal_time = select_test("Equal", &equal, filename, filename2, n);
    double equal_exact_time = select_test("Equal Exact (0.3)", &equal_exact, filename, filename2, n);
    double cover_time = select_test("Cover", &cover, filename, filename2, n);
    double covered_by_time = select_test("Covered By", &covered_by, filename, filename2, n);

    cout << endl
         << endl
         << "------------------------------------------------------------------" << endl
         << "--------------- BENCHMARK RESULT (Process rank:" << rank << ") ---------------" << endl
         << "------------------------------------------------------------------" << endl
         << "Average Create Time: " << create_time << endl
         << "Average Iterate Time: " << iterate_time << endl
         << "Average Query Time: " << query_time << endl
         << "Average Intersect Time: " << intersect_time << endl
         << "Average Overlap Time: " << overlap_time << endl
         << "Average Touch Time: " << touch_time << endl
         << "Average Cross Time: " << cross_time << endl
         << "Average Contain Time: " << contain_time << endl
         << "Average Equal Time: " << equal_time << endl
         << "Average Equal Exact (0.3) Time: " << equal_exact_time << endl
         << "Average Cover Time: " << cover_time << endl
         << "Average Covered By Time: " << covered_by_time << endl
         << "------------------------------------------------------------------" << endl
         << "------------------------------------------------------------------" << endl
         << endl
         << endl;

    double test_time_arr_max[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    double test_time_arr_sum[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    double test_time_arr[12] = {create_time, iterate_time, query_time, intersect_time, overlap_time, touch_time,
                                cross_time, contain_time, equal_time, equal_exact_time, cover_time, covered_by_time};

    MPI_Reduce(test_time_arr, test_time_arr_max, 12, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(test_time_arr, test_time_arr_sum, 12, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        cout << endl
             << endl
             << "------------------------------------------------------------------" << endl
             << "-------------------- BENCHMARK RESULT (MAX) --------------------" << endl
             << "------------------------------------------------------------------" << endl
             << "Max Create Time: " << test_time_arr_max[0] << endl
             << "Max Iterate Time: " << test_time_arr_max[1] << endl
             << "Max Query Time: " << test_time_arr_max[2] << endl
             << "Max Intersect Time: " << test_time_arr_max[3] << endl
             << "Max Overlap Time: " << test_time_arr_max[4] << endl
             << "Max Touch Time: " << test_time_arr_max[5] << endl
             << "Max Cross Time: " << test_time_arr_max[6] << endl
             << "Max Contain Time: " << test_time_arr_max[7] << endl
             << "Max Equal Time: " << test_time_arr_max[8] << endl
             << "Max Equal Exact (0.3) Time: " << test_time_arr_max[9] << endl
             << "Max Cover Time: " << test_time_arr_max[10] << endl
             << "Max Covered By Time: " << test_time_arr_max[11] << endl
             << "------------------------------------------------------------------" << endl
             << "------------------------------------------------------------------" << endl
             << endl
             << endl
             << endl
             << endl
             << "------------------------------------------------------------------" << endl
             << "-------------------- BENCHMARK RESULT (TOTAL) --------------------" << endl
             << "------------------------------------------------------------------" << endl
             << "Total Create Time: " << test_time_arr_sum[0] << endl
             << "Total Iterate Time: " << test_time_arr_sum[1] << endl
             << "Total Query Time: " << test_time_arr_sum[2] << endl
             << "Total Intersect Time: " << test_time_arr_sum[3] << endl
             << "Total Overlap Time: " << test_time_arr_sum[4] << endl
             << "Total Touch Time: " << test_time_arr_sum[5] << endl
             << "Total Cross Time: " << test_time_arr_sum[6] << endl
             << "Total Contain Time: " << test_time_arr_sum[7] << endl
             << "Total Equal Time: " << test_time_arr_sum[8] << endl
             << "Total Equal Exact (0.3) Time: " << test_time_arr_sum[9] << endl
             << "Total Cover Time: " << test_time_arr_sum[10] << endl
             << "Total Covered By Time: " << test_time_arr_sum[11] << endl
             << "------------------------------------------------------------------" << endl
             << "------------------------------------------------------------------" << endl
             << endl
             << endl;
    }

    MPI_Finalize();
    return 0;
}