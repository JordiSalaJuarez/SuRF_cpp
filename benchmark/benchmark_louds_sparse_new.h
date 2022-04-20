#include <benchmark/benchmark.h>
#include "utils.h"
#include "../surf/surf.h"




static void BM_PointQueryLoudsSparse(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    auto louds_sparse = yas::LoudsSparse<suffix::Suffix>(keys);
    for (auto _ : state){
        benchmark::DoNotOptimize(louds_sparse.look_up(keys[rand()%size(keys)]));
    }
    state.counters["Size(MB)"] = louds_sparse.get_memory_usage() / 1024 / 1024 ;
}

static void BM_AccessBitLoudsSparse(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    auto louds_sparse = yas::LoudsSparse<suffix::Suffix>(keys);
    for (auto _ : state){
        benchmark::DoNotOptimize(louds_sparse.has_child[rand()%100000]);
    }
    state.counters["Size(MB)"] = louds_sparse.get_memory_usage() / 1024 / 1024 ;
}