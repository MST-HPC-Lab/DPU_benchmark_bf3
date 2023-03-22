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

int create_tree(vector<GEOSGeometry *> *geoms, vector<GEOSGeometry *> *geoms2)
{

    GEOSSTRtree *tree = GEOSSTRtree_create(10);

    // std::ifstream input(filename);
    // for (std::string line; getline(input, line);)
    // {
    //     if (line.length() > 5 && line.find("("))
    //     {
    //         GEOSGeometry *geom = GEOSGeomFromWKT(line.c_str());
    //         GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
    //     }
    // }

    for (auto cur = geoms->begin(); cur != geoms->end(); ++cur)
    {
        GEOSGeometry *geom = *cur;
        GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
    }

    GEOSSTRtree_destroy(tree);

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

int iterate_tree(vector<GEOSGeometry *> *geoms, vector<GEOSGeometry *> *geoms2)
{

    GEOSSTRtree *tree = GEOSSTRtree_create(10);

    for (auto cur = geoms->begin(); cur != geoms->end(); ++cur)
    {
        GEOSGeometry *geom = *cur;
        GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
    }

    GEOSSTRtree_iterate(tree, iterate_tree_callback, 0);
    // cout << "Rank " << processRank << ": File 1 # of Geometries: " << element_count << endl;

    GEOSSTRtree_destroy(tree);

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

int query(vector<GEOSGeometry *> *geoms, vector<GEOSGeometry *> *geoms2)
{

    GEOSSTRtree *tree = GEOSSTRtree_create(10);

    for (auto cur = geoms->begin(); cur != geoms->end(); ++cur)
    {
        GEOSGeometry *geom = *cur;
        GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
    }

    // vector<GEOSGeometry *> geoms2;
    // std::ifstream input2(filename2);
    // for (std::string line2; getline(input2, line2);)
    // {
    //     if (line2.length() > 5 && line2.find("("))
    //     {
    //         GEOSGeometry *geom2 = GEOSGeomFromWKT(line2.c_str());
    //         geoms2.push_back(geom2);
    //     }
    // }

    GEOSGeometry *geoms2_cur;
    for (auto cur = geoms2->begin(); cur != geoms2->end(); ++cur)
    {
        geoms2_cur = *cur;
        GEOSSTRtree_query(tree, geoms2_cur, query_callback, geoms2_cur);
    }

    // for (auto cur = geoms2->begin(); cur != geoms2->end(); ++cur)
    // {
    //     geoms2_cur = *cur;
    //     GEOSGeom_destroy(geoms2_cur);
    // }

    GEOSSTRtree_destroy(tree);

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

int intersect(vector<GEOSGeometry *> *geoms, vector<GEOSGeometry *> *geoms2)
{

    GEOSSTRtree *tree = GEOSSTRtree_create(10);

    for (auto cur = geoms->begin(); cur != geoms->end(); ++cur)
    {
        GEOSGeometry *geom = *cur;
        GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
    }

    GEOSGeometry *geoms2_cur;
    for (auto cur = geoms2->begin(); cur != geoms2->end(); ++cur)
    {
        geoms2_cur = *cur;
        GEOSSTRtree_query(tree, geoms2_cur, intersect_callback, geoms2_cur);
    }

    GEOSSTRtree_destroy(tree);

    return 0;
}

double select_test(const char *name, int (*test_function)(vector<GEOSGeometry *> *, vector<GEOSGeometry *> *), vector<GEOSGeometry *> *geoms, vector<GEOSGeometry *> *geoms2, int n)
{
    double time_arr[n];
    for (int i = 0; i < n; i++)
    {
        // clock_t t;
        // t = clock();

        struct timeval tv1, tv2;
        gettimeofday(&tv1, NULL);

        test_function(geoms, geoms2);

        gettimeofday(&tv2, NULL);
        double time_spent = (double)(tv2.tv_usec - tv1.tv_usec) / 1000000 + (double)(tv2.tv_sec - tv1.tv_sec);
        time_arr[i] = time_spent;

        // t = clock() - t;
        // double time_taken = ((double)t) / CLOCKS_PER_SEC;
        // time_arr[i] = time_taken;
    }

    // cout << "------------------- " << name << " -------------------" << endl;
    // for (int i = 0; i < n; i++)
    // {
    //     cout << "The program " << i << " took " << time_arr[i] << " seconds to execute" << endl;
    // }
    // cout << "---------------------------------------------" << endl
    //      << endl;

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

    initGEOS(geos_message_handler, geos_message_handler);

    int n = 1;
    if (argc > 4)
        n = atoi(argv[4]);
    if (!n)
        n = 1;

    double create_time = 0;
    double iterate_time = 0;
    double query_time = 0;
    double intersect_time = 0;

    const char *filename = argv[1];
    const char *filename2 = argv[2];

    try
    {
        vector<GEOSGeometry *> *geoms = get_polygons(argv[1]);
        // vector<GEOSGeometry *> *geoms2 = get_polygons((string(argv[2]) + "/" + to_string(rank + i)).c_str());
        if (geoms->size() > 0)
        {
            vector<GEOSGeometry *> *geoms2 = get_polygons(argv[2]);
            // create_time += select_test("Create", &create_tree, geoms, geoms2, n);
            // iterate_time += select_test("Iterate", &iterate_tree, geoms, geoms2, n);
            // query_time += select_test("Query", &query, geoms, geoms2, n);
            intersect_time += select_test("Intersect", &intersect, geoms, geoms2, n);

            GEOSGeometry *geom;
            for (auto cur = geoms->begin(); cur != geoms->end(); ++cur)
            {
                geom = *cur;
                GEOSGeom_destroy(geom);
            }
            for (auto cur = geoms2->begin(); cur != geoms2->end(); ++cur)
            {
                geom = *cur;
                GEOSGeom_destroy(geom);
            }
        }
    }
    catch (...)
    {
        // cout << "No file named " << rank + i << endl;
        cout << "No file" << endl;
        // continue;
    }

    cout << endl
         << endl
         << "------------------------------------------------------------------" << endl
         << "------------------------ BENCHMARK RESULT ------------------------" << endl
         << "------------------------------------------------------------------" << endl
         << argv[1] << " - " << argv[2] << " - " << endl
         << "Max Intersect Time: " << intersect_time << endl
         << "------------------------------------------------------------------" << endl
         << "------------------------------------------------------------------" << endl;

    finishGEOS();
    return 0;
}