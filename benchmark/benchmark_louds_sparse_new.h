#include <benchmark/benchmark.h>
#include "utils.h"
#include "../surf/surf.h"



namespace benchmark::louds_sparse{
    using namespace utils;
    using namespace benchmark;
    template<size_t Bits, Dataset Kind, size_t Size>
    static auto point_query(benchmark::State& state) {
        auto keys = get_keys(Kind, Size);
        auto louds_sparse = yas::LoudsSparse<suffix::SuffixArray<kind::Suffix>>(keys);
        for (auto _ : state){
            for (auto const & key : keys){
                benchmark::DoNotOptimize(louds_sparse.look_up(key));
            }
        }
        state.counters["Ops"] = benchmark::Counter(keys.size(), benchmark::Counter::kIsIterationInvariantRate);
        state.counters["Bytes"] = benchmark::Counter(utils::reduce(keys, [](auto acc, auto key){return acc + key.size();}, 0UL), 
                benchmark::Counter::kIsIterationInvariantRate, benchmark::Counter::kIs1024);
        state.counters["Size(MB)"] = louds_sparse.get_memory_usage() / 1024 / 1024 ;
    };
};

BENCHMARK(benchmark::louds_sparse::point_query<10, utils::Dataset::DNA, 100'000'000>);

// static void BM_AccessBitLoudsSparse(benchmark::State& state) {
//     auto keys = get_input_data(state.range());
//     auto louds_sparse = yas::LoudsSparse<suffix::Suffix>(keys);
//     for (auto _ : state){
//         benchmark::DoNotOptimize(louds_sparse.has_child[rand()%100000]);
//     }
//     state.counters["Size(MB)"] = louds_sparse.get_memory_usage() / 1024 / 1024 ;
// }