/**
 * File: mapreduce-reducer.cc
 * --------------------------
 * Presents the implementation of the MapReduceReducer class,
 * which is charged with the responsibility of collating all of the
 * intermediate files for a given hash number, sorting that collation,
 * grouping the sorted collation by key, and then pressing that result
 * through the reducer executable.
 *
 * See the documentation in mapreduce-reducer.h for more information.
 */

#include "mapreduce-reducer.h"
using namespace std;

MapReduceReducer::MapReduceReducer(const string& serverHost, unsigned short serverPort,
                                   const string& cwd, const string& executable, const string& outputPath) : 
  MapReduceWorker(serverHost, serverPort, cwd, executable, outputPath) {}

void MapReduceReducer::reduce() const {}
