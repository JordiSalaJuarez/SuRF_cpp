#include <benchmark/benchmark.h>
// #include <gperftools/profiler.h>
#include "utils.h"
#include "../surf/surf.h"

auto BM_PointQueryLoudsDense = [](benchmark::State& state, auto keys_insert, auto keys_query) {
    auto keys = get_input_data(state.range());
    auto louds_dense = yas::LoudsDense<suffix::Suffix>(keys_insert);
    for (auto _ : state){
        for (auto const & key : keys_query){
            auto found = louds_dense.look_up(key);
            benchmark::DoNotOptimize(found);
            benchmark::DoNotOptimize(louds_dense);
        }
    }
    state.counters["Size(MB)"] = louds_dense.get_memory_usage() / 1024 / 1024 ;
};

auto BM_AccessBitLoudsDense = [](benchmark::State& state) {
    auto keys = get_input_data(state.range());
    auto louds_dense = yas::LoudsDense<suffix::Suffix>(keys);
    for (auto _ : state){
        std::size_t pos = rand()%100000;
        auto found = louds_dense.has_child[pos / 255][pos & (255-1)];
        benchmark::DoNotOptimize(found);
        benchmark::DoNotOptimize(louds_dense);
    }
    state.counters["Size(MB)"] = louds_dense.get_memory_usage() / 1024 / 1024 ;
};

auto BM_RankLoudsDense = [](benchmark::State& state) {
    auto keys = get_input_data(state.range());
    auto louds_dense = yas::LoudsDense<suffix::Suffix>(keys);
    for (auto _ : state){
        std::size_t pos = rand()%10000000;
        auto found = louds_dense.rank_c(pos);
        benchmark::DoNotOptimize(found);
        benchmark::DoNotOptimize(louds_dense);
    }
    state.counters["Size(MB)"] = louds_dense.get_memory_usage() / 1024 / 1024 ;
};
