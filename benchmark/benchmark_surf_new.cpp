#include <algorithm>
#include <benchmark/benchmark.h>
#include <cstddef>
#include <iterator>
#include <random>
#include "utils.h"
#include <execution>
#include "../surf/surf.h"

namespace benchmark{
    namespace SuRF {
        using namespace utils;
        using namespace std;
        using namespace benchmark;
        template<Dataset Kind, size_t Bits>
        static auto build(benchmark::State& state) -> void {
            auto keys = get_keys(Kind);
            sorted(keys);
            for (auto _ : state){ 
                auto surf = yas::Surf<suffix::SuffixArray<kind::Hash<Bits>>>(keys);
            }
            auto surf = yas::Surf<suffix::SuffixArray<kind::Hash<Bits>>>(keys);
            Stats stats{};
            stats.keys_insert = keys;
            stats.keys_query = keys;
            stats.ds_size_bytes = surf.get_memory_usage();
            stats.bits = Bits;
            fill_counters(state, stats);
        }

        template<Dataset Kind, size_t Bits>
        static auto point_query(benchmark::State& state) -> void {
            auto keys = get_keys(Kind);
            auto [keys_insert, keys_query] = split(keys);
            keys_insert = sorted(keys_insert);
            auto surf = yas::Surf<suffix::SuffixArray<kind::Hash<Bits>>>(keys_insert);
            size_t leaf;
            {
                Stats stats{};
                auto db = yas::Surf<suffix::SuffixArray<kind::Suffix>>(keys_insert);
                auto p = 0UL;
                for(auto const & key : keys_query){
                    if (surf.traverse(key, leaf)) {
                        ++stats.p;
                    }
                }
                auto tp = 0UL;
                for(auto const & key : keys_query){
                    if (db.look_up(key)) {
                        ++stats.tp;
                    }
                }
                stats.ds_size_bytes = surf.get_memory_usage();
                stats.keys_insert = keys_insert;
                stats.keys_query = keys_query;
                stats.bits = Bits;
                fill_counters(state, stats);
            }
            for (auto _ : state) {
                for (auto const & key : keys_query){
                    auto found = surf.traverse(key, leaf);
                    benchmark::DoNotOptimize(found);
                }
            }

        }

        template<Dataset Kind, size_t Bits>
        static auto range_query(benchmark::State& state) -> void {
            auto keys = get_keys(Kind);
            auto [keys_insert, keys_query] = split(keys);
            keys_insert = sorted(keys_insert);
            auto surf = yas::LoudsSparse<suffix::SuffixArray<kind::Hash<Bits>>>(keys_insert);
            size_t leaf;
            string from{};
            string to{};
            {
                Stats stats{};
                
                auto db = yas::LoudsSparse<suffix::SuffixArray<kind::Suffix>>(keys_insert);
                for(auto const & key : keys_query){
                    range_key(key, 1, from, to);
                    auto ans = surf.ub(from);
                    if ( from <= ans && ans <= to) {
                        ++stats.p;
                    }
                }
                for(auto const & key : keys_query){
                    range_key(key, 1, from, to);
                    auto ans = db.ub(from);
                    if ( from < ans && ans < to) {
                        ++stats.tp;
                    }
                }
                stats.ds_size_bytes = surf.get_memory_usage();
                stats.keys_insert = keys_insert;
                stats.keys_query = keys_query;
                stats.bits = Bits;
                fill_counters(state, stats);
            }
            for (auto _ : state) {
                for (auto const & key : keys_query){
                    range_key(key, 1, from, to);
                    auto ans = surf.ub(from);
                    auto found = from < ans && ans < to;
                    benchmark::DoNotOptimize(found);
                }
            }
        }
    }
}

namespace SuRF = benchmark::SuRF;
using utils::Dataset;


BENCHMARK(SuRF::build<Dataset::DNA, 6>);
BENCHMARK(SuRF::build<Dataset::WIKI, 6>);
BENCHMARK(SuRF::build<Dataset::GEO, 6>);
BENCHMARK(SuRF::build<Dataset::ZIPFIAN, 6>);

BENCHMARK(SuRF::point_query<Dataset::DNA, 6>);
BENCHMARK(SuRF::point_query<Dataset::WIKI, 6>);
BENCHMARK(SuRF::point_query<Dataset::GEO, 6>);
BENCHMARK(SuRF::point_query<Dataset::ZIPFIAN, 6>);

BENCHMARK(SuRF::range_query<Dataset::DNA, 6>);
BENCHMARK(SuRF::range_query<Dataset::WIKI, 6>);
BENCHMARK(SuRF::range_query<Dataset::GEO, 6>);
BENCHMARK(SuRF::range_query<Dataset::ZIPFIAN, 6>);

BENCHMARK_MAIN();




// Surf new


// auto x = []<size_t... Bits>() {
//   ((benchmark::internal::RegisterBenchmarkInternal(new benchmark::internal::FunctionBenchmark("", 
//       &BM_ConstructionSurf<suffix::Hash<Bits>>)))
//                               ->Name(fmt::format("Surfnew/Load/YCSB/Bits/{}",Bits)) // Gotta love constexpr
//                               ->ArgsProduct({{Type::Int, Type::String},
//                                              {Distribution::Uniform},
//                                              {100'000, 1'000'000, 10'000'000}}),
//    ...);
//   return 1;
// }.operator()<1, 2, 3, 4, 5, 6, 7, 8, 9, 10>(); // The more you know


// auto y = []<size_t... Bits>() {
//   ((benchmark::internal::RegisterBenchmarkInternal(new benchmark::internal::FunctionBenchmark("", 
//       &BM_ConstructionSurf<suffix::Hash<Bits>>)))
//                               ->Name(fmt::format("Surfnew/Load/Words/Bits/{}",Bits)) // Gotta love constexpr
//                               ->ArgsProduct({{Type::String},
//                                              {Distribution::Words},
//                                              {125'000, 250'000, 500'000}}),
//    ...);
//   return 1;
// }.operator()<1, 2, 3, 4, 5, 6, 7, 8, 9, 10>(); // The more you know



// auto z = []<size_t... Bits>() {
//   ((benchmark::internal::RegisterBenchmarkInternal(new benchmark::internal::FunctionBenchmark("", 
//       &BM_PointQuerySurf<suffix::Hash<Bits>>)))
//                               ->Name(fmt::format("Surfnew/Query/YCSB/Bits/{}",Bits)) // Gotta love constexpr
//                               ->ArgsProduct({{Type::Int, Type::String},
//                                              {Distribution::Uniform, Distribution::Zipfian},
//                                              {100'000, 1'000'000, 10'000'000}}),
//    ...);
//   return 1;
// }.operator()<1, 2, 3, 4, 5, 6, 7, 8, 9, 10>(); // The more you know



// auto w = []<size_t... Bits>() {
//   ((benchmark::internal::RegisterBenchmarkInternal(new benchmark::internal::FunctionBenchmark("", 
//       &BM_PointQuerySurf<suffix::Hash<Bits>>)))
//                               ->Name(fmt::format("Surfnew/Query/Words/Bits/{}",Bits)) // Gotta love constexpr
//                               ->ArgsProduct({
//                                     {Type::String},
//                                     {Distribution::Words},
//                                     {125'000, 250'000, 500'000}}),
//    ...);
//   return 1;
// }.operator()<1, 2, 3, 4, 5, 6, 7, 8, 9, 10>(); // The more you know


// auto w = []<size_t... Bits>() {
//   ((benchmark::internal::RegisterBenchmarkInternal(new benchmark::internal::FunctionBenchmark("", 
//       &BM_PointQuerySurf<suffix::SuffixArray<kind::Hash<Bits>>>)))
//                               ->Name(fmt::format("Surfnew/Query/Words/Bits/{}",Bits))
//                               ->ArgsProduct({
//                                     {Type::String},
//                                     {Distribution::Words},
//                                     {125'000}}),
//    ...);
//   return 1;
// }.operator()<4>(); // The more you know


// auto x = []<size_t... Bits>() {
//   ((benchmark::internal::RegisterBenchmarkInternal(new benchmark::internal::FunctionBenchmark("", 
//       &benchmark::SuRF::point_query<suffix::SuffixSuperBloom<Bits, 1024>>)))
//                               ->Name(fmt::format("SuffixSuperBloom<1024>/{}",Bits))
//                               ->ArgsProduct({
//                                     {utils::Type::String},
//                                     {utils::Distribution::Zipfian},
//                                     {10'000'000}}),
//    ...);
//   return 1;
// }.operator()<6>(); // The more you know

// auto y = []<size_t... Bits>() {
//   ((benchmark::internal::RegisterBenchmarkInternal(new benchmark::internal::FunctionBenchmark("", 
//       &benchmark::SuRF::point_query<suffix::SuffixSuperBloom<Bits, 1024 * 1024>>)))
//                               ->Name(fmt::format("SuffixSuperBloom<512>/{}",Bits))
//                               ->ArgsProduct({
//                                     {utils::Type::String},
//                                     {utils::Distribution::Zipfian},
//                                     {10'000'000}}),
//    ...);
//   return 1;
// }.operator()<6>(); // The more you know


// auto z = []<size_t... Bits>() {
//   ((benchmark::internal::RegisterBenchmarkInternal(new benchmark::internal::FunctionBenchmark("", 
//       &benchmark::SuRF::point_query<suffix::SuffixBloom<Bits>>)))
//                               ->Name(fmt::format("SuffixSuperBloom<512>/{}",Bits))
//                               ->ArgsProduct({
//                                     {utils::Type::String},
//                                     {utils::Distribution::Zipfian},
//                                     {10'000'000}}),
//    ...);
//   return 1;
// }.operator()<6>(); // The more you know

// auto y = []<size_t... Bits>() {
//   ((benchmark::internal::RegisterBenchmarkInternal(new benchmark::internal::FunctionBenchmark("", 
//       &benchmark::SuRF::point_query<suffix::SuffixArray<kind::Hash<Bits>>>)))
//                               ->Name(fmt::format("SuffixArray/{}",Bits))
//                               ->ArgsProduct({
//                                     {utils::Type::String},
//                                     {utils::Distribution::Zipfian},
//                                     {100'000'000}}),
//    ...);
//   return 1;
// }.operator()<6>(); // The more you know