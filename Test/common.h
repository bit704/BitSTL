#ifndef COMMON_H
#define COMMON_H

#include "pch.h"
#include "vector.h"
#include "parallel/algo_paral.h"
#include "threadsafe/stack_ts.h"
#include "threadsafe/queue_ts.h"
#include "threadsafe/unordered_map_ts.h"
#include "threadsafe/list_ts.h"

// 在项目属性中配置
#ifdef DEBUGGING
#define DEBUG_ST (std::osyncstream(std::cout) << "[DEBUG_ST] ")
#define DEBUG (std::cout << "[DEBUG] ")
#endif

#define LOG (std::cout << "[LOG] ")

using namespace bitstl;

using std::chrono::steady_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;

#define RED(S) "\033[1;31m" << S << "\033[0m"
#define YELLOW(S) "\033[1;33m" << S << "\033[0m"

#define BENCHMARK(BITSTL, STDSTL)                                 \
std::cout << "[BENCHMARK]" << std::endl;                          \
auto start = steady_clock::now();                                 \
BITSTL                                                            \
auto end = steady_clock::now();                                   \
auto time = duration_cast<milliseconds>(end - start).count();     \
std::cout << " <BITSTL> ";                                        \
std::cout << #BITSTL << " " << YELLOW(time << "ms") << std::endl; \
start = steady_clock::now();                                      \
STDSTL                                                            \
end = steady_clock::now();                                        \
time = duration_cast<milliseconds>(end - start).count();          \
std::cout << " <STDSTL> ";                                        \
std::cout << #STDSTL << " " << YELLOW(time << "ms") << std::endl

#endif // !COMMON_H

