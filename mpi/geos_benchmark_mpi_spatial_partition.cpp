// This program performs various geospatial operations using 
// the two given geospatial datasets and outputs the elapsed time. 
// This program uses the MPI library. Both files should be spatially partitioned 
// and the directory names of the data should be given as the first two arguments.
// You have to specifiy the number of partitions as the third argument.
//
// USAGE
// -----
// mpic++ geos_benchmark.cpp
// mpirun -np <# of processes>  ./a.out <directorypath1> <directorypath2> <# of partitions> <# of repetition of operations>
// mpirun -np 8 ./geos_benchmark_mpi_spatial_partition ../Data/spatial_partition/cemetery/64_parts_cemetery ../Data/spatial_partition/sports/64_parts_sports 64 1
// mpirun -np 8 ./geos_benchmark_mpi_spatial_partition ../Data/spatial_partition/sports/64_parts_sports ../Data/spatial_partition/lakes/64_parts_lakes 64 1

#include <iostream>
#include <stdarg.h>
#include <stdlib.h>
#include <fstream>

#include <geos_c.h>
#include <cstring>
#include <vector>
#include <sys/time.h>

#include <mpi.h>

using namespace std;

// int processRank = -1;


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

void destroy_polygons(vector<GEOSGeometry *> *geoms)
{
    GEOSGeometry *geom;
    for (auto cur = geoms->begin(); cur != geoms->end(); ++cur)
    {
        geom = *cur;
        GEOSGeom_destroy(geom);
    }
    delete geoms;
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

int overlap(vector<GEOSGeometry *> *geoms, vector<GEOSGeometry *> *geoms2)
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
        GEOSSTRtree_query(tree, geoms2_cur, overlap_callback, geoms2_cur);
    }

    GEOSSTRtree_destroy(tree);

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

int touch(vector<GEOSGeometry *> *geoms, vector<GEOSGeometry *> *geoms2)
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
        GEOSSTRtree_query(tree, geoms2_cur, touch_callback, geoms2_cur);
    }

    GEOSSTRtree_destroy(tree);

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

int cross(vector<GEOSGeometry *> *geoms, vector<GEOSGeometry *> *geoms2)
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
        GEOSSTRtree_query(tree, geoms2_cur, cross_callback, geoms2_cur);
    }

    GEOSSTRtree_destroy(tree);

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

int contain(vector<GEOSGeometry *> *geoms, vector<GEOSGeometry *> *geoms2)
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
        GEOSSTRtree_query(tree, geoms2_cur, contain_callback, geoms2_cur);
    }

    GEOSSTRtree_destroy(tree);

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

int equal(vector<GEOSGeometry *> *geoms, vector<GEOSGeometry *> *geoms2)
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
        GEOSSTRtree_query(tree, geoms2_cur, equal_callback, geoms2_cur);
    }

    GEOSSTRtree_destroy(tree);

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

int equal_exact(vector<GEOSGeometry *> *geoms, vector<GEOSGeometry *> *geoms2)
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
        GEOSSTRtree_query(tree, geoms2_cur, equal_exact_callback, geoms2_cur);
    }

    GEOSSTRtree_destroy(tree);

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

int cover(vector<GEOSGeometry *> *geoms, vector<GEOSGeometry *> *geoms2)
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
        GEOSSTRtree_query(tree, geoms2_cur, cover_callback, geoms2_cur);
    }

    GEOSSTRtree_destroy(tree);

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

int covered_by(vector<GEOSGeometry *> *geoms, vector<GEOSGeometry *> *geoms2)
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
        GEOSSTRtree_query(tree, geoms2_cur, covered_by_callback, geoms2_cur);
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
    int rank;
    int numProcs;
    int root = 0; // The process handling the control

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    initGEOS(geos_message_handler, geos_message_handler);
    // processRank = rank;

    int numberOfPartitions = atoi(argv[3]);
    int n = 1;
    if (argc > 4)
        n = atoi(argv[4]);
    if (!n)
        n = 1;

    double create_time = 0;
    double iterate_time = 0;
    double query_time = 0;
    double intersect_time = 0;
    double overlap_time = 0;
    double touch_time = 0;
    double cross_time = 0;
    double contain_time = 0;
    double equal_time = 0;
    double equal_exact_time = 0;
    double cover_time = 0;
    double covered_by_time = 0;
    double total_time = 0;

    for (int i = 0; i < numberOfPartitions; i += numProcs) // Fixed round robin over partitions
    {
        // const char *filename = (string(argv[1]) + "/" + to_string(rank + i)).c_str();
        // const char *filename2 = (string(argv[2]) + "/" + to_string(rank + i)).c_str();
        // cout << "File: " << (string(argv[1]) + "/" + to_string(rank + i)).c_str() << endl;
        // cout << "File2: " << (string(argv[2]) + "/" + to_string(rank + i)).c_str() << endl;
        try
        {
            vector<GEOSGeometry *> *geoms = get_polygons((string(argv[1]) + "/" + to_string(rank + i)).c_str());
            // vector<GEOSGeometry *> *geoms2 = get_polygons((string(argv[2]) + "/" + to_string(rank + i)).c_str());
            if (geoms->size() > 0)
            {
                vector<GEOSGeometry *> *geoms2 = get_polygons((string(argv[2]) + "/" + to_string(rank + i)).c_str());
                
                create_time += select_test("Create", &create_tree, geoms, geoms2, n);
                iterate_time += select_test("Iterate", &iterate_tree, geoms, geoms2, n);
                query_time += select_test("Query", &query, geoms, geoms2, n);
                intersect_time += select_test("Intersect", &intersect, geoms, geoms2, n);
                overlap_time += select_test("Overlap", &overlap, geoms, geoms2, n);
                touch_time += select_test("Touch", &touch, geoms, geoms2, n);
                cross_time += select_test("Cross", &cross, geoms, geoms2, n);
                contain_time += select_test("Contain", &contain, geoms, geoms2, n);
                equal_time += select_test("Equal", &equal, geoms, geoms2, n);
                equal_exact_time += select_test("Equal Exact (0.3)", &equal_exact, geoms, geoms2, n);
                cover_time += select_test("Cover", &cover, geoms, geoms2, n);
                covered_by_time += select_test("Covered By", &covered_by, geoms, geoms2, n);
                
                total_time = create_time + iterate_time + query_time + intersect_time + overlap_time + touch_time + cross_time + contain_time + equal_time + equal_exact_time + cover_time + covered_by_time;

                // GEOSGeometry *geom;
                // for (auto cur = geoms->begin(); cur != geoms->end(); ++cur)
                // {
                //     geom = *cur;
                //     GEOSGeom_destroy(geom);
                // }
                // for (auto cur = geoms2->begin(); cur != geoms2->end(); ++cur)
                // {
                //     geom = *cur;
                //     GEOSGeom_destroy(geom);
                // }
                // delete geoms2;
                destroy_polygons(geoms2);
            }
            destroy_polygons(geoms);
            // delete geoms;
        }
        catch (...)
        {
            cout << "No file named " << rank + i << endl;
            continue;
        }
    }

    // cout << endl
    //      << endl
    //      << "------------------------------------------------------------------" << endl
    //      << "--------------- BENCHMARK RESULT (Process rank:" << rank << ") ---------------" << endl
    //      << "------------------------------------------------------------------" << endl
    //      << "Average Create Time: " << create_time << endl
    //      << "Average Iterate Time: " << iterate_time << endl
    //      << "Average Query Time: " << query_time << endl
    //      << "Average Intersect Time: " << intersect_time << endl
    //      << "Average Overlap Time: " << overlap_time << endl
    //      << "Average Touch Time: " << touch_time << endl
    //      << "Average Cross Time: " << cross_time << endl
    //      << "Average Contain Time: " << contain_time << endl
    //      << "Average Equal Time: " << equal_time << endl
    //      << "Average Equal Exact (0.3) Time: " << equal_exact_time << endl
    //      << "Average Cover Time: " << cover_time << endl
    //      << "Average Covered By Time: " << covered_by_time << endl
    //      << "Average TOTAL: " << create_time + iterate_time + query_time + intersect_time + overlap_time + touch_time + cross_time + contain_time + equal_time + equal_exact_time + cover_time + covered_by_time << endl
    //      << "------------------------------------------------------------------" << endl
    //      << "------------------------------------------------------------------" << endl
    //      << endl
    //      << endl;

    double test_time_arr_max[13]   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    double test_time_arr_min[13]   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    double test_time_arr_sum[13]   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    double test_time_arr_avg[13]   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    double test_time_arr_range[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    double test_time_arr[13] = {create_time, iterate_time, query_time, intersect_time, overlap_time, touch_time,
                                cross_time, contain_time, equal_time, equal_exact_time, cover_time, covered_by_time,
                                total_time};

    MPI_Reduce(test_time_arr, test_time_arr_max, 13, MPI_DOUBLE, MPI_MAX, root, MPI_COMM_WORLD);
    MPI_Reduce(test_time_arr, test_time_arr_min, 13, MPI_DOUBLE, MPI_MIN, root, MPI_COMM_WORLD);
    MPI_Reduce(test_time_arr, test_time_arr_sum, 13, MPI_DOUBLE, MPI_SUM, root, MPI_COMM_WORLD);
    // Avg and range arrays filled later

    if (rank == root)
    {
        // Fill avg and range arrays
        for (int i = 0; i < 13; i++) {
            test_time_arr_avg[i] = test_time_arr_sum[i]/numProcs;
            test_time_arr_range[i] = test_time_arr_max[i] - test_time_arr_min[i];
        }

        // if (numProcs > 1)
        // {
        //     system("rm ./file_part_*");
        // }
        
        // Generate Report
        cout << endl
             << endl
             << "------------------------------------------------------------------" << endl
             << "--------- BENCHMARK RESULT (TOTAL, AVG, RANGE, MIN, MAX) ---------" << endl
             << "------------------------------------------------------------------" << endl
             << "Processes:  " << numProcs << "  Partitions: " << argv[3] << "  Replication (n): " << n << endl
             << "Directory1: " << argv[1] << endl
             << "Directory2: " << argv[2] << endl
             << "------------------------------------------------------------------" << endl
             << "Create Time:             " << test_time_arr_sum[ 0] << ", " << test_time_arr_avg[ 0] << ", " << test_time_arr_range[ 0] << ", " << test_time_arr_min[ 0] << ", " << test_time_arr_max[ 0] << endl
             << "Iterate Time:            " << test_time_arr_sum[ 1] << ", " << test_time_arr_avg[ 1] << ", " << test_time_arr_range[ 1] << ", " << test_time_arr_min[ 1] << ", " << test_time_arr_max[ 1] << endl
             << "Query Time:              " << test_time_arr_sum[ 2] << ", " << test_time_arr_avg[ 2] << ", " << test_time_arr_range[ 2] << ", " << test_time_arr_min[ 2] << ", " << test_time_arr_max[ 2] << endl
             << "Intersect Time:          " << test_time_arr_sum[ 3] << ", " << test_time_arr_avg[ 3] << ", " << test_time_arr_range[ 3] << ", " << test_time_arr_min[ 3] << ", " << test_time_arr_max[ 3] << endl
             << "Overlap Time:            " << test_time_arr_sum[ 4] << ", " << test_time_arr_avg[ 4] << ", " << test_time_arr_range[ 4] << ", " << test_time_arr_min[ 4] << ", " << test_time_arr_max[ 4] << endl
             << "Touch Time:              " << test_time_arr_sum[ 5] << ", " << test_time_arr_avg[ 5] << ", " << test_time_arr_range[ 5] << ", " << test_time_arr_min[ 5] << ", " << test_time_arr_max[ 5] << endl
             << "Cross Time:              " << test_time_arr_sum[ 6] << ", " << test_time_arr_avg[ 6] << ", " << test_time_arr_range[ 6] << ", " << test_time_arr_min[ 6] << ", " << test_time_arr_max[ 6] << endl
             << "Contain Time:            " << test_time_arr_sum[ 7] << ", " << test_time_arr_avg[ 7] << ", " << test_time_arr_range[ 7] << ", " << test_time_arr_min[ 7] << ", " << test_time_arr_max[ 7] << endl
             << "Equal Time:              " << test_time_arr_sum[ 8] << ", " << test_time_arr_avg[ 8] << ", " << test_time_arr_range[ 8] << ", " << test_time_arr_min[ 8] << ", " << test_time_arr_max[ 8] << endl
             << "Equal Exact (0.3) Time:  " << test_time_arr_sum[ 9] << ", " << test_time_arr_avg[ 9] << ", " << test_time_arr_range[ 9] << ", " << test_time_arr_min[ 9] << ", " << test_time_arr_max[ 9] << endl
             << "Cover Time:              " << test_time_arr_sum[10] << ", " << test_time_arr_avg[10] << ", " << test_time_arr_range[10] << ", " << test_time_arr_min[10] << ", " << test_time_arr_max[10] << endl
             << "Covered By Time:         " << test_time_arr_sum[11] << ", " << test_time_arr_avg[11] << ", " << test_time_arr_range[11] << ", " << test_time_arr_min[11] << ", " << test_time_arr_max[11] << endl
             << "TOTAL TIME:              " << test_time_arr_sum[12] << ", " << test_time_arr_avg[12] << ", " << test_time_arr_range[12] << ", " << test_time_arr_min[12] << ", " << test_time_arr_max[12] << endl
             << "------------------------------------------------------------------" << endl
             << "------------------------------------------------------------------" << endl
            //  << endl
            //  << endl
            ;
    }

    finishGEOS();
    MPI_Finalize();
    return 0;
}