#ifndef COMMON_H
#define COMMON_H

#include <syncstream>

#include "../BitSTL/parallel/sort_paral.h"

#include "../BitSTL/threadsafe/stack_ts.h"
#include "../BitSTL/threadsafe/queue_ts.h"
#include "../BitSTL/threadsafe/unordered_map_ts.h"

#if 1
#define DEBUGGING
#endif

#ifdef DEBUGGING
#define DEBUG std::osyncstream(std::cout)
#endif

#define LOG (std::cout)

using namespace bitstl;

using std::chrono::steady_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;

#endif // !COMMON_H

