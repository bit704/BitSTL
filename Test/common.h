#ifndef COMMON_H
#define COMMON_H

#include <syncstream>
#include <random>
#include <iterator>
#include <cmath>
#include <numeric>

#include "parallel/sort_paral.h"
#include "threadsafe/stack_ts.h"
#include "threadsafe/queue_ts.h"
#include "threadsafe/unordered_map_ts.h"

#if 1
#define DEBUGGING
#endif

#ifdef DEBUGGING
#define DEBUG (std::osyncstream(std::cout) << "[DEBUG]")
#endif

#define LOG (std::cout << "[LOG]")

using namespace bitstl;

using std::chrono::steady_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;

#endif // !COMMON_H

