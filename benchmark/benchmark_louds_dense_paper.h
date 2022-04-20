#include <benchmark/benchmark.h>
#include "utils.h"
#include "../surf_paper/include/surf.hpp"

static void BM_PointQueryLoudsDensePaper(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    surf::SuRFBuilder builder(true, 0, surf::SuffixType::kNone, 0, 0);
    builder.build(keys);
    auto louds_dense = surf::LoudsDense(&builder);
    // ProfilerStart("BM_PointQueryLoudsDensePaper.prof");
    for (auto _ : state){
        surf::position_t pos = 0;
        auto key = keys[rand()%size(keys)];
        auto found = louds_dense.lookupKey(key, pos);
        benchmark::DoNotOptimize(found);
        benchmark::DoNotOptimize(louds_dense);
    }
    state.counters["Size(MB)"] = louds_dense.getMemoryUsage() / 1024 / 1024 ;
    // ProfilerStop();
}

static void BM_AccessBitLoudsDensePaper(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    surf::SuRFBuilder builder(true, 0, surf::SuffixType::kNone, 0, 0);
    builder.build(keys);
    auto louds_dense = surf::LoudsDense(&builder);
    for (auto _ : state){
        surf::position_t pos = rand()%100000;
        auto found = louds_dense.child_indicator_bitmaps_->readBit(pos);
        benchmark::DoNotOptimize(found);
        benchmark::DoNotOptimize(louds_dense);
    }
    state.counters["Size(MB)"] = louds_dense.getMemoryUsage() / 1024 / 1024 ;

}

static void BM_RankLoudsDensePaper(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    surf::SuRFBuilder builder(true, 0, surf::SuffixType::kNone, 0, 0);
    builder.build(keys);
    auto louds_dense = surf::LoudsDense(&builder);
    for (auto _ : state){
        surf::position_t pos = rand()%10000000;
        auto found = louds_dense.child_indicator_bitmaps_->rank(pos);
        benchmark::DoNotOptimize(found);
        benchmark::DoNotOptimize(louds_dense);
    }
    state.counters["Size(MB)"] = louds_dense.getMemoryUsage() / 1024 / 1024 ;
}
