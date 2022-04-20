#include <benchmark/benchmark.h>
#include "utils.h"
#include "../surf_paper/include/surf.hpp"

static void BM_PointQueryLoudsSparsePaper(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    surf::SuRFBuilder builder(false, 0, surf::SuffixType::kNone, 0, 0);
    builder.build(keys);
    auto louds_sparse = surf::LoudsSparse(&builder);
    for (auto _ : state){
        benchmark::DoNotOptimize(louds_sparse.lookupKey(keys[rand()%size(keys)], 0));
    }
    state.counters["Size(MB)"] = louds_sparse.getMemoryUsage() / 1024 / 1024 ;
}

static void BM_AccessBitLoudsSparsePaper(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    surf::SuRFBuilder builder(false, 0, surf::SuffixType::kNone, 0, 0);
    builder.build(keys);
    auto louds_sparse = surf::LoudsSparse(&builder);
    for (auto _ : state){
        benchmark::DoNotOptimize(louds_sparse.child_indicator_bits_->readBit(rand()%100000));
    }
    state.counters["Size(MB)"] = louds_sparse.getMemoryUsage() / 1024 / 1024 ;
}