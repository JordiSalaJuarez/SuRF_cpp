#include <benchmark/benchmark.h>
// #include <gperftools/profiler.h>
#include "utils.h"
#include "../surf/surf.h"

static void BM_PointQueryLoudsDense(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    auto louds_dense = yas::LoudsDense<suffix::Suffix>(keys);
    // ProfilerStart("BM_PointQueryLoudsDense.prof");
    for (auto _ : state){
        auto key = keys[rand()%size(keys)];
        auto found = louds_dense.look_up(key);
        benchmark::DoNotOptimize(found);
        benchmark::DoNotOptimize(louds_dense);
    }
    state.counters["Size(MB)"] = louds_dense.get_memory_usage() / 1024 / 1024 ;
    // ProfilerStop();
}

static void BM_AccessBitLoudsDense(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    auto louds_dense = yas::LoudsDense<suffix::Suffix>(keys);
    for (auto _ : state){
        std::size_t pos = rand()%100000;
        auto found = louds_dense.has_child[pos / 255][pos & (255-1)];
        benchmark::DoNotOptimize(found);
        benchmark::DoNotOptimize(louds_dense);
    }
    state.counters["Size(MB)"] = louds_dense.get_memory_usage() / 1024 / 1024 ;
}

static void BM_RankLoudsDense(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    auto louds_dense = yas::LoudsDense<suffix::Suffix>(keys);
    for (auto _ : state){
        std::size_t pos = rand()%10000000;
        auto found = louds_dense.rank_c(pos);
        benchmark::DoNotOptimize(found);
        benchmark::DoNotOptimize(louds_dense);
    }
    state.counters["Size(MB)"] = louds_dense.get_memory_usage() / 1024 / 1024 ;
}
