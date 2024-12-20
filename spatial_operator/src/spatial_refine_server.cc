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

#include <iostream>
#include <memory>
#include <string>
#include <chrono>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include "bluefield_spatial.grpc.pb.h"

#include "utilities/spatial_utilities.cc"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

// using bluefield_spatial::SpatialOp;
// using bluefield_spatial::Polygon;
using bluefield_spatial::SimpleReply;
// using bluefield_spatial::SimpleRequest;
// using bluefield_spatial::OpRequest;
using bluefield_spatial::OpResponse;
using bluefield_spatial::GeomReply;
// using bluefield_spatial::GeomRequest;
// using bluefield_spatial::IndexedRequest;
using bluefield_spatial::IndexedReply;

using std::chrono::system_clock;



// Logic and data behind the server's behavior.
class SpatialServiceImpl final : public SpatialOp::Service {
  Status SayMessage(ServerContext* context, const SimpleRequest* request,
                  SimpleReply* reply) override {
    std::string prefix("Hello ");
    reply->set_message(prefix + request->operation());
    return Status::OK;
  }
};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  SpatialServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char** argv) {
  RunServer();

  return 0;
}
