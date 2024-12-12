#include <geos_c.h>
// #include <stdarg.h>
// #include <stdlib.h>
// #include <unistd.h>

// #include <cstring>
// #include <fstream>
// #include <iostream>
// #include <memory>
// #include <queue>
// #include <string>
// #include <thread>
// #include <vector>

// #include <grpc/support/log.h>
// #include <grpcpp/grpcpp.h>
// #include "bluefield_grpc.grpc.pb.h"

using bluefield_spatial::Polygon;
// using grpc::Server;
// using grpc::ServerAsyncResponseWriter;
// using grpc::ServerBuilder;
// using grpc::ServerCompletionQueue;
// using grpc::ServerContext;
// using grpc::Status;

using namespace std;



/////// Proto class helper functions ///////////////////////////////////////////

// Helper to construct Polygon proto class
Polygon MakePolygon(long id, vector<float>& coords) {
    Polygon p;
    p.set_id(id);
    p.mutable_coord()->Reserve(coords.size());
    p.mutable_coord()->Add(coords.begin(), coords.end()); // TODO: check to make sure that this method won't delete the vector before we're done with it
    return p;
}

// // Helper to construct Geoms proto class
// Geoms MakeGeoms(const std::string& name, const vector<Polygon>& polys) { // or const map<long, Polygon>& index
//     Geoms g;
//     g.set_name(name);
//     g.mutable_polys()->CopyFrom(polys);
//     // The following is in case one wants to use a map; requires changing the proto
//     // g.set_name(name);
//     // for (auto& [key, value]: index) {
//     //     (*mutable_index())[key] = value;
//     //     // NOTE: This conversion process strikes me as inefficient
//     // }
//     return g;
// }

// // Helper to construct GeomIDs proto class
// GeomIDs MakeGeomIDs(const std::string& name, const vector<int>& ids) {
//     GeomIDs g;
//     g.set_name(name);
//     g.mutable_ids()->CopyFrom(ids);
//     return g;
// }


////// Polygon Processing //////////////////////////////////////////////////////

// Working with Substrings
std::string replaceAll(std::string str, const std::string& from,
                       const std::string& to) {
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); 
    // Handles case where 'to' is a substring of 'from'
  }
  return str;
}

// String to vector of floats
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