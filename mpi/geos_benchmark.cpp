// This program performs various geospatial operations using 
// the two given geospatial datasets and outputs the elapsed time. 
// This program does not include the MPI library, it runs as a single process.
//
// USAGE
// -----
// g++ geos_benchmark.cpp
// ./a.out <filepath1> <filepath2> <# of repetition of operations>
// ./geos_benchmark ../Data/no_partition/cemetery.wkt ../Data/no_partition/sports.wkt 1 


#include <iostream>
#include <stdarg.h>
#include <stdlib.h>

#include <geos_c.h>
#include <cstring>
#include <vector>
#include <sys/time.h>

#define MAX_LINE_LENGTH 300000 // 30000 wasn't enough, nor was 100000
#define MAX_GEOM_NUMBER 500000 // 1800000 // I believe the Sports file has 1,767,137 lines, Cemetery has 193,076, and Lakes has 8,419,324.

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
    // printf("Entering initGEOS\n");
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
        if (index >= MAX_GEOM_NUMBER) {
            cout << "MAX SIZE OF " << MAX_GEOM_NUMBER << " EXCEEDED! TRUNCATING.";
            break;
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

    // GEOSGeometry *geoms2[MAX_GEOM_NUMBER];
    // FILE *input2;
    // input2 = fopen(filename2, "r");
    // char line2[MAX_LINE_LENGTH];
    // int index2 = 0;
    // while (fgets(line2, MAX_LINE_LENGTH, input2))
    // {
    //     if (strlen(line2) > 5)
    //     {
    //         GEOSGeometry *geom2 = GEOSGeomFromWKT(line2);
    //         geoms2[index2] = geom2;
    //         index2++;
    //     }
    // }

    // for (int i = 0; i < index2; i++)
    // {
    //     GEOSSTRtree_query(tree, geoms2[i], query_callback, geoms2[i]);
    // }

    // for (size_t i = 0; i < index2; i++)
    // {
    //     GEOSGeom_destroy(geoms2[i]);
    // }

    vector<GEOSGeometry *> geoms2;
    FILE *input2;
    input2 = fopen(filename2, "r");
    char line2[MAX_LINE_LENGTH];
    while (fgets(line2, MAX_LINE_LENGTH, input2))
    {
        if (strlen(line2) > 5)
        {
            GEOSGeometry *geom2 = GEOSGeomFromWKT(line2);
            geoms2.push_back(geom2);
        }
    }

    GEOSGeometry *geoms2_cur;
    for (auto cur = geoms2.begin(); cur != geoms2.end(); ++cur)
    {
        geoms2_cur = *cur;
        GEOSSTRtree_query(tree, geoms2_cur, query_callback, geoms2_cur);
    }

    for (auto cur = geoms2.begin(); cur != geoms2.end(); ++cur)
    {
        geoms2_cur = *cur;
        GEOSGeom_destroy(geoms2_cur);
    }

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

    vector<GEOSGeometry *> geoms2;
    FILE *input2;
    input2 = fopen(filename2, "r");
    char line2[MAX_LINE_LENGTH];
    while (fgets(line2, MAX_LINE_LENGTH, input2))
    {
        if (strlen(line2) > 5)
        {
            GEOSGeometry *geom2 = GEOSGeomFromWKT(line2);
            geoms2.push_back(geom2);
        }
    }

    GEOSGeometry *geoms2_cur;
    for (auto cur = geoms2.begin(); cur != geoms2.end(); ++cur)
    {
        geoms2_cur = *cur;
        GEOSSTRtree_query(tree, geoms2_cur, intersect_callback, geoms2_cur);
    }

    for (auto cur = geoms2.begin(); cur != geoms2.end(); ++cur)
    {
        geoms2_cur = *cur;
        GEOSGeom_destroy(geoms2_cur);
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

    vector<GEOSGeometry *> geoms2;
    FILE *input2;
    input2 = fopen(filename2, "r");
    char line2[MAX_LINE_LENGTH];
    while (fgets(line2, MAX_LINE_LENGTH, input2))
    {
        if (strlen(line2) > 5)
        {
            GEOSGeometry *geom2 = GEOSGeomFromWKT(line2);
            geoms2.push_back(geom2);
        }
    }

    GEOSGeometry *geoms2_cur;
    for (auto cur = geoms2.begin(); cur != geoms2.end(); ++cur)
    {
        geoms2_cur = *cur;
        GEOSSTRtree_query(tree, geoms2_cur, overlap_callback, geoms2_cur);
    }

    for (auto cur = geoms2.begin(); cur != geoms2.end(); ++cur)
    {
        geoms2_cur = *cur;
        GEOSGeom_destroy(geoms2_cur);
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

    vector<GEOSGeometry *> geoms2;
    FILE *input2;
    input2 = fopen(filename2, "r");
    char line2[MAX_LINE_LENGTH];
    while (fgets(line2, MAX_LINE_LENGTH, input2))
    {
        if (strlen(line2) > 5)
        {
            GEOSGeometry *geom2 = GEOSGeomFromWKT(line2);
            geoms2.push_back(geom2);
        }
    }

    GEOSGeometry *geoms2_cur;
    for (auto cur = geoms2.begin(); cur != geoms2.end(); ++cur)
    {
        geoms2_cur = *cur;
        GEOSSTRtree_query(tree, geoms2_cur, touch_callback, geoms2_cur);
    }

    for (auto cur = geoms2.begin(); cur != geoms2.end(); ++cur)
    {
        geoms2_cur = *cur;
        GEOSGeom_destroy(geoms2_cur);
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

    vector<GEOSGeometry *> geoms2;
    FILE *input2;
    input2 = fopen(filename2, "r");
    char line2[MAX_LINE_LENGTH];
    while (fgets(line2, MAX_LINE_LENGTH, input2))
    {
        if (strlen(line2) > 5)
        {
            GEOSGeometry *geom2 = GEOSGeomFromWKT(line2);
            geoms2.push_back(geom2);
        }
    }

    GEOSGeometry *geoms2_cur;
    for (auto cur = geoms2.begin(); cur != geoms2.end(); ++cur)
    {
        geoms2_cur = *cur;
        GEOSSTRtree_query(tree, geoms2_cur, cross_callback, geoms2_cur);
    }

    for (auto cur = geoms2.begin(); cur != geoms2.end(); ++cur)
    {
        geoms2_cur = *cur;
        GEOSGeom_destroy(geoms2_cur);
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

    vector<GEOSGeometry *> geoms2;
    FILE *input2;
    input2 = fopen(filename2, "r");
    char line2[MAX_LINE_LENGTH];
    while (fgets(line2, MAX_LINE_LENGTH, input2))
    {
        if (strlen(line2) > 5)
        {
            GEOSGeometry *geom2 = GEOSGeomFromWKT(line2);
            geoms2.push_back(geom2);
        }
    }

    GEOSGeometry *geoms2_cur;
    for (auto cur = geoms2.begin(); cur != geoms2.end(); ++cur)
    {
        geoms2_cur = *cur;
        GEOSSTRtree_query(tree, geoms2_cur, contain_callback, geoms2_cur);
    }

    for (auto cur = geoms2.begin(); cur != geoms2.end(); ++cur)
    {
        geoms2_cur = *cur;
        GEOSGeom_destroy(geoms2_cur);
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

        vector<GEOSGeometry *> geoms2;
    FILE *input2;
    input2 = fopen(filename2, "r");
    char line2[MAX_LINE_LENGTH];
    while (fgets(line2, MAX_LINE_LENGTH, input2))
    {
        if (strlen(line2) > 5)
        {
            GEOSGeometry *geom2 = GEOSGeomFromWKT(line2);
            geoms2.push_back(geom2);
        }
    }

    GEOSGeometry *geoms2_cur;
    for (auto cur = geoms2.begin(); cur != geoms2.end(); ++cur)
    {
        geoms2_cur = *cur;
        GEOSSTRtree_query(tree, geoms2_cur, equal_callback, geoms2_cur);
    }

    for (auto cur = geoms2.begin(); cur != geoms2.end(); ++cur)
    {
        geoms2_cur = *cur;
        GEOSGeom_destroy(geoms2_cur);
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

        vector<GEOSGeometry *> geoms2;
    FILE *input2;
    input2 = fopen(filename2, "r");
    char line2[MAX_LINE_LENGTH];
    while (fgets(line2, MAX_LINE_LENGTH, input2))
    {
        if (strlen(line2) > 5)
        {
            GEOSGeometry *geom2 = GEOSGeomFromWKT(line2);
            geoms2.push_back(geom2);
        }
    }

    GEOSGeometry *geoms2_cur;
    for (auto cur = geoms2.begin(); cur != geoms2.end(); ++cur)
    {
        geoms2_cur = *cur;
        GEOSSTRtree_query(tree, geoms2_cur, equal_exact_callback, geoms2_cur);
    }

    for (auto cur = geoms2.begin(); cur != geoms2.end(); ++cur)
    {
        geoms2_cur = *cur;
        GEOSGeom_destroy(geoms2_cur);
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

        vector<GEOSGeometry *> geoms2;
    FILE *input2;
    input2 = fopen(filename2, "r");
    char line2[MAX_LINE_LENGTH];
    while (fgets(line2, MAX_LINE_LENGTH, input2))
    {
        if (strlen(line2) > 5)
        {
            GEOSGeometry *geom2 = GEOSGeomFromWKT(line2);
            geoms2.push_back(geom2);
        }
    }

    GEOSGeometry *geoms2_cur;
    for (auto cur = geoms2.begin(); cur != geoms2.end(); ++cur)
    {
        geoms2_cur = *cur;
        GEOSSTRtree_query(tree, geoms2_cur, cover_callback, geoms2_cur);
    }

    for (auto cur = geoms2.begin(); cur != geoms2.end(); ++cur)
    {
        geoms2_cur = *cur;
        GEOSGeom_destroy(geoms2_cur);
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

        vector<GEOSGeometry *> geoms2;
    FILE *input2;
    input2 = fopen(filename2, "r");
    char line2[MAX_LINE_LENGTH];
    while (fgets(line2, MAX_LINE_LENGTH, input2))
    {
        if (strlen(line2) > 5)
        {
            GEOSGeometry *geom2 = GEOSGeomFromWKT(line2);
            geoms2.push_back(geom2);
        }
    }

    GEOSGeometry *geoms2_cur;
    for (auto cur = geoms2.begin(); cur != geoms2.end(); ++cur)
    {
        geoms2_cur = *cur;
        GEOSSTRtree_query(tree, geoms2_cur, covered_by_callback, geoms2_cur);
    }

    for (auto cur = geoms2.begin(); cur != geoms2.end(); ++cur)
    {
        geoms2_cur = *cur;
        GEOSGeom_destroy(geoms2_cur);
    }

    GEOSSTRtree_destroy(tree);
    for (size_t i = 0; i < index; i++)
    {
        GEOSGeom_destroy(geoms[i]);
    }
    finishGEOS();

    return 0;
}

/*vector<GEOSGeometry *> *read_wkt(const string filename, GEOSContextHandle_t ctx)
{
    vector<GEOSGeometry *> *geom = new vector<GEOSGeometry *>();
    ifstream f(filename);

    if (f.is_open())
    {
        string l;
        GEOSGeometry *possiblePolygon;
        GEOSWKTReader *parser = GEOSWKTReader_create_r(ctx);
        while (getline(f, l))
        {
            size_t wktStart = l.find('\t');
            size_t wktEnd = l.rfind('\t');
            if (wktStart == wktEnd)
            {
                aprintf(33 + integerLength(geom->size() + 1),
                        "Cannot find read line %d, skipping\n",
                        geom->size() + 1);
                continue;
            }
            l[wktEnd] = '\0';

            possiblePolygon =
                GEOSWKTReader_read_r(ctx, parser, l.c_str() + wktStart);

            if (GEOSGeomTypeId_r(ctx, possiblePolygon) != GEOS_POLYGON)
            {
                GEOSGeom_destroy_r(ctx, possiblePolygon);
            }
            else
            {

                if (GEOSisValid_r(ctx, possiblePolygon) != 1)
                {
                    possiblePolygon = GEOSMakeValid_r(ctx, possiblePolygon);
                    if (GEOSisValid_r(ctx, possiblePolygon) != 1)
                    {
                        aprintf(23, "Failed to fix polygon!\n");
                        GEOSGeom_destroy_r(ctx, possiblePolygon);
                        continue;
                    }
                }
                geom->push_back(possiblePolygon);
            }
        }

        GEOSWKTReader_destroy_r(ctx, parser);

        geom->shrink_to_fit();
    }
    else
    {
        aprintf(18 + filename.length(), "Could not open \"%s\"\n",
                filename.c_str());
    }

    return geom;
}*/

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

int main(int argc, char **argv)
{
    int n = 1;
    if (argc > 3)
        n = atoi(argv[3]);
    if (!n)
        n = 1;
    const char *filename = argv[1];
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
         << endl;
    cout << "------------------------------------------------------------------" << endl;
    cout << "------------------------ BENCHMARK RESULT ------------------------" << endl;
    cout << "------------------------------------------------------------------" << endl;
    cout << "Average Create Time: " << create_time << endl;
    cout << "Average Iterate Time: " << iterate_time << endl;
    cout << "Average Query Time: " << query_time << endl;
    cout << "Average Intersect Time: " << intersect_time << endl;
    cout << "Average Overlap Time: " << overlap_time << endl;
    cout << "Average Touch Time: " << touch_time << endl;
    cout << "Average Cross Time: " << cross_time << endl;
    cout << "Average Contain Time: " << contain_time << endl;
    cout << "Average Equal Time: " << equal_time << endl;
    cout << "Average Equal Exact (0.3) Time: " << equal_exact_time << endl;
    cout << "Average Cover Time: " << cover_time << endl;
    cout << "Average Covered By Time: " << covered_by_time << endl;
    cout << "------------------------------------------------------------------" << endl;
    cout << "------------------------------------------------------------------" << endl;
    cout << endl
         << endl;

    return 0;
}