/*
  METANOTES by Nathan Tibbetts, Oct 2024:
  - I corrected this version to separate
    GEOS contexts between threads rather than sharing one, as per their proper
    use in the GEOS documentation.
  - Still incorrect: appears to have an incomplete async implementation in the
    calldata class, which means we have three trees, and that's not right.
  - Current work is in the spatial_operator project folder.
*/

#include <geos_c.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

#ifdef BAZEL_BUILD
#include "../protos/bluefield_grpc.grpc.pb.h"
#else
#include "bluefield_grpc.grpc.pb.h"
#endif

using bluefield_grpc::Greeter;
using bluefield_grpc::HelloReply;
using bluefield_grpc::HelloRequest;
using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;

using namespace std;


double total_intersect_time = 0;
// int start_test(std::string file1, std::string file2, GEOSContextHandle_t ctx);

struct timeval tv1, tv2;
vector<double> elapsedTimes;
int intersectCount = 0;



static void geos_message_handler(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
}

static GEOSContextHandle_t init_geos()
{ // Simple function to run some GEOS initialization, to be called in each
  //   thread, including root (reentrant version)
  GEOSContextHandle_t ctx = GEOS_init_r();
  GEOSContext_setNoticeHandler_r(ctx, geos_message_handler);
  GEOSContext_setErrorHandler_r(ctx, geos_message_handler);
  return ctx;
}

// static void cleanup_geos(GEOSContextHandle_t &ctx)
// { // Clean up GEOS, to be called in each thread & root (reentrant version)
//   GEOS_finish_r(ctx);
// }



map<std::string, GEOSGeometry *> get_polygon_map(const char *filename,
                                                 GEOSContextHandle_t ctx)
{
  map<std::string, GEOSGeometry *> geom_map;
  std::ifstream input(filename);
  for (std::string line; getline(input, line);)
  {
    if (line.length() > 5 && line.find("("))
    {
      string id = line.substr(0, line.find(" - "));
      string geom_string = line.substr(line.find(" - ") + 3);
      // cout << geom_string << endl;
      GEOSGeometry *geom = GEOSGeomFromWKT_r(ctx, geom_string.c_str());
      geom_map[id] = geom;
    }
  }
  return geom_map;
}

map<std::string, GEOSGeometry *> get_polygon_map_without_id(
    const char *filename, GEOSContextHandle_t ctx)
{
  map<std::string, GEOSGeometry *> geom_map;
  std::ifstream input(filename);
  int ai_id = 0;
  for (std::string line; getline(input, line);)
  {
    if (line.length() > 5 && line.find("("))
    {
      string geom_string = line;
      // cout << geom_string << endl;
      GEOSGeometry *geom = GEOSGeomFromWKT_r(ctx, geom_string.c_str());
      geom_map[to_string(ai_id)] = geom;
      ai_id++;
    }
  }
  return geom_map;
}

struct CallbackData
{
  GEOSContextHandle_t* ctx;
  GEOSGeometry* geom;
};

void intersect_callback(void* matching_geom, void* callback_data) {
  // GEOSWKTWriter* writer = GEOSWKTWriter_create_r(ctx);
  // GEOSWKTWriter_setTrim_r(ctx, writer, 1);
  // GEOSWKTWriter_setRoundingPrecision_r(ctx, writer, 3);
  // char* wkt_geom = GEOSWKTWriter_write_r(
  //     ctx, writer, static_cast<const GEOSGeometry*>(geom));
  // char* wkt_base_geom = GEOSWKTWriter_write_r(
  //     ctx, writer, static_cast<const GEOSGeometry*>(base_geom));
  // GEOSWKTWriter_destroy_r(ctx, writer);
  // cout << "Intersect candidate: " << wkt_base_geom << " - " << wkt_geom <<
  // endl;

  // intersectCandidateQueue.push(string(wkt_base_geom) + " - " +
  //                              string(wkt_geom));
  // cout << "Intersect candidate..." << endl;

  CallbackData inputs = *static_cast<CallbackData*>(callback_data);
  if (GEOSIntersects_r(*(inputs.ctx), static_cast<const GEOSGeometry*>(matching_geom),
                       inputs.geom)) {
    cout << "Intersection found!" << endl;
    intersectCount++;
    if (intersectCount % 100 == 0)
    {
      gettimeofday(&tv2, NULL);
      double time_spent = (double)(tv2.tv_usec - tv1.tv_usec) / 1000000 +
                          (double)(tv2.tv_sec - tv1.tv_sec);
      elapsedTimes.push_back(time_spent);
    }
    // cout << "Intersect: " << wkt_base_geom << " - " << wkt_geom << endl;
  }
}

// Reentrant-prepared function for a threaded call
void non_blocking_rtree_intersect_thread(
          map<std::string, GEOSGeometry *> geoms, 
          GEOSSTRtree *tree1,
          GEOSSTRtree *tree2)
{
  // Initialize GEOS for this thread
  GEOSContextHandle_t ctx = init_geos();

  for (auto const &cur : geoms)
  {
    GEOSGeometry *geom = cur.second;
    CallbackData inputs = { &ctx, geom };
    GEOSSTRtree_insert_r(ctx, tree1, geom, GEOSEnvelope_r(ctx, geom));
    GEOSSTRtree_query_r(ctx, tree2, geom, intersect_callback, &inputs);
  }

  // Clean up GEOS for this thread
  GEOS_finish_r(ctx);
}

// string intersectsRefinement(string candidatePair) {
//   string base_geom_string = candidatePair.substr(0, candidatePair.find(" -
//   ")); string query_geom_string =
//       candidatePair.substr(candidatePair.find(" - ") + 3);
//   GEOSGeometry* base_geom = GEOSGeomFromWKT_r(ctx, base_geom_string.c_str());
//   GEOSGeometry* query_geom = GEOSGeomFromWKT_r(ctx,
//   query_geom_string.c_str()); if (!GEOSIntersects_r(ctx, static_cast<const
//   GEOSGeometry*>(base_geom),
//                         static_cast<const GEOSGeometry*>(query_geom))) {
//     return "Intersect: " + candidatePair;
//   } else {
//     return " No Intersect: " + candidatePair;
//   }
// }

class ServerImpl final
{
public:
  ServerImpl() // Constructor
  {
    ctx_0 = init_geos(); // The GEOS context for the root thread
    // queue<std::string> intersectCandidateQueue;
  }

  ~ServerImpl() // Destructor
  {
    finishGEOS_r(ctx_0);

    server_->Shutdown();
    // Always shutdown the completion queue after the server.
    cq_->Shutdown();
  }

  // There is no shutdown handling in this code.
  void Run()
  {
    std::string server_address("0.0.0.0:50051");
    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service_" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *asynchronous* service.
    builder.RegisterService(&service_);
    // Get hold of the completion queue used for the asynchronous communication
    // with the gRPC runtime.
    cq_ = builder.AddCompletionQueue();
    // Finally assemble the server.
    server_ = builder.BuildAndStart();
    std::cout << "Server listening on " << server_address << std::endl;

    // Proceed to the server's main loop.
    // HandleRpcs();

    //////

    cout << "GEOS version: %s\n"
         << GEOSversion() << endl;

    // const char* filename = "/home/dpu/data/cemet_data_indexed";
    // const char* filename = "/home/dpu/data/cemet_data_indexed";
    const char *fileBase =
        // "/global/scratch/users/satishp/data/lakes_data_indexed";
        "../../Data/no_partition/cemetery.wkt_indexed";
    const char *fileQuery =
        // "/global/scratch/users/satishp/data/sports_data_indexed";
        "../../Data/no_partition/sports.wkt_indexed";

    map<std::string, GEOSGeometry *> geomsBase = get_polygon_map(fileBase, ctx_0);
    map<std::string, GEOSGeometry *> geomsQuery = get_polygon_map(fileQuery, ctx_0);
    // const char* filename = "/home/dpu/data/lakes_data";
    // map<std::string, GEOSGeometry*> geoms =
    // get_polygon_map_without_id(filename, ctx);

    ///////////////////////////////////////////////////////////////////////////
    // struct timeval tv1, tv2;
    // gettimeofday(&tv1, NULL);

    // tree = GEOSSTRtree_create_r(ctx, 10);

    // for (auto const& cur : geomsBase) {
    //   GEOSGeometry* geom = cur.second;
    //   GEOSSTRtree_insert_r(ctx, tree, geom, GEOSEnvelope_r(ctx, geom));
    // }

    // cout << "rtree created" << endl;

    // for (auto const& cur : geomsQuery) {
    //   GEOSGeometry* geom = cur.second;
    //   GEOSSTRtree_query_r(ctx, tree, geom, intersect_callback, geom);
    // }

    // cout << "intersectCount: " << intersectCount << endl;

    // gettimeofday(&tv2, NULL);
    // double time_spent = (double)(tv2.tv_usec - tv1.tv_usec) / 1000000 +
    //                     (double)(tv2.tv_sec - tv1.tv_sec);
    // cout << endl << endl << "Total Time:" << time_spent << endl;
    ///////////////////////////////////////////////////////////////////////////

    // struct timeval tv1, tv2;
    // gettimeofday(&tv1, NULL);

    // treeBase = GEOSSTRtree_create_r(ctx, 10);
    // treeQuery = GEOSSTRtree_create_r(ctx, 10);

    // std::thread thread1(non_blocking_rtree_intersect, geomsBase, treeQuery,
    //                     ctx);
    // std::thread thread2(non_blocking_rtree_intersect, geomsQuery, treeBase,
    //                     ctx);

    // thread1.join();
    // thread2.join();

    // cout << "intersectCount: " << intersectCount << endl;

    // gettimeofday(&tv2, NULL);
    // double time_spent = (double)(tv2.tv_usec - tv1.tv_usec) / 1000000 +
    //                     (double)(tv2.tv_sec - tv1.tv_sec);
    // cout << endl << endl << "Total Time:" << time_spent << endl;
    ///////////////////////////////////////////////////////////////////////////

    gettimeofday(&tv1, NULL);

    // GEOSSTRtree *tree;
    GEOSSTRtree *treeBase;
    GEOSSTRtree *treeQuery;
    treeBase = GEOSSTRtree_create_r(ctx_0, 10); // 2nd param is node capacity. Defaults to 10.
    treeQuery = GEOSSTRtree_create_r(ctx_0, 10);

    std::thread thread1(
        non_blocking_rtree_intersect_thread, geomsBase, treeBase, treeQuery);
    std::thread thread2(
        non_blocking_rtree_intersect_thread, geomsQuery, treeQuery, treeBase);

    thread1.join();
    thread2.join();

    ///////////////////////////////////////////////////////////////////////////

    // GEOSGeometry* geom = GEOSGeomFromWKT_r(ctx, geomString.c_str());

    // std::thread thread1(HandleRpcsHelper, 1, this);
    // std::thread thread2(HandleRpcsHelper, 2, this);
    // std::thread thread3(HandleRpcsHelper, 3, this);
    // std::thread thread4(HandleRpcsHelper, 4, this);
    // std::thread thread5(HandleRpcsHelper, 5, this);
    // std::thread thread6(HandleRpcsHelper, 6, this);
    // std::thread thread7(HandleRpcsHelper, 7, this);
    // std::thread thread8(HandleRpcsHelper, 8, this);
    // // HandleRpcs();
    // thread1.join();
    // thread2.join();
    // thread3.join();
    // thread4.join();
    // thread5.join();
    // thread6.join();
    // thread7.join();
    // thread8.join();
  }

private:
  // Class encompasing the state and logic needed to serve a request.
  class CallData
  {
  public:
    // Take in the "service" instance (in this case representing an asynchronous
    // server) and the completion queue "cq" used for asynchronous communication
    // with the gRPC runtime.
    CallData(Greeter::AsyncService *service, ServerCompletionQueue *cq, GEOSContextHandle_t ctx_geos)
        : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE)
    {
      // Invoke the serving logic right away.
      Proceed();
    }

    void Proceed()
    {
      if (status_ == CREATE)
      {
        // Make this instance progress to the PROCESS state.
        status_ = PROCESS;

        // As part of the initial CREATE state, we *request* that the system
        // start processing SayHello requests. In this request, "this" acts are
        // the tag uniquely identifying the request (so that different CallData
        // instances can serve different requests concurrently), in this case
        // the memory address of this CallData instance.
        service_->RequestSayHello(&ctx_, &request_, &responder_, cq_, cq_,
                                  this);
      }
      else if (status_ == PROCESS)
      {
        // Spawn a new CallData instance to serve new clients while we process
        // the one for this CallData. The instance will deallocate itself as
        // part of its FINISH state.
        new CallData(service_, cq_, ctx_geos);

        // The actual processing.

        // cout << "Processing " << request_.name() << "..." << endl;
        cout << "Processing..." << endl;

        // string queryString = request_.name();
        // vector<string> geomStrings;
        // int startIndex = 0, endIndex = 0;
        // for (int i = 0; i <= queryString.size(); i++) {
        //   if (queryString[i] == '\n' || i == queryString.size()) {
        //     endIndex = i;
        //     string temp;
        //     temp.append(queryString, startIndex, endIndex - startIndex);
        //     geomStrings.push_back(temp);
        //     startIndex = endIndex + 1;
        //   }
        // }

        // for (auto cur = geomStrings.begin(); cur != geomStrings.end(); ++cur)
        // {
        //   string geomString = *cur;
        //   if (geomString.length() > 5 && geomString.find("(")) {
        //     GEOSGeometry* geom = GEOSGeomFromWKT_r(ctx, geomString.c_str());
        //     GEOSSTRtree_query_r(ctx, tree, geom, intersect_callback, geom);
        //   }
        //   // cout << "extracted:" << geomString << endl;
        // }

        // cout << "Geom point " << request_.geom()[0] << "..." << endl;
        vector<float> polygonArray(request_.geom().begin(),
                                   request_.geom().end());
        // cout << "Size:" << polygonArray.size() << endl;
        //  for (int i = 0; i < polygonArray.size(); i++) {
        //    cout << polygonArray[i] << endl;
        //  }
        if (polygonArray.size() > 0)
        {
          int geomSize = polygonArray[0];
          // cout << "GeomSize:" << geomSize << endl;

          for (int i = 1; i < polygonArray.size(); i++)
          {
            string geomString = "POLYGON((";
            for (int j = i; j <= geomSize; j += 2)
            {
              i += 2;
              geomString = geomString + to_string(polygonArray[j]) + " " +
                           to_string(polygonArray[j + 1]);
              if (i < geomSize)
              {
                geomString = geomString + ",";
              }
            }
            geomString = geomString + "))";
            geomSize = polygonArray[i];
            
            GEOSGeometry *geom = GEOSGeomFromWKT_r(ctx_geos, geomString.c_str());
            CallbackData inputs = { &ctx_geos, geom };
            cout << "DOING Proceed FUNCTION" << endl; // TODO: remove
            GEOSSTRtree_query_r(ctx_geos, tree, geom, intersect_callback, &inputs); // TODO: should the tree pointer being passed in be blank? It is. // <-----------------------------------------------<<< ?
            // cout << "GeomSize:" << geomSize << endl;

            // cout << "i:" << i << endl;

            // cout << geomString << endl;
          }
        }

        // string geomString = "POLYGON((";
        // int numCoords = 10;

        // for (int i = 0; i < numCoords; i += 2) {
        //   geomString = geomString + to_string(request_.geom()[i]) + " " +
        //                to_string(request_.geom()[i + 1]);
        //   if (i < numCoords - 2) {
        //     geomString = geomString + ",";
        //   }
        // }
        // geomString = geomString + "))";

        // cout << geomString << endl;
        // GEOSGeometry* geom = GEOSGeomFromWKT_r(ctx, geomString.c_str());
        // GEOSSTRtree_query_r(ctx, tree, geom, intersect_callback, geom);

        cout << "Result: " << intersectCount << endl;
        polygonArray.clear();

        reply_.set_message(to_string(intersectCount));
        intersectCount = 0;
        // And we are done! Let the gRPC runtime know we've finished, using the
        // memory address of this instance as the uniquely identifying tag for
        // the event.
        status_ = FINISH;
        responder_.Finish(reply_, Status::OK, this);
      }
      else
      {
        GPR_ASSERT(status_ == FINISH);
        // Once in the FINISH state, deallocate ourselves (CallData).
        delete this;
      }
    }

  private:
    // The means of communication with the gRPC runtime for an asynchronous
    // server.
    Greeter::AsyncService *service_;
    // The producer-consumer queue where for asynchronous server notifications.
    ServerCompletionQueue *cq_;
    // Context for the rpc, allowing to tweak aspects of it such as the use
    // of compression, authentication, as well as to send metadata back to the
    // client.
    ServerContext ctx_;

    // What we get from the client.
    HelloRequest request_;
    // What we send back to the client.
    HelloReply reply_;

    // The means to get back to the client.
    ServerAsyncResponseWriter<HelloReply> responder_;

    // Let's implement a tiny state machine with the following states.
    enum CallStatus
    {
      CREATE,
      PROCESS,
      FINISH
    };
    CallStatus status_; // The current serving state.

    GEOSContextHandle_t ctx_geos;
    GEOSSTRtree *tree;
  };

  static void HandleRpcsHelper(int id, ServerImpl *server)
  {
    std::cout << "ID:" << id << std::endl;
    server->HandleRpcs();
  }

  // This can be run in multiple threads if needed.
  void HandleRpcs()
  {
    // Spawn a new CallData instance to serve new clients.
    new CallData(&service_, cq_.get(), ctx_0);
    void *tag; // uniquely identifies a request.
    bool ok;
    while (true)
    {
      // Block waiting to read the next event from the completion queue. The
      // event is uniquely identified by its tag, which in this case is the
      // memory address of a CallData instance.
      // The return value of Next should always be checked. This return value
      // tells us whether there is any kind of event or cq_ is shutting down.
      GPR_ASSERT(cq_->Next(&tag, &ok));
      GPR_ASSERT(ok);
      static_cast<CallData *>(tag)->Proceed();
    }
  }

  GEOSContextHandle_t ctx_0;
  std::unique_ptr<ServerCompletionQueue> cq_;
  Greeter::AsyncService service_;
  std::unique_ptr<Server> server_;
};


////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
  ServerImpl server;
  server.Run();

  return 0;
}


////////////////////////////////////////////////////////////////////////////////

// vector<GEOSGeometry*>* get_polygons(const char* filename,
//                                     GEOSContextHandle_t ctx) {
//   vector<GEOSGeometry*>* geoms = new vector<GEOSGeometry*>;
//   std::ifstream input(filename);
//   for (std::string line; getline(input, line);) {
//     if (line.length() > 5 && line.find("(")) {
//       GEOSGeometry* geom = GEOSGeomFromWKT_r(ctx, line.c_str());
//       geoms->push_back(geom);
//     }
//   }
//   return geoms;
// }

// map<std::string, GEOSGeometry*> get_polygon_map(const char* filename,
//                                                 GEOSContextHandle_t ctx) {
//   map<std::string, GEOSGeometry*> geom_map;
//   std::ifstream input(filename);
//   for (std::string line; getline(input, line);) {
//     if (line.length() > 5 && line.find("(")) {
//       string id = line.substr(0, line.find(" - "));
//       string geom_string = line.substr(line.find(" - ") + 3);
//       // cout << geom_string << endl;
//       GEOSGeometry* geom = GEOSGeomFromWKT_r(ctx, geom_string.c_str());
//       geom_map[id] = geom;
//     }
//   }
//   return geom_map;
// }

// // void intersect_callback(void* geom, void* base_geom) {
// //   GEOSWKTWriter* writer = GEOSWKTWriter_create_r(ctx);
// //   GEOSWKTWriter_setTrim_r(ctx, writer, 1);
// //   GEOSWKTWriter_setRoundingPrecision_r(ctx, writer, 3);
// //   char* wkt_geom =
// //       GEOSWKTWriter_write_r(ctx, writer, static_cast<const
// //       GEOSGeometry*>(geom));
// //   char* wkt_base_geom =
// //       GEOSWKTWriter_write_r(ctx, writer, static_cast<const
// //       GEOSGeometry*>(base_geom));
// //   GEOSWKTWriter_destroy_r(ctx, writer);
// //   cout << "Intersect candidate: " << wkt_base_geom << " - " << wkt_geom <<
// //   endl;

// //   intersectCandidateQueue.push(string(wkt_base_geom) + " - " +
// //                                string(wkt_geom));
// //   // if (!GEOSIntersects(static_cast<const GEOSGeometry*>(geom),
// //   //                     static_cast<const GEOSGeometry*>(base_geom))) {
// //   //   cout << "Intersect: " << wkt_base_geom << " - " << wkt_geom <<
// endl;
// //   // }
// // }

// int intersect(map<string, GEOSGeometry*> geoms,
//               map<string, GEOSGeometry*> geoms2, GEOSContextHandle_t ctx) {
//   GEOSSTRtree* tree = GEOSSTRtree_create_r(ctx, 10);

//   // for (auto cur = geoms->begin(); cur != geoms->end(); ++cur) {
//   //   GEOSGeometry* geom = *cur;
//   //   GEOSSTRtree_insert_r(ctx, tree, geom, GEOSEnvelope_r(ctx, geom));
//   // }

//   for (auto const& cur : geoms) {
//     GEOSGeometry* geom = cur.second;
//     GEOSSTRtree_insert_r(ctx, tree, geom, GEOSEnvelope_r(ctx, geom));
//   }

//   cout << "rtree created" << endl;

//   // GEOSGeometry* geoms2_cur;
//   // for (auto cur = geoms2->begin(); cur != geoms2->end(); ++cur) {
//   //   geoms2_cur = *cur;
//   //   GEOSSTRtree_query_r(ctx, tree, geoms2_cur, intersect_callback,
//   //   geoms2_cur);
//   // }

//   for (auto const& cur : geoms2) {
//     GEOSGeometry* geom = cur.second;
//     GEOSSTRtree_query_r(ctx, tree, geom, intersect_callback, geom);
//   }

//   GEOSSTRtree_destroy_r(ctx, tree);

//   return 0;
// }

// // double select_test(const char* name,
// //                    int (*test_function)(vector<GEOSGeometry*>*,
// //                                         vector<GEOSGeometry*>*,
// //                                         GEOSContextHandle_t),
// //                    vector<GEOSGeometry*>* geoms, vector<GEOSGeometry*>*
// //                    geoms2, int n, GEOSContextHandle_t ctx) {
// //   double time_arr[n];
// //   for (int i = 0; i < n; i++) {
// //     // clock_t t;
// //     // t = clock();

// //     struct timeval tv1, tv2;
// //     gettimeofday(&tv1, NULL);

// //     test_function(geoms, geoms2, ctx);

// //     gettimeofday(&tv2, NULL);
// //     double time_spent = (double)(tv2.tv_usec - tv1.tv_usec) / 1000000 +
// //                         (double)(tv2.tv_sec - tv1.tv_sec);
// //     time_arr[i] = time_spent;

// //     // t = clock() - t;
// //     // double time_taken = ((double)t) / CLOCKS_PER_SEC;
// //     // time_arr[i] = time_taken;
// //   }

// //   cout << "------------------- " << name << " -------------------" <<
// endl;
// //   for (int i = 0; i < n; i++) {
// //     cout << "The program " << i << " took " << time_arr[i]
// //          << " seconds to execute" << endl;
// //   }
// //   cout << "---------------------------------------------" << endl << endl;

// //   if (n == 1) {
// //     return time_arr[0];
// //   } else {
// //     double avg_time = 0;
// //     for (int i = 1; i < n; i++) {
// //       avg_time += time_arr[i];
// //     }
// //     avg_time /= (n - 1);
// //     return avg_time;
// //   }
// // }

// int start_test(std::string file1, std::string file2, GEOSContextHandle_t ctx)
// {
//   int n = 1;

//   const char* filename = file1.c_str();
//   const char* filename2 = file2.c_str();
//   cout << "File: " << filename << endl;

//   map<std::string, GEOSGeometry*> geoms = get_polygon_map(filename, ctx);
//   map<std::string, GEOSGeometry*> geoms2 = get_polygon_map(filename2, ctx);

//   intersect(geoms, geoms2, ctx);

//   // vector<GEOSGeometry*>* geoms = get_polygons(filename, ctx);
//   // vector<GEOSGeometry*>* geoms2 = get_polygons(filename2, ctx);

//   // // double create_time = 0;
//   // //  double iterate_time = 0;
//   // //  double query_time = 0;
//   // //  double intersect_time = 0;
//   // //  double overlap_time = 0;
//   // //  double touch_time = 0;
//   // //  double cross_time = 0;
//   // //  double contain_time = 0;
//   // //  double equal_time = 0;
//   // //  double equal_exact_time = 0;
//   // //  double cover_time = 0;
//   // //  double covered_by_time = 0;
//   // //  double create_time = select_test("Create", &create_tree, geoms,
//   // geoms2,
//   // //  n); double iterate_time = select_test("Iterate", &iterate_tree,
//   // geoms,
//   // //  geoms2, n); double query_time = select_test("Query", &query, geoms,
//   // //  geoms2, n);
//   // double intersect_time =
//   //     select_test("Intersect", &intersect, geoms, geoms2, n, ctx);
//   // total_intersect_time += intersect_time;
//   // // double overlap_time = select_test("Overlap", &overlap, geoms,
//   // geoms2, n);
//   // // double touch_time = select_test("Touch", &touch, geoms, geoms2, n);
//   // // double cross_time = select_test("Cross", &cross, geoms, geoms2, n);
//   // // double contain_time = select_test("Contain", &contain, geoms,
//   // geoms2, n);
//   // // double equal_time = select_test("Equal", &equal, geoms, geoms2, n);
//   // // double equal_exact_time = select_test("Equal Exact (0.3)",
//   // &equal_exact,
//   // // geoms, geoms2, n); double cover_time = select_test("Cover", &cover,
//   // geoms,
//   // // geoms2, n); double covered_by_time = select_test("Covered By",
//   // &covered_by,
//   // // geoms, geoms2, n);

//   // cout << endl
//   //      << endl
//   //      <<
//   // "------------------------------------------------------------------"
//   //      << endl
//   //      << "--------------------- BENCHMARK RESULT (CPU)
//   //      ---------------------"
//   //      << endl
//   //      <<
//   // "------------------------------------------------------------------"
//   //      << endl
//   //      //  << "Average Create Time: " << create_time << endl
//   //      //  << "Average Iterate Time: " << iterate_time << endl
//   //      //  << "Average Query Time: " << query_time << endl
//   //      << "Average Intersect Time: " << intersect_time
//   //      << endl
//   //      //  << "Average Overlap Time: " << overlap_time << endl
//   //      //  << "Average Touch Time: " << touch_time << endl
//   //      //  << "Average Cross Time: " << cross_time << endl
//   //      //  << "Average Contain Time: " << contain_time << endl
//   //      //  << "Average Equal Time: " << equal_time << endl
//   //      //  << "Average Equal Exact (0.3) Time: " << equal_exact_time <<
//   //      endl
//   //      //  << "Average Cover Time: " << cover_time << endl
//   //      //  << "Average Covered By Time: " << covered_by_time << endl
//   //      <<
//   // "------------------------------------------------------------------"
//   //      << endl
//   //      <<
//   // "------------------------------------------------------------------"
//   //      << endl
//   //      << endl
//   //      << endl;

//   return 0;
// }
