/*
 *
 * Copyright 2021 gRPC authors.
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
 * MODIFIED BY: Nathan Tibbetts, October 2024
 * ORIGINAL FILE: route_guide_callback_client.cc
 */

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <chrono>

#include <grpc/grpc.h>
#include <grpcpp/alarm.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include "bluefield_spatial.grpc.pb.h"

#include "utilities/spatial_utilities.cc"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using bluefield_spatial::SpatialOp;
using bluefield_spatial::Polygon;
using bluefield_spatial::SimpleReply;
using bluefield_spatial::SimpleRequest;
using bluefield_spatial::OpRequest;
using bluefield_spatial::OpResponse;
using bluefield_spatial::GeomReply;
using bluefield_spatial::GeomRequest;
using bluefield_spatial::IndexedRequest;
using bluefield_spatial::IndexedReply;

using std::chrono::system_clock;


enum SimpleOperation {
  PING_CHAIN, // Ping the servers, passing through the filter to get to and from the refine server
  PING_MERRY_GO_ROUND, // Ping the servers, requesting also a response directly from the distant refine server
  PING_MERRY_GO_ROUND_RECEIVE, // Ask the distant refine server for the response it's supposed to send after a Merry-Go-Round ping
  LIST, // List available datasets
};

/////// Wrapper Client /////////////////////////////////////////////////////////

class SpatialClient {
 public:
  SpatialClient(std::string target_addr, std::string distant_target_addr, bool verbose)
    : target_addr, distant_target_addr, verbose {
    // this->target_addr = target_addr;
    // this->distant_target_addr = distant_target_addr;
    // this->verbose = verbose;

    // Set up both internal clients - one for the Filter server, one for Refine.
    //   Each requires a channel, out of which the actual RPCs are created. 
    //   This channel models a connection to the endpoints specified.
    //   We indicate that the channel isn't authenticated (use of InsecureChannelCredentials()).
    client = GRPCClient(
      grpc::CreateChannel(target_addr, grpc::InsecureChannelCredentials())
    );
    distant_client = GRPCClient(
      grpc::CreateChannel(distant_target_addr, grpc::InsecureChannelCredentials())
    );

    // Run pings
    if (verbose) ping();
  }

  bool ping() {
    // Make sure the filter-refine pair is setup
    std::string result = client.sendSimple(PING_CHAIN) ? "UP" : "DOWN";
    std::cout << "Filter-Refine Servers STATUS: " << reply << std::endl;

    // Make sure we also have a direct link to the distant refine server;
    //  Since this is a client, not a server, this requires also sending to that
    //  distant (refine) server, which will only return right on this command if
    //  it first received a request from the middle (filter) server.
    bool first_step = client.sendSimple(PING_MERRY_GO_ROUND);
    bool second_step = distant_client.sendSimple(PING_MERRY_GO_ROUND_RECEIVE);
    reply = (first_step && second_step) ? "UP" : "DOWN";
    std::cout << "Merry-Go-Round Connection STATUS: " << reply << std::endl;
  }

 private:
  std::string target_addr;
  std::string distant_target_addr;
  GRPCClient client;
  GRPCClient distant_client;
  bool verbose;
}


/////// GRPC Client code ///////////////////////////////////////////////////////

class GRPCClient {
 public:
  GRPCClient(std::shared_ptr<Channel> channel) // TODO: Can add parameters
    : stub_(SpatialOp::NewStub(channel)) {
      // TODO: Init
    }

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  bool sendSimple(const SimpleOperation op) {
    // Data we are sending to the server.
    SimpleRequest request;
    request.set_operation(op);

    // Container for the data we expect from the server.
    SimpleReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->SayMessage(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      std::cout << reply.message() << std::endl;
      return true;
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl << "RPC FAILED" << std::endl;
      return false;
    }
  }

  bool sendRequestForStream() { // Don't worry about merry-go-round for now.
    return False
  }

  //// Being able to send shapes from the client is a more complex use case than a join request, so we're not doing that yet.
  // bool sendBidiStream() {
  //   return False
  // }

 private:
  std::unique_ptr<SpatialOp::Stub> stub_;
};


class RouteGuideClient {
 public:
  RouteGuideClient(std::shared_ptr<Channel> channel, const std::string& db)
      : stub_(RouteGuide::NewStub(channel)) {
    routeguide::ParseDb(db, &feature_list_);
  }

  void GetFeature() {
    Point point;
    Feature feature;
    point = MakePoint(409146138, -746188906);
    GetOneFeature(point, &feature);
    point = MakePoint(0, 0);
    GetOneFeature(point, &feature);
  }

  void ListFeatures() {
    routeguide::Rectangle rect;
    Feature feature;

    rect.mutable_lo()->set_latitude(400000000);
    rect.mutable_lo()->set_longitude(-750000000);
    rect.mutable_hi()->set_latitude(420000000);
    rect.mutable_hi()->set_longitude(-730000000);
    std::cout << "Looking for features between 40, -75 and 42, -73"
              << std::endl;

    class Reader : public grpc::ClientReadReactor<Feature> {
     public:
      Reader(RouteGuide::Stub* stub, float coord_factor,
             const routeguide::Rectangle& rect)
          : coord_factor_(coord_factor) {
        stub->async()->ListFeatures(&context_, &rect, this);
        StartRead(&feature_);
        StartCall();
      }
      void OnReadDone(bool ok) override {
        if (ok) {
          std::cout << "Found feature called " << feature_.name() << " at "
                    << feature_.location().latitude() / coord_factor_ << ", "
                    << feature_.location().longitude() / coord_factor_
                    << std::endl;
          StartRead(&feature_);
        }
      }
      void OnDone(const Status& s) override {
        std::unique_lock<std::mutex> l(mu_);
        status_ = s;
        done_ = true;
        cv_.notify_one();
      }
      Status Await() {
        std::unique_lock<std::mutex> l(mu_);
        cv_.wait(l, [this] { return done_; });
        return std::move(status_);
      }

     private:
      ClientContext context_;
      float coord_factor_;
      Feature feature_;
      std::mutex mu_;
      std::condition_variable cv_;
      Status status_;
      bool done_ = false;
    };
    Reader reader(stub_.get(), kCoordFactor_, rect);
    Status status = reader.Await();
    if (status.ok()) {
      std::cout << "ListFeatures rpc succeeded." << std::endl;
    } else {
      std::cout << "ListFeatures rpc failed." << std::endl;
    }
  }

  void RouteChat() {
    class Chatter : public grpc::ClientBidiReactor<RouteNote, RouteNote> {
     public:
      explicit Chatter(RouteGuide::Stub* stub)
          : notes_{MakeRouteNote("First message", 0, 0),
                   MakeRouteNote("Second message", 0, 1),
                   MakeRouteNote("Third message", 1, 0),
                   MakeRouteNote("Fourth message", 0, 0)},
            notes_iterator_(notes_.begin()) {
        stub->async()->RouteChat(&context_, this);
        NextWrite();
        StartRead(&server_note_);
        StartCall();
      }
      void OnWriteDone(bool /*ok*/) override { NextWrite(); }
      void OnReadDone(bool ok) override {
        if (ok) {
          std::cout << "Got message " << server_note_.message() << " at "
                    << server_note_.location().latitude() << ", "
                    << server_note_.location().longitude() << std::endl;
          StartRead(&server_note_);
        }
      }
      void OnDone(const Status& s) override {
        std::unique_lock<std::mutex> l(mu_);
        status_ = s;
        done_ = true;
        cv_.notify_one();
      }
      Status Await() {
        std::unique_lock<std::mutex> l(mu_);
        cv_.wait(l, [this] { return done_; });
        return std::move(status_);
      }

     private:
      void NextWrite() {
        if (notes_iterator_ != notes_.end()) {
          const auto& note = *notes_iterator_;
          std::cout << "Sending message " << note.message() << " at "
                    << note.location().latitude() << ", "
                    << note.location().longitude() << std::endl;
          StartWrite(&note);
          notes_iterator_++;
        } else {
          StartWritesDone();
        }
      }
      ClientContext context_;
      const std::vector<RouteNote> notes_;
      std::vector<RouteNote>::const_iterator notes_iterator_;
      RouteNote server_note_;
      std::mutex mu_;
      std::condition_variable cv_;
      Status status_;
      bool done_ = false;
    };

    Chatter chatter(stub_.get());
    Status status = chatter.Await();
    if (!status.ok()) {
      std::cout << "RouteChat rpc failed." << std::endl;
    }
  }

 private:
  bool GetOneFeature(const Point& point, Feature* feature) {
    ClientContext context;
    bool result;
    std::mutex mu;
    std::condition_variable cv;
    bool done = false;
    stub_->async()->GetFeature(
        &context, &point, feature,
        [&result, &mu, &cv, &done, feature, this](Status status) {
          bool ret;
          if (!status.ok()) {
            std::cout << "GetFeature rpc failed." << std::endl;
            ret = false;
          } else if (!feature->has_location()) {
            std::cout << "Server returns incomplete feature." << std::endl;
            ret = false;
          } else if (feature->name().empty()) {
            std::cout << "Found no feature at "
                      << feature->location().latitude() / kCoordFactor_ << ", "
                      << feature->location().longitude() / kCoordFactor_
                      << std::endl;
            ret = true;
          } else {
            std::cout << "Found feature called " << feature->name() << " at "
                      << feature->location().latitude() / kCoordFactor_ << ", "
                      << feature->location().longitude() / kCoordFactor_
                      << std::endl;
            ret = true;
          }
          std::lock_guard<std::mutex> lock(mu);
          result = ret;
          done = true;
          cv.notify_one();
        });
    std::unique_lock<std::mutex> lock(mu);
    cv.wait(lock, [&done] { return done; });
    return result;
  }

  const float kCoordFactor_ = 10000000.0;
  std::unique_ptr<RouteGuide::Stub> stub_;
  std::vector<Feature> feature_list_;
};


///////// MAIN /////////////////////////////////////////////////////////////////

// From basic example
int main(int argc, char** argv) {
  //// Process args to find "target" server address
  std::string target_addr = "sm8680bf3.itrss.mst.edu:50051";
  std::string distant_target_addr = "sm8680.managed.mst.edu:50053";
  // std::string arg_str("--target");
  // if (argc > 1) {
  //   std::string arg_val = argv[1];
  //   size_t start_pos = arg_val.find(arg_str);
  //   if (start_pos != std::string::npos) {
  //     start_pos += arg_str.size();
  //     if (arg_val[start_pos] == '=') {
  //       target_str = arg_val.substr(start_pos + 1);
  //     } else {
  //       std::cout << "The only correct argument syntax is --target="
  //                 << std::endl;
  //       return 0;
  //     }
  //   } else {
  //     std::cout << "The only acceptable argument is --target=" << std::endl;
  //     return 0;
  //   }
  // } else {
  //  target_str = "localhost:50051";
  // }

  std::cout << "-------------- Pings --------------" << std::endl;

  // Set up the client and do pings to make sure the filter-refine pair is setup properly
  SpatialClient client(target_addr, distant_target_addr, true);

  //// Do the desired operations

  // string geomString =
  //     "POLYGON((0.6598416796696894 0.5728159243234688,0.6600047433572486 "
  //     "0.5728159243234688,0.6600047433572486 "
  //     "0.5857102696498179,0.6598416796696894 "
  //     "0.5857102696498179,0.6598416796696894 0.5728159243234688))";

  // std::cout << "-------------- GetFeature --------------" << std::endl;
  // client.GetFeature();
  // std::cout << "-------------- ListFeatures --------------" << std::endl;
  // client.ListFeatures();
  // std::cout << "-------------- RecordRoute --------------" << std::endl;
  // client.RecordRoute();
  // std::cout << "-------------- RouteChat --------------" << std::endl;
  // client.RouteChat();

  return 0;
}

/*
//// from exp2: TODO: look at this again after initial tests

int main(int argc, char** argv) {
    int bunchSize = 3000;
    string ipAddress = "192.168.0.12:50051";
    string baseLayerFile = "/global/scratch/users/satishp/data/cemet_data_indexed";
    if (argc > 1)
        bunchSize = atoi(argv[1]);
    if (argc > 2)
        ipAddress = argv[2];
    if (argc > 3)
        baseLayerFile = argv[3];
    cout << bunchSize << endl;
    cout << ipAddress << endl;
    cout << baseLayerFile << endl;

  struct timeval tv1, tv2;
  gettimeofday(&tv1, NULL);

  GreeterClient greeter(grpc::CreateChannel(
      "192.168.100.1:50051", grpc::InsecureChannelCredentials()));



  std::vector<std::thread> threads;
  vector<float> polygonArray;
  int count = 0;
  // std::ifstream input("/users/home/dkaymak/cemet_data_indexed");
  // std::ifstream input("/home/dpu/data/sports_data_indexed");
  std::ifstream input(baseLayerFile);
  for (std::string line; getline(input, line);) {
    if (line.length() > 5 && line.find("(")) {
      string id = line.substr(0, line.find(" - "));
      string geom_string = line.substr(line.find(" - ") + 3);
      if (geom_string.rfind("POLYGON", 0) == 0) {
        // cout << "Geom: " << geom_string << endl;
        count++;
        vector<float> polygon = geomToArray(geom_string);
        polygonArray.push_back(polygonArray.size() + polygon.size());
        for (auto cur = polygon.begin(); cur != polygon.end(); ++cur) {
          polygonArray.push_back(*cur);
        }
        polygon.clear();

        if (count % bunchSize == 0) {
          vector<float> polygonArray2;
          for (int i = 0; i < polygonArray.size(); i++)
            polygonArray2.push_back(polygonArray[i]);
          polygonArray.clear();
          threads.emplace_back(std::thread(sendQuery, ipAddress, polygonArray2));
          // thread queryThread(sendQuery, polygonArray);
          // cout << "Sending..." << endl;
          // std::string reply = greeter.SendQueries(polygonArray);
          // std::cout << "Result: " << reply << std::endl;
          if (count % 1000 == 0) {
            cout << count << " polygon sent!" << endl
                 << endl
                 << endl
                 << endl
                 << endl;
          }
        }
      }
    }
  }
          for (auto& th : threads) th.join();

  cout << "Count: " << count << endl;

  gettimeofday(&tv2, NULL);
  double time_spent = (double)(tv2.tv_usec - tv1.tv_usec) / 1000000 +
                      (double)(tv2.tv_sec - tv1.tv_sec);
  // cout << "Total Intersect Time:" << total_intersect_time << endl;
  cout << endl << endl << "Total Time:" << time_spent << endl;

  return 0;
}
*/