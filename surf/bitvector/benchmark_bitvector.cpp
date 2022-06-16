#include <benchmark/benchmark.h>
#include <cstdint>
#include "bitvector.h"

using benchmark::DoNotOptimize;

static void popcount_512(benchmark::State &state) {

  for (auto _ : state) {
    size_t ans = popcount(Word512{
        .i = {rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand()}});
    DoNotOptimize(ans);
  }
}

static void popcount_64(benchmark::State & state){
    
    for (auto _ : state){
        size_t ans = popcount(Word64{rand()});
        DoNotOptimize(ans);
    }
}




BENCHMARK(popcount_512);
BENCHMARK(popcount_64);
BENCHMARK_MAIN();