/*
 * # GEOS C API example 3
 *
 * Build a spatial index and search it for a
 * nearest pair.
 */

/* To print to stdout */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
/* Only the CAPI header is required */
#include <geos_c.h>

#include <string.h>
#include <time.h>

#define MAX_LINE_LENGTH 10000

/*
 * GEOS requires two message handlers to return
 * error and notice message to the calling program.
 *
 *   typedef void(* GEOSMessageHandler) (const char *fmt,...)
 *
 * Here we stub out an example that just prints the
 * messages to stdout.
 */
static void
geos_message_handler(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

/*
 * Generate a random point in the range of
 * POINT(0..range, 0..range). Caller must
 * free.
 */
static GEOSGeometry *
geos_random_point(double range)
{
    double x = range * rand() / RAND_MAX;
    double y = range * rand() / RAND_MAX;
    /* Make a point in the point grid */
    return GEOSGeom_createPointFromXY(x, y);
}

int main(int argc, char **argv)
{
    /* Send notice and error messages to our stdout handler */
    initGEOS(geos_message_handler, geos_message_handler);

    /* How many points to add to our random field */
    const size_t npoints = 10000;
    /* The coordinate range of the field (0->100.0) */
    const double range = 1.0;

    /*
     * The tree doesn't take ownership of inputs just
     * holds references, so we keep our point field
     * handy in an array
     */
    int n = 1;
    if (argc > 2)
        n = atoi(argv[2]);
    if (!n)
        n = 1;

    double time_arr[n];
    for (int ith_run = 0; ith_run < n; ith_run++)
    {
        GEOSGeometry *geoms[npoints];
        /*
         * The create parameter for the tree is not the
         * number of inputs, it is the number of entries
         * per node. 10 is a good default number to use.
         */
        GEOSSTRtree *tree = GEOSSTRtree_create(10);
        // for (size_t i = 0; i < npoints; i++) {
        //     /* Make a random point */
        //     GEOSGeometry* geom = geos_random_point(range);
        //     /* Store away a reference so we can free it after */
        //     geoms[i] = geom;
        //     /* Add an entry for it to the tree */
        //     GEOSSTRtree_insert(tree, geom, geom);
        // }

        /// Creating rtree from file ///
        FILE *input;
        input = fopen(argv[1], "r");
        char line[MAX_LINE_LENGTH];
        int index = 0;
        while (fgets(line, MAX_LINE_LENGTH, input))
        {
            if (strlen(line) > 5)
            {
                GEOSGeometry *geom = GEOSGeomFromWKT(line);
                geoms[index] = geom;
                GEOSSTRtree_insert(tree, geom, geom);
                index++;
            }
        }

        clock_t t;
        t = clock();
        for (int i = 0; i < 1000; i++)
        {
            /* Convert results to WKT */
            GEOSWKTWriter *writer = GEOSWKTWriter_create();
            /* Trim trailing zeros off output */
            GEOSWKTWriter_setTrim(writer, 1);
            GEOSWKTWriter_setRoundingPrecision(writer, 3);

            /* Random point to compare to the field */
            GEOSGeometry *geom_random = geos_random_point(range);
            /* Nearest point in the field to our test point */
            const GEOSGeometry *geom_nearest = GEOSSTRtree_nearest(tree, geom_random);
            char *wkt_random = GEOSWKTWriter_write(writer, geom_random);
            char *wkt_nearest = GEOSWKTWriter_write(writer, geom_nearest);
            GEOSWKTWriter_destroy(writer);

            /* Print answer */
            printf(" Random Point: %s\n", wkt_random);
            printf("Nearest Point: %s\n", wkt_nearest);

            GEOSGeom_destroy(geom_random);
            /*
             * Don't forget to free memory allocated by the
             * printing functions!
             */
            GEOSFree(wkt_random);
            GEOSFree(wkt_nearest);
        }

        t = clock() - t;
        double time_taken = ((double)t) / CLOCKS_PER_SEC; // calculate the elapsed time
        time_arr[ith_run] = time_taken;

        /* Clean up all allocated objects */
        /* Destroying tree does not destroy inputs */
        GEOSSTRtree_destroy(tree);
        /* Destroy all the points in our random field */
        for (size_t i = 0; i < index; i++)
        {
            GEOSGeom_destroy(geoms[i]);
        }
    }

    /* Clean up the global context */
    finishGEOS();

    for (int i = 0; i < n; i++)
    {
        printf("The program %d took %f seconds to execute\n", i, time_arr[i]);
    }

    /* Done */
    return 0;
}

int strtree_insert(char *file_name, int num_of_iterations)
{
    initGEOS(geos_message_handler, geos_message_handler);

    const size_t npoints = 10000;
    const double range = 1.0;

    // int n=1;
    // if ( argc > 2 ) n=atoi(argv[2]);
    // if ( ! n ) n=1;

    double time_arr[n];
    for (int ith_run = 0; ith_run < num_of_iterations; ith_run++)
    {

        clock_t t;
        t = clock();
        for (int i = 0; i < 1000; i++)
        {
            GEOSGeometry *geoms[npoints];

            GEOSSTRtree *tree = GEOSSTRtree_create(10);

            FILE *input;
            input = fopen(file_name, "r");
            char line[MAX_LINE_LENGTH];
            int index = 0;
            while (fgets(line, MAX_LINE_LENGTH, input))
            {
                if (strlen(line) > 5)
                {
                    GEOSGeometry *geom = GEOSGeomFromWKT(line);
                    geoms[index] = geom;
                    GEOSSTRtree_insert(tree, geom, geom);
                    index++;
                }
            }
        }

        t = clock() - t;
        double time_taken = ((double)t) / CLOCKS_PER_SEC;
        time_arr[ith_run] = time_taken;

        GEOSSTRtree_destroy(tree);
        for (size_t i = 0; i < index; i++)
        {
            GEOSGeom_destroy(geoms[i]);
        }
    }
    finishGEOS();

    printf("-------- Tree Insert --------");
    for (int i = 0; i < n; i++)
    {
        printf("The program %d took %f seconds to execute\n", i, time_arr[i]);
    }
    printf("-----------------------------");
    return 0;
}

void queryCallback(void *geom, void *userdata)
{
    GEOSWKTWriter *writer = GEOSWKTWriter_create();
    GEOSWKTWriter_setTrim(writer, 1);
    GEOSWKTWriter_setRoundingPrecision(writer, 3);
    char *wkt_geom = GEOSWKTWriter_write(writer, geom);

    GEOSWKTWriter_destroy(writer);
    printf("Geom: %s\n", wkt_geom);
}

int strtree_query(char *file_name, int num_of_iterations)
{
    initGEOS(geos_message_handler, geos_message_handler);

    const size_t npoints = 10000;
    const double range = 1.0;

    // int n=1;
    // if ( argc > 2 ) n=atoi(argv[2]);
    // if ( ! n ) n=1;

    double time_arr[n];
    for (int ith_run = 0; ith_run < num_of_iterations; ith_run++)
    {
        GEOSGeometry *geoms[npoints];

        GEOSSTRtree *tree = GEOSSTRtree_create(10);

        FILE *input;
        input = fopen(file_name, "r");
        char line[MAX_LINE_LENGTH];
        int index = 0;
        while (fgets(line, MAX_LINE_LENGTH, input))
        {
            if (strlen(line) > 5)
            {
                GEOSGeometry *geom = GEOSGeomFromWKT(line);
                geoms[index] = geom;
                GEOSSTRtree_insert(tree, geom, geom);
                index++;
            }
        }

        clock_t t;
        t = clock();
        for (int i = 0; i < 1000; i++)
        {
            GEOSWKTWriter *writer = GEOSWKTWriter_create();
            GEOSWKTWriter_setTrim(writer, 1);
            GEOSWKTWriter_setRoundingPrecision(writer, 3);

            // GEOSGeometry *geom_random = geos_random_point(range);
            const GEOSGeometry *geom_query = GEOSSTRtree_query(tree, geoms[i], queryCallback); // todo: add callback function
            // char *wkt_random = GEOSWKTWriter_write(writer, geom_random);
            char *wkt_query = GEOSWKTWriter_write(writer, geom_query);
            GEOSWKTWriter_destroy(writer);

            // printf(" Random Point: %s\n", wkt_random);
            printf("Nearest Point: %s\n", wkt_query);

            // GEOSGeom_destroy(geom_random);
            // GEOSFree(wkt_random);
            GEOSFree(wkt_query);
        }

        t = clock() - t;
        double time_taken = ((double)t) / CLOCKS_PER_SEC;
        time_arr[ith_run] = time_taken;

        GEOSSTRtree_destroy(tree);
        for (size_t i = 0; i < index; i++)
        {
            GEOSGeom_destroy(geoms[i]);
        }
    }
    finishGEOS();

    printf("-------- Geom Query --------");
    for (int i = 0; i < n; i++)
    {
        printf("The program %d took %f seconds to execute\n", i, time_arr[i]);
    }
    printf("----------------------------");
    return 0;
}

void iterationCallback(void *geom, void *userdata)
{
    GEOSWKTWriter *writer = GEOSWKTWriter_create();
    GEOSWKTWriter_setTrim(writer, 1);
    GEOSWKTWriter_setRoundingPrecision(writer, 3);
    char *wkt_geom = GEOSWKTWriter_write(writer, geom);

    GEOSWKTWriter_destroy(writer);
    printf("Geom: %s\n", wkt_geom);
}

int strtree_iterate(char *file_name, int num_of_iterations)
{
    initGEOS(geos_message_handler, geos_message_handler);

    const size_t npoints = 10000;
    const double range = 1.0;

    // int n=1;
    // if ( argc > 2 ) n=atoi(argv[2]);
    // if ( ! n ) n=1;

    double time_arr[n];
    for (int ith_run = 0; ith_run < num_of_iterations; ith_run++)
    {
        GEOSGeometry *geoms[npoints];

        GEOSSTRtree *tree = GEOSSTRtree_create(10);

        FILE *input;
        input = fopen(file_name, "r");
        char line[MAX_LINE_LENGTH];
        int index = 0;
        while (fgets(line, MAX_LINE_LENGTH, input))
        {
            if (strlen(line) > 5)
            {
                GEOSGeometry *geom = GEOSGeomFromWKT(line);
                geoms[index] = geom;
                GEOSSTRtree_insert(tree, geom, geom);
                index++;
            }
        }

        clock_t t;
        t = clock();
        for (int i = 0; i < 1000; i++)
        {
            GEOSWKTWriter *writer = GEOSWKTWriter_create();
            GEOSWKTWriter_setTrim(writer, 1);
            GEOSWKTWriter_setRoundingPrecision(writer, 3);

            GEOSSTRtree_iterate(tree, geos[i], iterationCallback); // todo: add callback function

            GEOSWKTWriter_destroy(writer);
        }

        t = clock() - t;
        double time_taken = ((double)t) / CLOCKS_PER_SEC;
        time_arr[ith_run] = time_taken;

        GEOSSTRtree_destroy(tree);
        for (size_t i = 0; i < index; i++)
        {
            GEOSGeom_destroy(geoms[i]);
        }
    }
    finishGEOS();

    printf("-------- Tree Iteration --------");
    for (int i = 0; i < n; i++)
    {
        printf("The program %d took %f seconds to execute\n", i, time_arr[i]);
    }
    printf("--------------------------------");
    return 0;
}

int find_nearest_point(char *file_name, int num_of_iterations)
{
    initGEOS(geos_message_handler, geos_message_handler);

    const size_t npoints = 10000;
    const double range = 1.0;

    // int n=1;
    // if ( argc > 2 ) n=atoi(argv[2]);
    // if ( ! n ) n=1;

    double time_arr[n];
    for (int ith_run = 0; ith_run < num_of_iterations; ith_run++)
    {
        GEOSGeometry *geoms[npoints];

        GEOSSTRtree *tree = GEOSSTRtree_create(10);

        FILE *input;
        input = fopen(file_name, "r");
        char line[MAX_LINE_LENGTH];
        int index = 0;
        while (fgets(line, MAX_LINE_LENGTH, input))
        {
            if (strlen(line) > 5)
            {
                GEOSGeometry *geom = GEOSGeomFromWKT(line);
                geoms[index] = geom;
                GEOSSTRtree_insert(tree, geom, geom);
                index++;
            }
        }

        clock_t t;
        t = clock();
        for (int i = 0; i < 1000; i++)
        {
            GEOSWKTWriter *writer = GEOSWKTWriter_create();
            GEOSWKTWriter_setTrim(writer, 1);
            GEOSWKTWriter_setRoundingPrecision(writer, 3);

            GEOSGeometry *geom_random = geos_random_point(range);
            const GEOSGeometry *geom_nearest = GEOSSTRtree_nearest(tree, geom_random);
            char *wkt_random = GEOSWKTWriter_write(writer, geom_random);
            char *wkt_nearest = GEOSWKTWriter_write(writer, geom_nearest);
            GEOSWKTWriter_destroy(writer);

            printf(" Random Point: %s\n", wkt_random);
            printf("Nearest Point: %s\n", wkt_nearest);

            GEOSGeom_destroy(geom_random);
            GEOSFree(wkt_random);
            GEOSFree(wkt_nearest);
        }

        t = clock() - t;
        double time_taken = ((double)t) / CLOCKS_PER_SEC;
        time_arr[ith_run] = time_taken;

        GEOSSTRtree_destroy(tree);
        for (size_t i = 0; i < index; i++)
        {
            GEOSGeom_destroy(geoms[i]);
        }
    }
    finishGEOS();

    printf("-------- Nearest Point --------");
    for (int i = 0; i < n; i++)
    {
        printf("The program %d took %f seconds to execute\n", i, time_arr[i]);
    }
    printf("-------------------------------");
    return 0;
}

int find_intersect(char *file_name, int num_of_iterations)
{
    initGEOS(geos_message_handler, geos_message_handler);

    const size_t npoints = 10000;
    const double range = 1.0;

    // int n=1;
    // if ( argc > 2 ) n=atoi(argv[2]);
    // if ( ! n ) n=1;

    double time_arr[n];
    for (int ith_run = 0; ith_run < num_of_iterations; ith_run++)
    {
        GEOSGeometry *geoms[npoints];

        GEOSSTRtree *tree = GEOSSTRtree_create(10);

        FILE *input;
        input = fopen(file_name, "r");
        char line[MAX_LINE_LENGTH];
        int index = 0;
        while (fgets(line, MAX_LINE_LENGTH, input))
        {
            if (strlen(line) > 5)
            {
                GEOSGeometry *geom = GEOSGeomFromWKT(line);
                geoms[index] = geom;
                GEOSSTRtree_insert(tree, geom, geom);
                index++;
            }
        }

        clock_t t;
        t = clock();
        for (int i = 0; i < 1000; i++)
        {
            GEOSWKTWriter *writer = GEOSWKTWriter_create();
            GEOSWKTWriter_setTrim(writer, 1);
            GEOSWKTWriter_setRoundingPrecision(writer, 3);

            GEOSGeometry *geom_random = geos_random_point(range);

            for (int j = 0; j < 1000; j++)
            {

                if (GEOSIntersects(geoms[j], geom_random))
                {
                    char *wkt_random = GEOSWKTWriter_write(writer, geom_random);
                    char *wkt_intersect = GEOSWKTWriter_write(writer, geoms[j]);
                    printf(" Random Point: %s\n", wkt_random);
                    printf("Intersecting Point: %s\n", wkt_intersect);
                }
                else
                {
                    GEOSGeom_destroy(geom_random);
                }

                GEOSGeom_destroy(geom_random);
                GEOSFree(wkt_random);
                GEOSFree(wkt_intersect);
            }
        }

        t = clock() - t;
        double time_taken = ((double)t) / CLOCKS_PER_SEC;
        time_arr[ith_run] = time_taken;

        GEOSSTRtree_destroy(tree);
        for (size_t i = 0; i < index; i++)
        {
            GEOSGeom_destroy(geoms[i]);
        }
    }
    finishGEOS();

    printf("-------- Intersect --------");
    for (int i = 0; i < n; i++)
    {
        printf("The program %d took %f seconds to execute\n", i, time_arr[i]);
    }
    printf("---------------------------");
    return 0;
}

int find_overlap(char *file_name, int num_of_iterations)
{
    initGEOS(geos_message_handler, geos_message_handler);

    const size_t npoints = 10000;
    const double range = 1.0;

    // int n=1;
    // if ( argc > 2 ) n=atoi(argv[2]);
    // if ( ! n ) n=1;

    double time_arr[n];
    for (int ith_run = 0; ith_run < num_of_iterations; ith_run++)
    {
        GEOSGeometry *geoms[npoints];

        GEOSSTRtree *tree = GEOSSTRtree_create(10);

        FILE *input;
        input = fopen(file_name, "r");
        char line[MAX_LINE_LENGTH];
        int index = 0;
        while (fgets(line, MAX_LINE_LENGTH, input))
        {
            if (strlen(line) > 5)
            {
                GEOSGeometry *geom = GEOSGeomFromWKT(line);
                geoms[index] = geom;
                GEOSSTRtree_insert(tree, geom, geom);
                index++;
            }
        }

        clock_t t;
        t = clock();
        for (int i = 0; i < 1000; i++)
        {
            GEOSWKTWriter *writer = GEOSWKTWriter_create();
            GEOSWKTWriter_setTrim(writer, 1);
            GEOSWKTWriter_setRoundingPrecision(writer, 3);

            GEOSGeometry *geom_random = geos_random_point(range);

            for (int j = 0; j < 1000; j++)
            {

                if (GEOSOverlaps(geoms[j], geom_random))
                {
                    char *wkt_random = GEOSWKTWriter_write(writer, geom_random);
                    char *wkt_intersect = GEOSWKTWriter_write(writer, geoms[j]);
                    printf(" Random Point: %s\n", wkt_random);
                    printf("Intersecting Point: %s\n", wkt_intersect);
                }
                else
                {
                    GEOSGeom_destroy(geom_random);
                }

                GEOSGeom_destroy(geom_random);
                GEOSFree(wkt_random);
                GEOSFree(wkt_intersect);
            }
        }

        t = clock() - t;
        double time_taken = ((double)t) / CLOCKS_PER_SEC;
        time_arr[ith_run] = time_taken;

        GEOSSTRtree_destroy(tree);
        for (size_t i = 0; i < index; i++)
        {
            GEOSGeom_destroy(geoms[i]);
        }
    }
    finishGEOS();

    printf("-------- Overlap --------");
    for (int i = 0; i < n; i++)
    {
        printf("The program %d took %f seconds to execute\n", i, time_arr[i]);
    }
    printf("-------------------------");
    return 0;
}