// #include <geos_c.h>
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

// static void geos_message_handler(const char* fmt, ...) {
//   va_list ap;
//   va_start(ap, fmt);
//   vprintf(fmt, ap);
//   va_end(ap);
// }

std::string replaceAll(std::string str, const std::string& from,
                       const std::string& to) {
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos +=
        to.length();  // Handles case where 'to' is a substring of 'from'
  }
  return str;
}

// float* geomToArray(string geomString) {
//   static float geomCoordinates[10] = {0};
//   geomString =
//       geomString.substr(geomString.find("(") + 2,
//                         (geomString.find(")")) - (geomString.find("(") + 2));
//   // geomString = replaceAll(geomString, " ", "");
//   cout << "Coords:" << geomString << endl;
//   int startIndex = 0, endIndex = 0, arrayIndex = 0;
//   for (int i = 0; i <= geomString.size(); i++) {
//     if (geomString[i] == ',' || geomString[i] == ' ' ||
//         i == geomString.size()) {
//       endIndex = i;
//       string temp;
//       temp.append(geomString, startIndex, endIndex - startIndex);
//       cout << "temp:" << temp << endl;
//       geomCoordinates[arrayIndex] = stof(temp);
//       arrayIndex++;
//       startIndex = endIndex + 1;
//     }
//   }
//   return geomCoordinates;
// }

vector<float> geomToArray(string geomString) {
  vector<float> geomCoordinates;
  geomString =
      geomString.substr(geomString.find("(") + 2,
                        (geomString.find(")")) - (geomString.find("(") + 2));
  // geomString = replaceAll(geomString, "  ", " "); //todo: consider other
  // cases
  geomString = replaceAll(geomString, " ", ",");
  geomString = replaceAll(geomString, ",,", ",");
  // cout << "Coords:" << geomString << endl;
  int startIndex = 0, endIndex = 0;
  for (int i = 0; i <= geomString.size(); i++) {
    if (geomString[i] == ',' || geomString[i] == ' ' ||
        i == geomString.size()) {
      endIndex = i;
      string temp;
      temp.append(geomString, startIndex, endIndex - startIndex);
      // cout << "temp:" << temp << endl;
      geomCoordinates.push_back(stof(temp));
      startIndex = endIndex + 1;
    }
  }
  return geomCoordinates;
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

    // string geomString =
    //     "POLYGON((0.6598416796696894 0.5728159243234688,0.6600047433572486 "
    //     "0.5728159243234688,0.6600047433572486 "
    //     "0.5857102696498179,0.6598416796696894 "
    //     "0.5857102696498179,0.6598416796696894 0.5728159243234688))";

    // vector<float> p;
    // p = geomToArray(geomString);

    // for (auto cur = p.begin(); cur != p.end(); ++cur) {
    //   request.add_geom(*cur);
    // }

    // for (int i = 0; i < 10; i++) {
    //   cout << "*(p + " << i << ") : ";
    //   cout << *(p + i) << endl;
    //   request.add_geom(*(p + i));
    // }

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

  std::string SendQueries(const vector<float> polygonArray) {
    // Data we are sending to the server.
    HelloRequest request;

    // cout << "look" << endl;
    // for (auto cur = polygonArray.begin(); cur != polygonArray.end(); ++cur) {
    //   cout << *cur << endl;
    // }
    // cout << "taam" << endl;

    for (auto cur = polygonArray.begin(); cur != polygonArray.end(); ++cur) {
      request.add_geom(*cur);
    }

    HelloReply reply;
    ClientContext context;
    CompletionQueue cq;
    Status status;

    std::unique_ptr<ClientAsyncResponseReader<HelloReply> > rpc(
        stub_->AsyncSayHello(&context, request, &cq));

    rpc->Finish(&reply, &status, (void*)1);
    void* got_tag;
    bool ok = false;
    GPR_ASSERT(cq.Next(&got_tag, &ok));

    GPR_ASSERT(got_tag == (void*)1);
    GPR_ASSERT(ok);

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

void sendQuery(string ipAddress, vector<float> polygonArray) {
  // GreeterClient greeter(grpc::CreateChannel(
  //     "192.168.100.2:50051", grpc::InsecureChannelCredentials()));
  GreeterClient greeter(grpc::CreateChannel(
      ipAddress, grpc::InsecureChannelCredentials()));
  // GreeterClient greeter(grpc::CreateChannel(
  //     "192.168.3.201:50051", grpc::InsecureChannelCredentials()));
  cout << "Sending..." << endl;
  std::string reply = greeter.SendQueries(polygonArray);
  std::cout << "Result: " << reply << std::endl;
}

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
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint (in this case,
  // localhost at port 50051). We indicate that the channel isn't authenticated
  // (use of InsecureChannelCredentials()).

  // GEOSContextHandle_t ctx = GEOS_init_r();
  // GEOSContext_setNoticeHandler_r(ctx, geos_message_handler);
  // GEOSContext_setErrorHandler_r(ctx, geos_message_handler);
  // cout << "GEOS version: %s\n" << GEOSversion() << endl;

  struct timeval tv1, tv2;
  gettimeofday(&tv1, NULL);

  // GreeterClient greeter(grpc::CreateChannel(
  //     "192.168.100.2:50051", grpc::InsecureChannelCredentials()));

  // std::ifstream input("/home/dpu/data/cemet_data_indexed");
  // int count = 0;
  // string queryGeoms = "";
  // for (std::string line; getline(input, line);) {
  //   if (line.length() > 5 && line.find("(")) {
  //     count++;
  //     string id = line.substr(0, line.find(" - "));
  //     string geom_string = line.substr(line.find(" - ") + 3);
  //     queryGeoms = queryGeoms + "\n" + geom_string;

  //     if (count % 10 == 0) {
  //       // cout << "Sending " << queryGeoms << " ..." << endl;
  //       cout << "Sending..." << endl;
  //       std::string reply = greeter.SayHello(queryGeoms);
  //       std::cout << "Result: " << reply << std::endl;
  //       queryGeoms = "";
  //     }
  //   }
  // }

  // string geomString =
  //     "POLYGON((0.6598416796696894 0.5728159243234688,0.6600047433572486 "
  //     "0.5728159243234688,0.6600047433572486 "
  //     "0.5857102696498179,0.6598416796696894 "
  //     "0.5857102696498179,0.6598416796696894 0.5728159243234688))";

  // float* p;
  // p = geomToArray(geomString);
  // for (int i = 0; i < 10; i++) {
  //   cout << "*(p + " << i << ") : ";
  //   cout << *(p + i) << endl;
  // }

  // GreeterClient greeter(grpc::CreateChannel(
  //     "134.48.13.152:50051", grpc::InsecureChannelCredentials()));
  GreeterClient greeter(grpc::CreateChannel(
      "192.168.100.1:50051", grpc::InsecureChannelCredentials()));

  // string geomString =
  //     "POLYGON((0.6598416796696894 0.5728159243234688,0.6600047433572486 "
  //     "0.5728159243234688,0.6600047433572486 "
  //     "0.5857102696498179,0.6598416796696894 "
  //     "0.5857102696498179,0.6598416796696894 0.5728159243234688))";

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

  //   vector<float> polygonArray;
  // int count = 0;
  // // std::ifstream input("/users/home/dkaymak/cemet_data_indexed");
  // std::ifstream input("/home/dpu/data/lakes_data");
  // for (std::string line; getline(input, line);) {
  //   if (line.length() > 5 && line.find("(")) {
  //     // string id = line.substr(0, line.find(" - "));
  //     // string geom_string = line.substr(line.find(" - ") + 3);
  //     string geom_string = line;
  //     if (geom_string.rfind("POLYGON", 0) == 0) {
  //       // cout << "Geom: " << geom_string << endl;
  //       count++;
  //       vector<float> polygon = geomToArray(geom_string);
  //       polygonArray.push_back(polygonArray.size() + polygon.size());
  //       for (auto cur = polygon.begin(); cur != polygon.end(); ++cur) {
  //         polygonArray.push_back(*cur);
  //       }
  //       polygon.clear();

  //       if (count % 600 == 0) {
  //         cout << "Sending..." << endl;
  //         std::string reply = greeter.SendQueries(polygonArray);
  //         std::cout << "Result: " << reply << std::endl;
  //         polygonArray.clear();
  //         if (count % 1000 == 0) {
  //           cout << count << " polygon sent!" << endl
  //                << endl
  //                << endl
  //                << endl
  //                << endl;
  //         }
  //       }
  //     }
  //   }
  // }
  // cout << "Count: " << count << endl;

  // finishGEOS_r(ctx);

  gettimeofday(&tv2, NULL);
  double time_spent = (double)(tv2.tv_usec - tv1.tv_usec) / 1000000 +
                      (double)(tv2.tv_sec - tv1.tv_sec);
  // cout << "Total Intersect Time:" << total_intersect_time << endl;
  cout << endl << endl << "Total Time:" << time_spent << endl;

  return 0;
}