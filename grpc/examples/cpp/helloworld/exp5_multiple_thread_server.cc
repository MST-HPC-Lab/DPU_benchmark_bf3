/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
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
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "helloworld.grpc.pb.h"
#endif

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;
using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;

using namespace std;
double total_intersect_time = 0;
int start_test(std::string file1, std::string file2, GEOSContextHandle_t ctx);

GEOSContextHandle_t ctx;
queue<std::string> intersectCandidateQueue;

static void geos_message_handler(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
}

string intersectsRefinement(string candidatePair) {
  string base_geom_string = candidatePair.substr(0, candidatePair.find(" - "));
  string query_geom_string = candidatePair.substr(candidatePair.find(" - ") + 3);
  GEOSGeometry* base_geom = GEOSGeomFromWKT_r(ctx, base_geom_string.c_str());
  GEOSGeometry* query_geom = GEOSGeomFromWKT_r(ctx, query_geom_string.c_str());
  if (!GEOSIntersects_r(ctx, static_cast<const GEOSGeometry*>(base_geom),
                        static_cast<const GEOSGeometry*>(query_geom))) {
    return "Intersect: " + candidatePair;
  } else {
    return " No Intersect: " + candidatePair;
  }
}

class ServerImpl final {
 public:
  ~ServerImpl() {
    server_->Shutdown();
    // Always shutdown the completion queue after the server.
    cq_->Shutdown();
  }

  // There is no shutdown handling in this code.
  void Run() {
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

    ctx = GEOS_init_r();
    GEOSContext_setNoticeHandler_r(ctx, geos_message_handler);
    GEOSContext_setErrorHandler_r(ctx, geos_message_handler);
    cout << "GEOS version: %s\n" << GEOSversion() << endl;

    std::thread thread1(HandleRpcsHelper, 1, this);
    std::thread thread2(HandleRpcsHelper, 2, this);
    std::thread thread3(HandleRpcsHelper, 3, this);
    std::thread thread4(HandleRpcsHelper, 4, this);
    std::thread thread5(HandleRpcsHelper, 5, this);
    std::thread thread6(HandleRpcsHelper, 6, this);
    std::thread thread7(HandleRpcsHelper, 7, this);
    std::thread thread8(HandleRpcsHelper, 8, this);
    // HandleRpcs();
    thread1.join();
    thread2.join();
    thread3.join();
    thread4.join();
    thread5.join();
    thread6.join();
    thread7.join();
    thread8.join();

    finishGEOS_r(ctx);
  }

 private:
  // Class encompasing the state and logic needed to serve a request.
  class CallData {
   public:
    // Take in the "service" instance (in this case representing an asynchronous
    // server) and the completion queue "cq" used for asynchronous communication
    // with the gRPC runtime.
    CallData(Greeter::AsyncService* service, ServerCompletionQueue* cq)
        : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {
      // Invoke the serving logic right away.
      Proceed();
    }

    void Proceed() {
      if (status_ == CREATE) {
        // Make this instance progress to the PROCESS state.
        status_ = PROCESS;

        // As part of the initial CREATE state, we *request* that the system
        // start processing SayHello requests. In this request, "this" acts are
        // the tag uniquely identifying the request (so that different CallData
        // instances can serve different requests concurrently), in this case
        // the memory address of this CallData instance.
        service_->RequestSayHello(&ctx_, &request_, &responder_, cq_, cq_,
                                  this);
      } else if (status_ == PROCESS) {
        // Spawn a new CallData instance to serve new clients while we process
        // the one for this CallData. The instance will deallocate itself as
        // part of its FINISH state.
        new CallData(service_, cq_);

        // The actual processing.

        // if (request_.name() == "next") {
        //   if (intersectCandidateQueue.empty()) {
        //     reply_.set_message("done");
        //   } else {
        //     reply_.set_message(intersectCandidateQueue.front());
        //     intersectCandidateQueue.pop();
        //   }
        // } else {
        cout << "Processing " << request_.name() << "..." << endl;
        string answer = intersectsRefinement(request_.name());

        // sleep(3);
        // start_test(request_.name(),
        //            "/global/scratch/users/satishp/data/cemet_data_indexed",
        //            ctx);
        // start_test("/home/ubuntu/data/" + request_.name(),
        //            "/home/ubuntu/data/cemet_data_indexed",
        //            ctx);
        cout << "Result: " << answer << endl;

        reply_.set_message(answer);
        //} 

        // std::string prefix("Hello ");
        // reply_.set_message(prefix + request_.name());

        // And we are done! Let the gRPC runtime know we've finished, using the
        // memory address of this instance as the uniquely identifying tag for
        // the event.
        status_ = FINISH;
        responder_.Finish(reply_, Status::OK, this);
      } else {
        GPR_ASSERT(status_ == FINISH);
        // Once in the FINISH state, deallocate ourselves (CallData).
        delete this;
      }
    }

   private:
    // The means of communication with the gRPC runtime for an asynchronous
    // server.
    Greeter::AsyncService* service_;
    // The producer-consumer queue where for asynchronous server notifications.
    ServerCompletionQueue* cq_;
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
    enum CallStatus { CREATE, PROCESS, FINISH };
    CallStatus status_;  // The current serving state.
  };

  static void HandleRpcsHelper(int id, ServerImpl* server) {
    std::cout << "ID:" << id << std::endl;
    server->HandleRpcs();
  }

  // This can be run in multiple threads if needed.
  void HandleRpcs() {
    // Spawn a new CallData instance to serve new clients.
    new CallData(&service_, cq_.get());
    void* tag;  // uniquely identifies a request.
    bool ok;
    while (true) {
      // Block waiting to read the next event from the completion queue. The
      // event is uniquely identified by its tag, which in this case is the
      // memory address of a CallData instance.
      // The return value of Next should always be checked. This return value
      // tells us whether there is any kind of event or cq_ is shutting down.
      GPR_ASSERT(cq_->Next(&tag, &ok));
      GPR_ASSERT(ok);
      static_cast<CallData*>(tag)->Proceed();
    }
  }

  std::unique_ptr<ServerCompletionQueue> cq_;
  Greeter::AsyncService service_;
  std::unique_ptr<Server> server_;
};

int main(int argc, char** argv) {
  ServerImpl server;
  server.Run();

  return 0;
}

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
// //   //   cout << "Intersect: " << wkt_base_geom << " - " << wkt_geom << endl;
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

// //   cout << "------------------- " << name << " -------------------" << endl;
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

// int start_test(std::string file1, std::string file2, GEOSContextHandle_t ctx) {
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
//   //      "------------------------------------------------------------------"
//   //      << endl
//   //      << "--------------------- BENCHMARK RESULT (CPU)
//   //      ---------------------"
//   //      << endl
//   //      <<
//   //      "------------------------------------------------------------------"
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
//   //      "------------------------------------------------------------------"
//   //      << endl
//   //      <<
//   //      "------------------------------------------------------------------"
//   //      << endl
//   //      << endl
//   //      << endl;

//   return 0;
// }
