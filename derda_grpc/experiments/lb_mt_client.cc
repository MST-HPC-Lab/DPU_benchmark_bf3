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
#include "../protos/bluefield_grpc.grpc.pb.h"
#else
#include "bluefield_grpc.grpc.pb.h"
#endif

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using bluefield_grpc::Greeter;
using bluefield_grpc::HelloReply;
using bluefield_grpc::HelloRequest;

using namespace std;

queue<std::string> jobQueue;
double total_intersect_time = 0;
int start_test(std::string file1, std::string file2, GEOSContextHandle_t ctx);

static void geos_message_handler(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
}

class GreeterClient {
 public:
  explicit GreeterClient(std::shared_ptr<Channel> channel)
      : stub_(Greeter::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  std::string SayHello(const std::string& user) {
    // Data we are sending to the server.
    HelloRequest request;
    request.set_name(user);

    // Container for the data we expect from the server.
    HelloReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The producer-consumer queue we use to communicate asynchronously with the
    // gRPC runtime.
    CompletionQueue cq;

    // Storage for the status of the RPC upon completion.
    Status status;

    std::unique_ptr<ClientAsyncResponseReader<HelloReply> > rpc(
        stub_->AsyncSayHello(&context, request, &cq));

    // Request that, upon completion of the RPC, "reply" be updated with the
    // server's response; "status" with the indication of whether the operation
    // was successful. Tag the request with the integer 1.
    rpc->Finish(&reply, &status, (void*)1);
    void* got_tag;
    bool ok = false;
    // Block until the next result is available in the completion queue "cq".
    // The return value of Next should always be checked. This return value
    // tells us whether there is any kind of event or the cq_ is shutting down.
    GPR_ASSERT(cq.Next(&got_tag, &ok));

    // Verify that the result from "cq" corresponds, by its tag, our previous
    // request.
    GPR_ASSERT(got_tag == (void*)1);
    // ... and that the request was completed successfully. Note that "ok"
    // corresponds solely to the request for updates introduced by Finish().
    GPR_ASSERT(ok);

    // Act upon the status of the actual RPC.
    if (status.ok()) {
      return reply.message();
    } else {
      return "RPC failed";
    }
  }

 private:
  // Out of the passed in Channel comes the stub, stored here, our view of the
  // server's exposed services.
  std::unique_ptr<Greeter::Stub> stub_;
};

void doWork(GEOSContextHandle_t ctx) {
  while (!jobQueue.empty()) {
    string work = jobQueue.front();
    jobQueue.pop();
    cout << "Processing " << work << "..." << endl;
    // sleep(1);
    start_test(work, "/global/scratch/users/satishp/data/cemet_data", ctx);
    cout << work << " is done!" << endl;
  }
}

void sendWork() {
  GreeterClient greeter(grpc::CreateChannel(
      "192.168.3.110:50051", grpc::InsecureChannelCredentials()));

  while (!jobQueue.empty()) {
    cout << "Sending " << jobQueue.front() << "..." << endl;
    std::string user(jobQueue.front());
    jobQueue.pop();
    std::string reply = greeter.SayHello(user);  // The actual RPC call!
    std::cout << "Greeter received: " << reply << std::endl;
  }
}

int main(int argc, char** argv) {
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint (in this case,
  // localhost at port 50051). We indicate that the channel isn't authenticated
  // (use of InsecureChannelCredentials()).

  GEOSContextHandle_t ctx = GEOS_init_r();
  GEOSContext_setNoticeHandler_r(ctx, geos_message_handler);
  GEOSContext_setErrorHandler_r(ctx, geos_message_handler);
  cout << "GEOS version: %s\n" << GEOSversion() << endl;

  for (int i = 0; i < 128; i++) {
    jobQueue.push("/global/scratch/users/satishp/data/sports_128/" +
                  to_string(i));
  }

  struct timeval tv1, tv2;
  gettimeofday(&tv1, NULL);

  thread thread1(doWork, ctx);
  thread thread2(doWork, ctx);
  thread thread3(doWork, ctx);
  thread thread4(doWork, ctx);
  thread thread5(doWork, ctx);
  thread thread6(doWork, ctx);
  thread thread7(doWork, ctx);
  thread thread8(doWork, ctx);

  // thread th1(doWork);
  //thread th2(sendWork);
  // th1.join();
  //th2.join();

  thread1.join();
  thread2.join();
  thread3.join();
  thread4.join();
  thread5.join();
  thread6.join();
  thread7.join();
  thread8.join();

  finishGEOS_r(ctx);


  // doWork();

  gettimeofday(&tv2, NULL);
  double time_spent = (double)(tv2.tv_usec - tv1.tv_usec) / 1000000 +
                      (double)(tv2.tv_sec - tv1.tv_sec);
  cout << "Total Intersect Time:" << total_intersect_time << endl;
  cout << "Total Time:" << time_spent << endl;

  // std::string user("world");
  // std::string reply = greeter.SayHello(user);  // The actual RPC call!
  // std::cout << "Greeter received: " << reply << std::endl;

  return 0;
}

vector<GEOSGeometry*>* get_polygons(const char* filename,
                                    GEOSContextHandle_t ctx) {
  vector<GEOSGeometry*>* geoms = new vector<GEOSGeometry*>;
  std::ifstream input(filename);
  for (std::string line; getline(input, line);) {
    if (line.length() > 5 && line.find("(")) {
      GEOSGeometry* geom = GEOSGeomFromWKT_r(ctx, line.c_str());
      geoms->push_back(geom);
    }
  }
  return geoms;
}

int create_tree(vector<GEOSGeometry*>* geoms, vector<GEOSGeometry*>* geoms2) {
  GEOSSTRtree* tree = GEOSSTRtree_create(10);

  // std::ifstream input(filename);
  // for (std::string line; getline(input, line);)
  // {
  //     if (line.length() > 5 && line.find("("))
  //     {
  //         GEOSGeometry *geom = GEOSGeomFromWKT(line.c_str());
  //         GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
  //     }
  // }

  for (auto cur = geoms->begin(); cur != geoms->end(); ++cur) {
    GEOSGeometry* geom = *cur;
    GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
  }

  GEOSSTRtree_destroy(tree);

  return 0;
}

int element_count = 0;
void iterate_tree_callback(void* geom, void* userdata) {
  // GEOSWKTWriter *writer = GEOSWKTWriter_create();
  // GEOSWKTWriter_setTrim(writer, 1);
  // GEOSWKTWriter_setRoundingPrecision(writer, 3);
  // char *wkt_geom = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry
  // *>(geom));

  // GEOSWKTWriter_destroy(writer);
  // cout << wkt_geom << endl;
  element_count++;
}

int iterate_tree(vector<GEOSGeometry*>* geoms, vector<GEOSGeometry*>* geoms2) {
  GEOSSTRtree* tree = GEOSSTRtree_create(10);

  for (auto cur = geoms->begin(); cur != geoms->end(); ++cur) {
    GEOSGeometry* geom = *cur;
    GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
  }

  GEOSSTRtree_iterate(tree, iterate_tree_callback, 0);
  cout << "# of Geometries: " << element_count << endl;

  GEOSSTRtree_destroy(tree);

  return 0;
}

void query_callback(void* geom, void* base_geom) {
  // GEOSWKTWriter *writer = GEOSWKTWriter_create();
  // GEOSWKTWriter_setTrim(writer, 1);
  // GEOSWKTWriter_setRoundingPrecision(writer, 3);
  // char *wkt_geom = GEOSWKTWriter_write(writer, static_cast<const GEOSGeometry
  // *>(geom)); char *wkt_base_geom = GEOSWKTWriter_write(writer,
  // static_cast<const GEOSGeometry *>(base_geom));
  // GEOSWKTWriter_destroy(writer);
  // cout <<"MBR Intersection: " << wkt_base_geom << " - " << wkt_geom << endl;
}

int query(vector<GEOSGeometry*>* geoms, vector<GEOSGeometry*>* geoms2) {
  GEOSSTRtree* tree = GEOSSTRtree_create(10);

  for (auto cur = geoms->begin(); cur != geoms->end(); ++cur) {
    GEOSGeometry* geom = *cur;
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

  GEOSGeometry* geoms2_cur;
  for (auto cur = geoms2->begin(); cur != geoms2->end(); ++cur) {
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

void intersect_callback(void* geom, void* base_geom) {
  if (!GEOSIntersects(static_cast<const GEOSGeometry*>(geom),
                      static_cast<const GEOSGeometry*>(base_geom))) {
    // GEOSWKTWriter *writer = GEOSWKTWriter_create();
    // GEOSWKTWriter_setTrim(writer, 1);
    // GEOSWKTWriter_setRoundingPrecision(writer, 3);
    // char *wkt_geom = GEOSWKTWriter_write(writer, static_cast<const
    // GEOSGeometry *>(geom)); char *wkt_base_geom = GEOSWKTWriter_write(writer,
    // static_cast<const GEOSGeometry *>(base_geom));
    // GEOSWKTWriter_destroy(writer);
    // cout <<"Intersect: " << wkt_base_geom << " - " << wkt_geom << endl;
  }
}

int intersect(vector<GEOSGeometry*>* geoms, vector<GEOSGeometry*>* geoms2,
              GEOSContextHandle_t ctx) {
  GEOSSTRtree* tree = GEOSSTRtree_create_r(ctx, 10);

  for (auto cur = geoms->begin(); cur != geoms->end(); ++cur) {
    GEOSGeometry* geom = *cur;
    GEOSSTRtree_insert_r(ctx, tree, geom, GEOSEnvelope_r(ctx, geom));
  }

  GEOSGeometry* geoms2_cur;
  for (auto cur = geoms2->begin(); cur != geoms2->end(); ++cur) {
    geoms2_cur = *cur;
    GEOSSTRtree_query_r(ctx, tree, geoms2_cur, intersect_callback, geoms2_cur);
  }

  GEOSSTRtree_destroy_r(ctx, tree);

  return 0;
}

void overlap_callback(void* geom, void* base_geom) {
  if (!GEOSOverlaps(static_cast<const GEOSGeometry*>(geom),
                    static_cast<const GEOSGeometry*>(base_geom))) {
    // GEOSWKTWriter *writer = GEOSWKTWriter_create();
    // GEOSWKTWriter_setTrim(writer, 1);
    // GEOSWKTWriter_setRoundingPrecision(writer, 3);
    // char *wkt_geom = GEOSWKTWriter_write(writer, static_cast<const
    // GEOSGeometry *>(geom)); char *wkt_base_geom = GEOSWKTWriter_write(writer,
    // static_cast<const GEOSGeometry *>(base_geom));
    // GEOSWKTWriter_destroy(writer);
    // cout <<"Overlap: " << wkt_base_geom << " - " << wkt_geom << endl;
  }
}

int overlap(vector<GEOSGeometry*>* geoms, vector<GEOSGeometry*>* geoms2) {
  GEOSSTRtree* tree = GEOSSTRtree_create(10);

  for (auto cur = geoms->begin(); cur != geoms->end(); ++cur) {
    GEOSGeometry* geom = *cur;
    GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
  }

  GEOSGeometry* geoms2_cur;
  for (auto cur = geoms2->begin(); cur != geoms2->end(); ++cur) {
    geoms2_cur = *cur;
    GEOSSTRtree_query(tree, geoms2_cur, overlap_callback, geoms2_cur);
  }

  GEOSSTRtree_destroy(tree);

  return 0;
}

void touch_callback(void* geom, void* base_geom) {
  if (!GEOSTouches(static_cast<const GEOSGeometry*>(geom),
                   static_cast<const GEOSGeometry*>(base_geom))) {
    // GEOSWKTWriter *writer = GEOSWKTWriter_create();
    // GEOSWKTWriter_setTrim(writer, 1);
    // GEOSWKTWriter_setRoundingPrecision(writer, 3);
    // char *wkt_geom = GEOSWKTWriter_write(writer, static_cast<const
    // GEOSGeometry *>(geom)); char *wkt_base_geom = GEOSWKTWriter_write(writer,
    // static_cast<const GEOSGeometry *>(base_geom));
    // GEOSWKTWriter_destroy(writer);
    // cout <<"Touch: " << wkt_base_geom << " - " << wkt_geom << endl;
  }
}

int touch(vector<GEOSGeometry*>* geoms, vector<GEOSGeometry*>* geoms2) {
  GEOSSTRtree* tree = GEOSSTRtree_create(10);

  for (auto cur = geoms->begin(); cur != geoms->end(); ++cur) {
    GEOSGeometry* geom = *cur;
    GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
  }

  GEOSGeometry* geoms2_cur;
  for (auto cur = geoms2->begin(); cur != geoms2->end(); ++cur) {
    geoms2_cur = *cur;
    GEOSSTRtree_query(tree, geoms2_cur, touch_callback, geoms2_cur);
  }

  GEOSSTRtree_destroy(tree);

  return 0;
}

void cross_callback(void* geom, void* base_geom) {
  if (!GEOSCrosses(static_cast<const GEOSGeometry*>(geom),
                   static_cast<const GEOSGeometry*>(base_geom))) {
    // GEOSWKTWriter *writer = GEOSWKTWriter_create();
    // GEOSWKTWriter_setTrim(writer, 1);
    // GEOSWKTWriter_setRoundingPrecision(writer, 3);
    // char *wkt_geom = GEOSWKTWriter_write(writer, static_cast<const
    // GEOSGeometry *>(geom)); char *wkt_base_geom = GEOSWKTWriter_write(writer,
    // static_cast<const GEOSGeometry *>(base_geom));
    // GEOSWKTWriter_destroy(writer);
    // cout <<"Cross: " << wkt_base_geom << " - " << wkt_geom << endl;
  }
}

int cross(vector<GEOSGeometry*>* geoms, vector<GEOSGeometry*>* geoms2) {
  GEOSSTRtree* tree = GEOSSTRtree_create(10);

  for (auto cur = geoms->begin(); cur != geoms->end(); ++cur) {
    GEOSGeometry* geom = *cur;
    GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
  }

  GEOSGeometry* geoms2_cur;
  for (auto cur = geoms2->begin(); cur != geoms2->end(); ++cur) {
    geoms2_cur = *cur;
    GEOSSTRtree_query(tree, geoms2_cur, cross_callback, geoms2_cur);
  }

  GEOSSTRtree_destroy(tree);

  return 0;
}

void contain_callback(void* geom, void* base_geom) {
  if (!GEOSContains(static_cast<const GEOSGeometry*>(geom),
                    static_cast<const GEOSGeometry*>(base_geom))) {
    // GEOSWKTWriter *writer = GEOSWKTWriter_create();
    // GEOSWKTWriter_setTrim(writer, 1);
    // GEOSWKTWriter_setRoundingPrecision(writer, 3);
    // char *wkt_geom = GEOSWKTWriter_write(writer, static_cast<const
    // GEOSGeometry *>(geom)); char *wkt_base_geom = GEOSWKTWriter_write(writer,
    // static_cast<const GEOSGeometry *>(base_geom));
    // GEOSWKTWriter_destroy(writer);
    // cout <<"Contain: " << wkt_base_geom << " - " << wkt_geom << endl;
  }
}

int contain(vector<GEOSGeometry*>* geoms, vector<GEOSGeometry*>* geoms2) {
  GEOSSTRtree* tree = GEOSSTRtree_create(10);

  for (auto cur = geoms->begin(); cur != geoms->end(); ++cur) {
    GEOSGeometry* geom = *cur;
    GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
  }

  GEOSGeometry* geoms2_cur;
  for (auto cur = geoms2->begin(); cur != geoms2->end(); ++cur) {
    geoms2_cur = *cur;
    GEOSSTRtree_query(tree, geoms2_cur, contain_callback, geoms2_cur);
  }

  GEOSSTRtree_destroy(tree);

  return 0;
}

void equal_callback(void* geom, void* base_geom) {
  if (!GEOSEquals(static_cast<const GEOSGeometry*>(geom),
                  static_cast<const GEOSGeometry*>(base_geom))) {
    // GEOSWKTWriter *writer = GEOSWKTWriter_create();
    // GEOSWKTWriter_setTrim(writer, 1);
    // GEOSWKTWriter_setRoundingPrecision(writer, 3);
    // char *wkt_geom = GEOSWKTWriter_write(writer, static_cast<const
    // GEOSGeometry *>(geom)); char *wkt_base_geom = GEOSWKTWriter_write(writer,
    // static_cast<const GEOSGeometry *>(base_geom));
    // GEOSWKTWriter_destroy(writer);
    // cout <<"Equal: " << wkt_base_geom << " - " << wkt_geom << endl;
  }
}

int equal(vector<GEOSGeometry*>* geoms, vector<GEOSGeometry*>* geoms2) {
  GEOSSTRtree* tree = GEOSSTRtree_create(10);

  for (auto cur = geoms->begin(); cur != geoms->end(); ++cur) {
    GEOSGeometry* geom = *cur;
    GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
  }

  GEOSGeometry* geoms2_cur;
  for (auto cur = geoms2->begin(); cur != geoms2->end(); ++cur) {
    geoms2_cur = *cur;
    GEOSSTRtree_query(tree, geoms2_cur, equal_callback, geoms2_cur);
  }

  GEOSSTRtree_destroy(tree);

  return 0;
}

void equal_exact_callback(void* geom, void* base_geom) {
  if (!GEOSEqualsExact(static_cast<const GEOSGeometry*>(geom),
                       static_cast<const GEOSGeometry*>(base_geom), 0.3)) {
    // GEOSWKTWriter *writer = GEOSWKTWriter_create();
    // GEOSWKTWriter_setTrim(writer, 1);
    // GEOSWKTWriter_setRoundingPrecision(writer, 3);
    // char *wkt_geom = GEOSWKTWriter_write(writer, static_cast<const
    // GEOSGeometry *>(geom)); char *wkt_base_geom = GEOSWKTWriter_write(writer,
    // static_cast<const GEOSGeometry *>(base_geom));
    // GEOSWKTWriter_destroy(writer);
    // cout <<"Equal Exact (0.3): " << wkt_base_geom << " - " << wkt_geom <<
    // endl;
  }
}

int equal_exact(vector<GEOSGeometry*>* geoms, vector<GEOSGeometry*>* geoms2) {
  GEOSSTRtree* tree = GEOSSTRtree_create(10);

  for (auto cur = geoms->begin(); cur != geoms->end(); ++cur) {
    GEOSGeometry* geom = *cur;
    GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
  }

  GEOSGeometry* geoms2_cur;
  for (auto cur = geoms2->begin(); cur != geoms2->end(); ++cur) {
    geoms2_cur = *cur;
    GEOSSTRtree_query(tree, geoms2_cur, equal_exact_callback, geoms2_cur);
  }

  GEOSSTRtree_destroy(tree);

  return 0;
}

void cover_callback(void* geom, void* base_geom) {
  if (!GEOSCovers(static_cast<const GEOSGeometry*>(geom),
                  static_cast<const GEOSGeometry*>(base_geom))) {
    // GEOSWKTWriter *writer = GEOSWKTWriter_create();
    // GEOSWKTWriter_setTrim(writer, 1);
    // GEOSWKTWriter_setRoundingPrecision(writer, 3);
    // char *wkt_geom = GEOSWKTWriter_write(writer, static_cast<const
    // GEOSGeometry *>(geom)); char *wkt_base_geom = GEOSWKTWriter_write(writer,
    // static_cast<const GEOSGeometry *>(base_geom));
    // GEOSWKTWriter_destroy(writer);
    // cout <<"Cover: " << wkt_base_geom << " - " << wkt_geom << endl;
  }
}

int cover(vector<GEOSGeometry*>* geoms, vector<GEOSGeometry*>* geoms2) {
  GEOSSTRtree* tree = GEOSSTRtree_create(10);

  for (auto cur = geoms->begin(); cur != geoms->end(); ++cur) {
    GEOSGeometry* geom = *cur;
    GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
  }

  GEOSGeometry* geoms2_cur;
  for (auto cur = geoms2->begin(); cur != geoms2->end(); ++cur) {
    geoms2_cur = *cur;
    GEOSSTRtree_query(tree, geoms2_cur, cover_callback, geoms2_cur);
  }

  GEOSSTRtree_destroy(tree);

  return 0;
}

void covered_by_callback(void* geom, void* base_geom) {
  if (!GEOSCoveredBy(static_cast<const GEOSGeometry*>(geom),
                     static_cast<const GEOSGeometry*>(base_geom))) {
    // GEOSWKTWriter *writer = GEOSWKTWriter_create();
    // GEOSWKTWriter_setTrim(writer, 1);
    // GEOSWKTWriter_setRoundingPrecision(writer, 3);
    // char *wkt_geom = GEOSWKTWriter_write(writer, static_cast<const
    // GEOSGeometry *>(geom)); char *wkt_base_geom = GEOSWKTWriter_write(writer,
    // static_cast<const GEOSGeometry *>(base_geom));
    // GEOSWKTWriter_destroy(writer);
    // cout << "Covered By: " << wkt_base_geom << " - " << wkt_geom << endl;
  }
}

int covered_by(vector<GEOSGeometry*>* geoms, vector<GEOSGeometry*>* geoms2) {
  GEOSSTRtree* tree = GEOSSTRtree_create(10);

  for (auto cur = geoms->begin(); cur != geoms->end(); ++cur) {
    GEOSGeometry* geom = *cur;
    GEOSSTRtree_insert(tree, geom, GEOSEnvelope(geom));
  }

  GEOSGeometry* geoms2_cur;
  for (auto cur = geoms2->begin(); cur != geoms2->end(); ++cur) {
    geoms2_cur = *cur;
    GEOSSTRtree_query(tree, geoms2_cur, covered_by_callback, geoms2_cur);
  }

  GEOSSTRtree_destroy(tree);

  return 0;
}

double select_test(const char* name,
                   int (*test_function)(vector<GEOSGeometry*>*,
                                        vector<GEOSGeometry*>*,
                                        GEOSContextHandle_t),
                   vector<GEOSGeometry*>* geoms, vector<GEOSGeometry*>* geoms2,
                   int n, GEOSContextHandle_t ctx) {
  double time_arr[n];
  for (int i = 0; i < n; i++) {
    // clock_t t;
    // t = clock();

    struct timeval tv1, tv2;
    gettimeofday(&tv1, NULL);

    test_function(geoms, geoms2, ctx);

    gettimeofday(&tv2, NULL);
    double time_spent = (double)(tv2.tv_usec - tv1.tv_usec) / 1000000 +
                        (double)(tv2.tv_sec - tv1.tv_sec);
    time_arr[i] = time_spent;

    // t = clock() - t;
    // double time_taken = ((double)t) / CLOCKS_PER_SEC;
    // time_arr[i] = time_taken;
  }

  cout << "------------------- " << name << " -------------------" << endl;
  for (int i = 0; i < n; i++) {
    cout << "The program " << i << " took " << time_arr[i]
         << " seconds to execute" << endl;
  }
  cout << "---------------------------------------------" << endl << endl;

  if (n == 1) {
    return time_arr[0];
  } else {
    double avg_time = 0;
    for (int i = 1; i < n; i++) {
      avg_time += time_arr[i];
    }
    avg_time /= (n - 1);
    return avg_time;
  }
}

int start_test(std::string file1, std::string file2, GEOSContextHandle_t ctx) {
  int n = 1;

  const char* filename = file1.c_str();
  const char* filename2 = file2.c_str();
  cout << "File: " << filename << endl;

  vector<GEOSGeometry*>* geoms = get_polygons(filename, ctx);
  vector<GEOSGeometry*>* geoms2 = get_polygons(filename2, ctx);

  // double create_time = 0;
  //  double iterate_time = 0;
  //  double query_time = 0;
  //  double intersect_time = 0;
  //  double overlap_time = 0;
  //  double touch_time = 0;
  //  double cross_time = 0;
  //  double contain_time = 0;
  //  double equal_time = 0;
  //  double equal_exact_time = 0;
  //  double cover_time = 0;
  //  double covered_by_time = 0;
  //  double create_time = select_test("Create", &create_tree, geoms, geoms2,
  //  n); double iterate_time = select_test("Iterate", &iterate_tree, geoms,
  //  geoms2, n); double query_time = select_test("Query", &query, geoms,
  //  geoms2, n);
  double intersect_time =
      select_test("Intersect", &intersect, geoms, geoms2, n, ctx);
  total_intersect_time += intersect_time;
  // double overlap_time = select_test("Overlap", &overlap, geoms, geoms2, n);
  // double touch_time = select_test("Touch", &touch, geoms, geoms2, n);
  // double cross_time = select_test("Cross", &cross, geoms, geoms2, n);
  // double contain_time = select_test("Contain", &contain, geoms, geoms2, n);
  // double equal_time = select_test("Equal", &equal, geoms, geoms2, n);
  // double equal_exact_time = select_test("Equal Exact (0.3)", &equal_exact,
  // geoms, geoms2, n); double cover_time = select_test("Cover", &cover, geoms,
  // geoms2, n); double covered_by_time = select_test("Covered By", &covered_by,
  // geoms, geoms2, n);

  cout << endl
       << endl
       << "------------------------------------------------------------------"
       << endl
       << "--------------------- BENCHMARK RESULT (CPU) ---------------------"
       << endl
       << "------------------------------------------------------------------"
       << endl
       //  << "Average Create Time: " << create_time << endl
       //  << "Average Iterate Time: " << iterate_time << endl
       //  << "Average Query Time: " << query_time << endl
       << "Average Intersect Time: " << intersect_time
       << endl
       //  << "Average Overlap Time: " << overlap_time << endl
       //  << "Average Touch Time: " << touch_time << endl
       //  << "Average Cross Time: " << cross_time << endl
       //  << "Average Contain Time: " << contain_time << endl
       //  << "Average Equal Time: " << equal_time << endl
       //  << "Average Equal Exact (0.3) Time: " << equal_exact_time << endl
       //  << "Average Cover Time: " << cover_time << endl
       //  << "Average Covered By Time: " << covered_by_time << endl
       << "------------------------------------------------------------------"
       << endl
       << "------------------------------------------------------------------"
       << endl
       << endl
       << endl;

  return 0;
}
