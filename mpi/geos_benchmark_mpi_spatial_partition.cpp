// This program performs various geospatial operations using 
// the two given geospatial datasets and outputs the elapsed time. 
// This program uses the MPI library. Both files should be spatially partitioned 
// and the directory names of the data should be given as the first two arguments.
// You have to specifiy the number of partitions as the third argument.
// Do NOT add a "/" onto the ends of the directory paths, as that is done internally.
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
#include <limits>
// #include <numeric> // for std::accumulate(vector.begin(), vector.end(), 0.0) or std::reduce(vector.begin(), vector.end())
#include <sys/time.h>

#include <mpi.h>

using namespace std;
enum Tag {
   WORK_TAG,
   TERMINATION_TAG,
   // WARNING: If add to this, refactor code below to include checks for other tags
};

const double I = std::numeric_limits<double>::max();

const int root = 0; // The process handling the control
int processRank;
int numberOfPartitions;
int numProcs;



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

//----------------------------------------------------------------------------//

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

void all_tests(double* test_time_arr, int filenum, char *dir1, char *dir2, int n_repeats)
{
    // const char *filename = (string(argv[1]) + "/" + to_string(rank + filenum)).c_str();
    // const char *filename2 = (string(argv[2]) + "/" + to_string(rank + filenum)).c_str();
    // cout << "File: " << (string(argv[1]) + "/" + to_string(rank + filenum)).c_str() << endl;
    // cout << "File2: " << (string(argv[2]) + "/" + to_string(rank + filenum)).c_str() << endl;

    // Start total timer
    // struct timeval tv1, tv2;
    // gettimeofday(&tv1, NULL);
    double time = -MPI_Wtime();

    // Run tests
    try
    {
        vector<GEOSGeometry *> *geoms = get_polygons((string(dir1) + "/" + to_string(filenum)).c_str());
        // vector<GEOSGeometry *> *geoms2 = get_polygons((string(argv[2]) + "/" + to_string(rank + filenum)).c_str());
        if (geoms->size() > 0)
        {
            vector<GEOSGeometry *> *geoms2 = get_polygons((string(dir2) + "/" + to_string(filenum)).c_str());
            
            test_time_arr[ 0] += select_test("Create",            &create_tree,  geoms, geoms2, n_repeats);
            test_time_arr[ 1] += select_test("Iterate",           &iterate_tree, geoms, geoms2, n_repeats);
            test_time_arr[ 2] += select_test("Query",             &query,        geoms, geoms2, n_repeats);
            test_time_arr[ 3] += select_test("Intersect",         &intersect,    geoms, geoms2, n_repeats);
            test_time_arr[ 4] += select_test("Overlap",           &overlap,      geoms, geoms2, n_repeats);
            test_time_arr[ 5] += select_test("Touch",             &touch,        geoms, geoms2, n_repeats);
            test_time_arr[ 6] += select_test("Cross",             &cross,        geoms, geoms2, n_repeats);
            test_time_arr[ 7] += select_test("Contain",           &contain,      geoms, geoms2, n_repeats);
            test_time_arr[ 8] += select_test("Equal",             &equal,        geoms, geoms2, n_repeats);
            test_time_arr[ 9] += select_test("Equal Exact (0.3)", &equal_exact,  geoms, geoms2, n_repeats);
            test_time_arr[10] += select_test("Cover",             &cover,        geoms, geoms2, n_repeats);
            test_time_arr[11] += select_test("Covered By",        &covered_by,   geoms, geoms2, n_repeats);
            // test_time_arr[12] += create_time + iterate_time + query_time + intersect_time + overlap_time + touch_time + cross_time + contain_time + equal_time + equal_exact_time + cover_time + covered_by_time;

            destroy_polygons(geoms2);
        }
        destroy_polygons(geoms);
    }
    catch (...)
    {
        cout << "PARTIAL ABORT: No file named " << filenum << endl;
    }

    // End total timer
    // gettimeofday(&tv2, NULL);
    // test_time_arr[12] += (double)(tv2.tv_usec - tv1.tv_usec) / 1000000 + (double)(tv2.tv_sec - tv1.tv_sec);
    time += MPI_Wtime();
    test_time_arr[12] += time/(double)n_repeats; // This time, like the others, reflects the average of the n duplications
}

double all_files_round_robin(char *dir1, char *dir2, int n_repeats, int limit_procs, bool print_categories)
{
    // Fixed round robin over partition files
    // Also can be used for sequential (non-parallel) test, if limit_procs == 1
    if (processRank >= limit_procs) return 0.0;

    // Represents the times of all the different operations, and will be cumulative over all partitions that this process will do,
    //    but is for the local process only
    // Order: create_time, iterate_time, query_time, intersect_time, overlap_time, touch_time, cross_time, contain_time, equal_time, equal_exact_time, cover_time, covered_by_time, total_time
    double test_time_arr[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    // Places to store result stats gathered from accross all processes
    double comb_time_arr_sum[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    double comb_time_max = 0;
    double comb_time_min = 0;
    double comb_time_avg = 0;
    double comb_time_range = 0;

    // Start total timer
    double total_time = 0;
    if (processRank == root) total_time = -MPI_Wtime();

    // Do the actual work
    for (int n=0; n<n_repeats; n++) { // over-ride the n built-in to all_tests, so that we're doing it the same way the load-balancing function has to
        for (int filenum = processRank; filenum < numberOfPartitions; filenum += limit_procs) {
            all_tests(test_time_arr, filenum, dir1, dir2, 1);
            if (processRank == root) {
                printf("\rCURRENT FILENUM: %d", filenum);
                fflush(stdout);
            }
        }
    }
    for (int i=0; i<13; i++) test_time_arr[i] = test_time_arr[i] / (double)n_repeats;

    // Reduce between processes
    double reduction_time = -MPI_Wtime();
    MPI_Reduce(test_time_arr, comb_time_arr_sum, 13, MPI_DOUBLE, MPI_SUM, root, MPI_COMM_WORLD);
    MPI_Reduce(&(test_time_arr[12]), &comb_time_max, 1, MPI_DOUBLE, MPI_MAX, root, MPI_COMM_WORLD);
    if (processRank >= limit_procs) { // This is a hack to prevent the 0s from unused procs from being the min.
        MPI_Reduce(&I, &comb_time_min, 1, MPI_DOUBLE, MPI_MIN, root, MPI_COMM_WORLD);}
    else MPI_Reduce(&(test_time_arr[12]), &comb_time_min, 1, MPI_DOUBLE, MPI_MIN, root, MPI_COMM_WORLD);
    reduction_time += MPI_Wtime();

    if (processRank == root) {
        comb_time_avg = comb_time_arr_sum[12]/limit_procs;
        comb_time_range = comb_time_max - comb_time_min;
        // for (int i=0; i<12; i++) comb_time_arr_sum[i] /= comb_time_arr_sum[12];
    }

    // End total timer
    if (processRank == root) total_time += MPI_Wtime();

    // Print Report
    if (processRank == root) {
           cout << "-------------------------- ROUND ROBIN ---------------------------" << endl
                << "Workers: " << limit_procs << "  Duplication: " << n_repeats << endl;
        if (print_categories) {
           cout << "------------------------------------------------------------------" << endl
                << "Create Time:             " << comb_time_arr_sum[ 0] << endl
                << "Iterate Time:            " << comb_time_arr_sum[ 1] << endl
                << "Query Time:              " << comb_time_arr_sum[ 2] << endl
                << "Intersect Time:          " << comb_time_arr_sum[ 3] << endl
                << "Overlap Time:            " << comb_time_arr_sum[ 4] << endl
                << "Touch Time:              " << comb_time_arr_sum[ 5] << endl
                << "Cross Time:              " << comb_time_arr_sum[ 6] << endl
                << "Contain Time:            " << comb_time_arr_sum[ 7] << endl
                << "Equal Time:              " << comb_time_arr_sum[ 8] << endl
                << "Equal Exact (0.3) Time:  " << comb_time_arr_sum[ 9] << endl
                << "Cover Time:              " << comb_time_arr_sum[10] << endl
                << "Covered By Time:         " << comb_time_arr_sum[11] << endl
                ;
        }
        cout    << "------------------------------------------------------------------" << endl
                << "Reduction Time:      " << reduction_time        << endl
                << "Sequential Time:     " << comb_time_arr_sum[12] << endl
                << "Avg Worker Time:     " << comb_time_avg         << endl
                << "Bottleneck Range:    " << comb_time_range       << endl
                << "Min Worker Time:     " << comb_time_min         << endl
                << "Max Worker Time:     " << comb_time_max         << endl
                << "------------------------------------------------------------------" << endl
                << "TOTAL PARALLEL Time: " << total_time      << endl
                << "==================================================================" << endl
                ;
    }

    return total_time;
}

double all_files_load_balancing(char *dir1, char*dir2, int n_repeats, int limit_procs)
{
    // Allocates work as workers finish
    if (processRank >= limit_procs) return 0.0;
    
    MPI_Status status;
    int filenum;

    // Represents the times of all the different operations, and will be cumulative over all partitions that this process will do,
    //    but is for the local process only
    // Order: create_time, iterate_time, query_time, intersect_time, overlap_time, touch_time, cross_time, contain_time, equal_time, equal_exact_time, cover_time, covered_by_time, total_time
    double test_time_arr[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    // Places to store result stats gathered from accross all processes
    double comb_time_sum = 0;
    double comb_time_max = 0;
    double comb_time_min = 0;
    double comb_time_avg = 0;
    double comb_time_range = 0;

    // Start total timer
    double total_time = 0;
    if (processRank == root) total_time = -MPI_Wtime();

    // Do the actual work
    for (int n=0; n<n_repeats; n++) { // over-ride the n built-in to all_tests, because this way allows work to be reshuffled naturally

        if (processRank != root) { // Is Worker
            // Workers start work automatically
            int tasks_done = 0;
            if (processRank < numberOfPartitions) {
                all_tests(test_time_arr, processRank, dir1, dir2, n);
                // cout << "WORKER> Partition " << processRank << " from " << processRank << endl;
                tasks_done++;
            }

            // Then wait for more
            MPI_Send(NULL, 0, MPI_INT, root, WORK_TAG, MPI_COMM_WORLD);
            while (true) {
                MPI_Recv(&filenum, 1, MPI_INT, root, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                if (status.MPI_TAG == TERMINATION_TAG) { break; }
                else {
                    all_tests(test_time_arr, filenum, dir1, dir2, n);
                    tasks_done++;
                    // cout << "WORKER> Partition " << filenum << " from " << processRank << endl;
                    MPI_Send(NULL, 0, MPI_INT, root, WORK_TAG, MPI_COMM_WORLD);
                }
            }
            printf("WORKER %d COMPLETED %d TASKS\n", processRank, tasks_done);
            fflush(stdout);
        } else { // Is Master, i.e.: root
            cout << endl;

            // Start the main receiving/work-serving loop
            for (filenum = numProcs; filenum < numberOfPartitions; filenum++) {
                MPI_Recv(NULL, 0, MPI_INT, MPI_ANY_SOURCE, WORK_TAG, MPI_COMM_WORLD, &status);
                // cout << "MASTER> Received from " << status.MPI_SOURCE << endl;
                MPI_Send(&filenum, 1, MPI_INT, status.MPI_SOURCE, WORK_TAG, MPI_COMM_WORLD);
                printf("\rCURRENT FILENUM: %d", filenum);
                fflush(stdout);
            }
            cout << endl;
            // cout << "MASTER> Finished work" << endl;
            // Receive the last one from each, and then tell them the work is done
            for (int i=1; i<numProcs; i++) {
                MPI_Recv(NULL, 0, MPI_INT, MPI_ANY_SOURCE, WORK_TAG, MPI_COMM_WORLD, &status);
                MPI_Send(&filenum, 1, MPI_INT, status.MPI_SOURCE, TERMINATION_TAG, MPI_COMM_WORLD);
                // cout << "MASTER> Sent termination to " << i << endl;
            }
        }
    }
    for (int i=0; i<13; i++) test_time_arr[i] = test_time_arr[i] / (double)n_repeats;

    // Reduce between processes
    double reduction_time = -MPI_Wtime();
    MPI_Reduce(&(test_time_arr[12]), &comb_time_sum, 1, MPI_DOUBLE, MPI_SUM, root, MPI_COMM_WORLD);
    MPI_Reduce(&(test_time_arr[12]), &comb_time_max, 1, MPI_DOUBLE, MPI_MAX, root, MPI_COMM_WORLD);
    if (processRank >= limit_procs+1 || processRank == root) { // This is a hack to prevent the 0s from unused procs from being the min.
        MPI_Reduce(&I, &comb_time_min, 1, MPI_DOUBLE, MPI_MIN, root, MPI_COMM_WORLD);}
    else MPI_Reduce(&(test_time_arr[12]), &comb_time_min, 1, MPI_DOUBLE, MPI_MIN, root, MPI_COMM_WORLD);
    reduction_time += MPI_Wtime();

    if (processRank == root) {
        comb_time_avg = comb_time_sum/limit_procs;
        comb_time_range = comb_time_max - comb_time_min;
        // for (int i=0; i<12; i++) comb_time_arr_sum[i] /= comb_time_arr_sum[12];
    }

    // End total timer
    if (processRank == root) total_time += MPI_Wtime();

    // Print Report
    if (processRank == root) {
        cout    << "------------------------ LOAD-BALANCING --------------------------" << endl
                << "Workers: " << limit_procs << "  Duplication: " << n_repeats << endl;
        cout    << "------------------------------------------------------------------" << endl
                << "Reduction Time:      " << reduction_time  << endl
                << "Sequential Time:     " << comb_time_sum   << endl
                << "Avg Worker Time:     " << comb_time_avg   << endl
                << "Bottleneck Range:    " << comb_time_range << endl
                << "Min Worker Time:     " << comb_time_min   << endl
                << "Max Worker Time:     " << comb_time_max   << endl
                << "------------------------------------------------------------------" << endl
                << "TOTAL PARALLEL Time: " << total_time      << endl
                << "==================================================================" << endl
                ;
    }

    return total_time;
}

int main(int argc, char **argv)
{
    int rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    initGEOS(geos_message_handler, geos_message_handler);
    processRank = rank;

    numberOfPartitions = atoi(argv[3]);
    int n = 1;
    if (argc > 4)
        n = atoi(argv[4]);
    if (!n)
        n = 1;

    double total_time;
    if (rank == root)
    {        
        // Generate Report Start
        cout << endl
             << endl
             << "==================================================================" << endl
             << "----------------------- BENCHMARK RESULTS ------------------------" << endl
             << "------------------------------------------------------------------" << endl
             << "Processes:  " << numProcs << "  Partitions: " << argv[3] << "  Duplication (n): " << n << endl
             << "Directory1: " << argv[1] << endl
             << "Directory2: " << argv[2] << endl
             << "NOTE: All polygons are discarded and re-read between test suites." << endl
             << "==================================================================" << endl
             ;

        total_time = -MPI_Wtime();
    }

    // Perform the actual varied test types
    double seq_time = all_files_round_robin(argv[1], argv[2], n, 1, true);
    double rr_time  = all_files_round_robin(argv[1], argv[2], n, numProcs, false);
    double rr2_time = all_files_round_robin(argv[1], argv[2], n, numProcs-1, false);
    double lb_time  = all_files_load_balancing(argv[1], argv[2], n, numProcs);
    double rr2_speedup = seq_time/rr2_time;
    double rr_speedup = seq_time/rr_time;
    double lb_speedup  = seq_time/lb_time;

    if (rank == root)
    {
        total_time += MPI_Wtime();

        // if (numProcs > 1) system("rm ./file_part_*");

        // Generate Report End
        cout << "-------------------- PARALLELISM REPORT --------------------------" << endl
             << "Sequential Time:                  " << seq_time                     << endl
             << "Round Robin Parallel Time:        " << rr_time                      << endl
             << "     Speedup:                     " << rr_speedup                   << endl
             << "     Efficiency:                  " << rr_speedup/numProcs          << endl
             << "*Round Robin Parallel-1 Time:     " << rr2_time                     << endl
             << "     Speedup:                     " << rr2_speedup                  << endl
             << "     Efficiency:                  " << rr2_speedup/(numProcs-1)     << endl
             << "*Load-Balancing Par. Time:        " << lb_time                      << endl
             << "     Bottleneck Speedup (uses *): " << rr2_time/lb_time             << endl
             << "     Speedup:                     " << lb_speedup                   << endl
             << "     Efficiency:                  " << lb_speedup/numProcs          << endl
             << "------------------------------------------------------------------" << endl
             << "OVERALL TESTS RUNTIME:            " << total_time                   << endl
             << "==================================================================" << endl
             << endl
             << endl
            ;
    }

    finishGEOS();
    MPI_Finalize();
    return 0;
}