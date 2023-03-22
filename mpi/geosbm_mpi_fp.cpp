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

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    processRank = rank;

    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    initGEOS(geos_message_handler, geos_message_handler);

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

    char *filenameTemp;
    if (numProcs > 1)
    {

        if (rank == 0)
        {
            string line;
            int line_count = 0;
            ifstream file(argv[1]);
            while (getline(file, line))
                line_count++;

            int line_count_per_file = (line_count / numProcs) + 1;
            system((string("split -l ") + to_string(line_count_per_file) + " " + argv[1] + string(" -d --suffix-length=3 ") + argv[1] + string("_") + to_string(numProcs) +  string("_")).c_str());
        }
        MPI_Barrier(MPI_COMM_WORLD);

        string filenameTempSuffix = rank < 10 ? "00" : rank < 100 ? "0"
                                                                  : "";
        // filenameTemp = (char *)(string("file_part_") + filenameTempSuffix + to_string(rank)).c_str();
        filenameTemp = (char *)(argv[1] + string("_") + to_string(numProcs) + string("_") + filenameTempSuffix + to_string(rank)).c_str();
    }
    else
    {
        filenameTemp = argv[1];
    }

    const char *filename = filenameTemp;
    const char *filename2 = argv[2];

    // for (int i = 0; i < numberOfPartitions; i += numProcs)
    //{
    //  const char *filename = (string(argv[1]) + "/" + to_string(rank + i)).c_str();
    //  const char *filename2 = (string(argv[2]) + "/" + to_string(rank + i)).c_str();
    //  cout << "File: " << (string(argv[1]) + "/" + to_string(rank + i)).c_str() << endl;
    //  cout << "File2: " << (string(argv[2]) + "/" + to_string(rank + i)).c_str() << endl;
    try
    {
        // vector<GEOSGeometry *> *geoms = get_polygons((string(argv[1]) + "/" + to_string(rank + i)).c_str());
        string filenameTempSuffix = rank < 10 ? "00" : rank < 100 ? "0"
                                                                  : "";
        vector<GEOSGeometry *> *geoms = get_polygons((argv[1] + string("_") + to_string(numProcs) + string("_") + filenameTempSuffix + to_string(rank)).c_str());
        // vector<GEOSGeometry *> *geoms2 = get_polygons((string(argv[2]) + "/" + to_string(rank + i)).c_str());
        if (geoms->size() > 0)
        {
            vector<GEOSGeometry *> *geoms2 = get_polygons(argv[2]);
            // create_time += select_test("Create", &create_tree, geoms, geoms2, n);
            // iterate_time += select_test("Iterate", &iterate_tree, geoms, geoms2, n);
            // query_time += select_test("Query", &query, geoms, geoms2, n);
            intersect_time += select_test("Intersect", &intersect, geoms, geoms2, n);
            // overlap_time += select_test("Overlap", &overlap, geoms, geoms2, n);
            // touch_time += select_test("Touch", &touch, geoms, geoms2, n);
            // cross_time += select_test("Cross", &cross, geoms, geoms2, n);
            // contain_time += select_test("Contain", &contain, geoms, geoms2, n);
            // equal_time += select_test("Equal", &equal, geoms, geoms2, n);
            // equal_exact_time += select_test("Equal Exact (0.3)", &equal_exact, geoms, geoms2, n);
            // cover_time += select_test("Cover", &cover, geoms, geoms2, n);
            // covered_by_time += select_test("Covered By", &covered_by, geoms, geoms2, n);

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
    //}

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

    double test_time_arr_max[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    double test_time_arr_sum[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    double test_time_arr[13] = {create_time, iterate_time, query_time, intersect_time, overlap_time, touch_time,
                                cross_time, contain_time, equal_time, equal_exact_time, cover_time, covered_by_time,
                                create_time + iterate_time + query_time + intersect_time + overlap_time + touch_time + cross_time + contain_time + equal_time + equal_exact_time + cover_time + covered_by_time};

    MPI_Reduce(test_time_arr, test_time_arr_max, 13, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(test_time_arr, test_time_arr_sum, 13, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        if (numProcs > 1)
        {
            //system("rm ./file_part_*");
            system((string("rm ") + argv[1] + string("_") + to_string(numProcs) + string("_*")).c_str());
        }
        cout << endl
             << endl
             << "------------------------------------------------------------------" << endl
             << "-------------------- BENCHMARK RESULT (MAX) --------------------" << endl
             << "------------------------------------------------------------------" << endl
             << argv[1] << " - " << argv[2] << " - " << numProcs << endl
             //  << "Max Create Time: " << test_time_arr_max[0] << endl
             //  << "Max Iterate Time: " << test_time_arr_max[1] << endl
             //  << "Max Query Time: " << test_time_arr_max[2] << endl
             << "Max Intersect Time: " << test_time_arr_max[3] << endl
             //  << "Max Overlap Time: " << test_time_arr_max[4] << endl
             //  << "Max Touch Time: " << test_time_arr_max[5] << endl
             //  << "Max Cross Time: " << test_time_arr_max[6] << endl
             //  << "Max Contain Time: " << test_time_arr_max[7] << endl
             //  << "Max Equal Time: " << test_time_arr_max[8] << endl
             //  << "Max Equal Exact (0.3) Time: " << test_time_arr_max[9] << endl
             //  << "Max Cover Time: " << test_time_arr_max[10] << endl
             //  << "Max Covered By Time: " << test_time_arr_max[11] << endl
             << "Max TOTAL: " << test_time_arr_max[12] << endl
             << "------------------------------------------------------------------" << endl
             << "------------------------------------------------------------------" << endl
            //  << endl
            //  << endl
            //  << endl
            //  << endl
            //  << "------------------------------------------------------------------" << endl
            //  << "-------------------- BENCHMARK RESULT (AVG) --------------------" << endl
            //  << "------------------------------------------------------------------" << endl
            //  << "Average Create Time: " << test_time_arr_sum[0] / numProcs << endl
            //  << "Average Iterate Time: " << test_time_arr_sum[1] / numProcs << endl
            //  << "Average Query Time: " << test_time_arr_sum[2] / numProcs << endl
            //  << "Average Intersect Time: " << test_time_arr_sum[3] / numProcs << endl
            //  << "Average Overlap Time: " << test_time_arr_sum[4] / numProcs << endl
            //  << "Average Touch Time: " << test_time_arr_sum[5] / numProcs << endl
            //  << "Average Cross Time: " << test_time_arr_sum[6] / numProcs << endl
            //  << "Average Contain Time: " << test_time_arr_sum[7] / numProcs << endl
            //  << "Average Equal Time: " << test_time_arr_sum[8] / numProcs << endl
            //  << "Average Equal Exact (0.3) Time: " << test_time_arr_sum[9] / numProcs << endl
            //  << "Average Cover Time: " << test_time_arr_sum[10] / numProcs << endl
            //  << "Average Covered By Time: " << test_time_arr_sum[11] / numProcs << endl
            //  << "Average TOTAL: " << test_time_arr_sum[12] / numProcs << endl
            //  << "------------------------------------------------------------------" << endl
            //  << "------------------------------------------------------------------" << endl
            //  << endl
            //  << endl
            ;
    }

    finishGEOS();
    MPI_Finalize();
    return 0;
}