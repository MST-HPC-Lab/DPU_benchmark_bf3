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

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;

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

int main(int argc, char** argv) {
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint (in this case,
  // localhost at port 50051). We indicate that the channel isn't authenticated
  // (use of InsecureChannelCredentials()).

  GEOSContextHandle_t ctx = GEOS_init_r();
  GEOSContext_setNoticeHandler_r(ctx, geos_message_handler);
  GEOSContext_setErrorHandler_r(ctx, geos_message_handler);
  cout << "GEOS version: %s\n" << GEOSversion() << endl;

  struct timeval tv1, tv2;
  gettimeofday(&tv1, NULL);



  GreeterClient greeter(grpc::CreateChannel(
      "192.168.100.2:50051", grpc::InsecureChannelCredentials()));

  std::ifstream input("/home/dpu/data/cemet_data_indexed");
  int count = 0;
  string queryGeoms = "";
  for (std::string line; getline(input, line);) {
    if (line.length() > 5 && line.find("(")) {
      count++;
      string id = line.substr(0, line.find(" - "));
      string geom_string = line.substr(line.find(" - ") + 3);
      queryGeoms = queryGeoms + "\n" + geom_string;

      if (count % 10 == 0) {
        // cout << "Sending " << queryGeoms << " ..." << endl;
        cout << "Sending..." << endl;
        std::string reply = greeter.SayHello(queryGeoms);
        std::cout << "Result: " << reply << std::endl;
        queryGeoms = "";
      }
    }
  }

  finishGEOS_r(ctx);

  gettimeofday(&tv2, NULL);
  double time_spent = (double)(tv2.tv_usec - tv1.tv_usec) / 1000000 +
                      (double)(tv2.tv_sec - tv1.tv_sec);
  // cout << "Total Intersect Time:" << total_intersect_time << endl;
  cout << endl << endl << "Total Time:" << time_spent << endl;


  return 0;
}