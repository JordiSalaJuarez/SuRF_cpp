#include <benchmark/benchmark.h>
#include "utils.h"
#include "../surf_paper/include/surf.hpp"




namespace benchmark::louds_sparse_paper{
    using namespace utils;
    using namespace benchmark;
    template<size_t Bits, Dataset Kind, size_t Size>
    static auto point_query(benchmark::State& state) {
        auto keys = get_keys(Kind, Size);
        surf::SuRFBuilder builder(false, 0, surf::SuffixType::kNone, 0, 0);
        builder.build(keys);
        auto louds_sparse = surf::LoudsSparse(&builder);
        for (auto _ : state){
            for (auto const & key : keys){
                benchmark::DoNotOptimize(louds_sparse.lookupKey(key, 0));
            }    
        }
        state.counters["Ops"] = benchmark::Counter(keys.size(), benchmark::Counter::kIsIterationInvariantRate);
        state.counters["Bytes"] = benchmark::Counter(utils::reduce(keys, [](auto acc, auto key){return acc + key.size();}, 0UL), 
                benchmark::Counter::kIsIterationInvariantRate, benchmark::Counter::kIs1024);
        state.counters["Size(MB)"] = louds_sparse.getMemoryUsage() / 1024 / 1024 ;
    }
    template<size_t Bits, Dataset Kind, size_t Size>
    static auto bit_access(benchmark::State& state) {
        auto keys = get_input_data(state.range());
        surf::SuRFBuilder builder(false, 0, surf::SuffixType::kNone, 0, 0);
        builder.build(keys);
        auto louds_sparse = surf::LoudsSparse(&builder);
        for (auto _ : state){
            benchmark::DoNotOptimize(louds_sparse.child_indicator_bits_->readBit(rand()%100000));
        }
        state.counters["Ops"] = benchmark::Counter(keys.size(), benchmark::Counter::kIsIterationInvariantRate);
        state.counters["Bytes"] = benchmark::Counter(utils::reduce(keys, [](auto acc, auto key){return acc + key.size();}, 0UL), 
                benchmark::Counter::kIsIterationInvariantRate, benchmark::Counter::kIs1024);
        state.counters["Size(MB)"] = louds_sparse.getMemoryUsage() / 1024 / 1024 ;
    }

}

BENCHMARK(benchmark::louds_sparse_paper::point_query<10, utils::Dataset::DNA, 1'000'000>);
