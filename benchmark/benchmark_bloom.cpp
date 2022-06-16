#include "bloom.hpp"
#include <cstdint>
#include <iterator>
#include <numeric>
#include <string>
#include <tlx/container/btree_set.hpp>
#include <benchmark/benchmark.h>
#include "utils.h"
#include "../surf/surf.h"


// namespace benchmark::bloomfilter{
//     using namespace std;
//     using utils::Dataset;
//     using utils::Kind;
//     using utils::Type;
//     using utils::Distribution;

//     auto BM_ConstructionBloomFilter(benchmark::State& state) -> void {
//         Type type{state.range(0)};
//         Distribution dist{state.range(1)};
//         size_t size(state.range(2));
//         size_t bits_per_key(state.range(3));
//         auto keys_insert = get_data(size, Kind::Insert, type, dist);
//         for (auto _ : state) { 
//             BloomFilter filter(bits_per_key);
//             string filter_data;
//             filter.CreateFilter(keys_insert, std::size(keys_insert), &filter_data);
//             benchmark::DoNotOptimize(filter);
//             benchmark::DoNotOptimize(filter_data);
//         }
//         BloomFilter filter(bits_per_key);
//         string filter_data;
//         filter.CreateFilter(keys_insert, std::size(keys_insert), &filter_data);
//         state.SetLabel(fmt::format("Type={},Distribution={},Size={},Bits={}", to_string(type), to_string(dist), size, bits_per_key).c_str());
//         state.counters["Size"] = benchmark::Counter(std::size(filter_data) + sizeof(BloomFilter), benchmark::Counter::kDefaults, benchmark::Counter::kIs1024) ;
//         state.counters["NKeys"] = benchmark::Counter(size , benchmark::Counter::kDefaults, benchmark::Counter::kIs1000);
//         state.counters["Distribution"] = dist ;
//         state.counters["Type"] = type ;
//         state.counters["Bits"] = bits_per_key;
//         state.counters["OriginalSize"] = [&](){
//             auto sum = 0UL;
//             for (auto const & key : keys_insert) sum += std::size(key);
//             return sum;   
//         }();
//         state.counters["Ops"] = benchmark::Counter(std::size(keys_insert), benchmark::Counter::kIsIterationInvariantRate);
//         size_t bytes_iter = 0UL;
//         for(auto const & key : keys_insert) bytes_iter += std::size(key);
//         state.counters["Bytes"] = benchmark::Counter(bytes_iter, benchmark::Counter::kIsIterationInvariantRate, benchmark::Counter::kIs1024);
//     };

//     static auto BM_PointQueryBloomFilter(benchmark::State& state) -> void {
//         Type type{state.range(0)};
//         Distribution dist{state.range(1)};
//         size_t size(state.range(2));
//         size_t bits_per_key(state.range(3));
//         auto keys_insert = get_data(size, Kind::Insert, type, dist);
//         auto keys_query = get_data(size, Kind::Query, type, dist);
//         BloomFilter filter(bits_per_key);
//         string filter_data;
//         state.SetLabel(fmt::format("Type={},Distribution={},Size={},Bits={}", to_string(type), to_string(dist), size, bits_per_key).c_str());
//         filter.CreateFilter(keys_insert, std::size(keys_insert), &filter_data);
//         for (auto _ : state){
//             for (auto const & key : keys_query){
//                 auto found = filter.KeyMayMatch(key,filter_data);
//                 benchmark::DoNotOptimize(found);
//             }
//         }

//         auto db = yas::Surf<suffix::SuffixArray<kind::Suffix>>(keys_insert);
//         auto p = 0UL;
//         for(auto const & key : keys_query){
//             if (filter.KeyMayMatch(key, filter_data)) {
//                 ++p;
//             }
//         }
//         auto tp = 0UL;
//         for(auto const & key : keys_query){
//             if (db.look_up(key)) {
//                 ++tp;
//             }
//         }
//         auto fp = p - tp;
//         auto tn = keys_query.size() - p;
//         state.counters["Size"] = benchmark::Counter(std::size(filter_data) + sizeof(BloomFilter), benchmark::Counter::kDefaults, benchmark::Counter::kIs1024) ;
//         state.counters["NKeys"] = benchmark::Counter(size , benchmark::Counter::kDefaults, benchmark::Counter::kIs1000);
//         state.counters["Distribution"] = dist ;
//         state.counters["Type"] = type ;
//         state.counters["Bits"] = bits_per_key;
//         state.counters["OriginalSize"] = [&](){
//             auto sum = 0UL;
//             for (auto const & key : keys_insert) sum += std::size(key);
//             return sum;   
//         }();
//         state.counters["FPR"] = fp / (tn + fp + 0.0);
//         state.counters["Ops"] = benchmark::Counter(std::size(keys_query), benchmark::Counter::kIsIterationInvariantRate);
//         size_t bytes_iter = 0UL;
//         for(auto const & key : keys_query) bytes_iter += std::size(key);
//         state.counters["Bytes"] = benchmark::Counter(bytes_iter, benchmark::Counter::kIsIterationInvariantRate, benchmark::Counter::kIs1024);
//     };


//     template<size_t Bits, Dataset Kind, size_t Size>
//     static auto point_query(benchmark::State& state) -> void {
//         using namespace utils;
//         using namespace benchmark;
//         auto keys = get_keys(Kind, Size);
//         auto [keys_insert, keys_query] = split(keys);
//         keys_insert = sorted(keys_insert);
//         BloomFilter filter(Bits);
//         string filter_data;
//         filter.CreateFilter(keys_insert, keys_insert.size(), &filter_data);
//         auto db = yas::Surf<suffix::SuffixArray<kind::Suffix>>(keys_insert);
//         Stats stats{};
//         for (auto _ : state){
//             for (auto const & key : keys_query){
//                 auto found = filter.KeyMayMatch(key,filter_data);
//                 benchmark::DoNotOptimize(found);
//             }
//         }
//         auto p = 0UL;
//         for(auto const & key : keys_query){
//             if (filter.KeyMayMatch(key, filter_data)) {
//                 ++stats.p;
//             }
//         }
//         auto tp = 0UL;
//         for(auto const & key : keys_query){
//             if (db.look_up(key)) {
//                 ++stats.tp;
//             }
//         }
//         stats.ds_size_bytes = filter_data.size() + sizeof(BloomFilter);
//         stats.keys_insert = keys_insert;
//         stats.keys_query = keys_query;
//         stats.bits = Bits;
//         fill_counters(state, stats);
//     };
//     template<size_t Bits, Dataset Kind, size_t Size>
//     static auto test_custom_bloom(benchmark::State& state) -> void {
//         using namespace utils;
//         using namespace benchmark;
//         auto keys = get_keys(Kind, Size);
//         auto [keys_insert, keys_query] = split(keys);
//         keys_insert = sorted(keys_insert);
//         suffix::SuffixBloom<Bits> filter(keys_insert);
//         auto db = yas::Surf<suffix::SuffixArray<kind::Suffix>>(keys_insert);
//         for (auto _ : state){
//             for (auto const & key : keys_query){
//                 auto found = filter.contains(0, key);
//                 benchmark::DoNotOptimize(found);
//             }
//         }
//         Stats stats;
//         for(auto const & key : keys_query){
//             if (filter.contains(0, key)) {
//                 ++stats.p;
//             }
//         }
//         auto tp = 0UL;
//         for(auto const & key : keys_query){
//             if (db.look_up(key)) {
//                 ++stats.tp;
//             }
//         }
//         stats.ds_size_bytes = filter.get_memory_usage();
//         stats.keys_insert = keys_insert;
//         stats.keys_query = keys_query;
//         fill_counters(state, stats);
//     };
//     template<size_t Bits, Dataset Kind, size_t Size>
//     static auto test_surf_bloom(benchmark::State& state) -> void {
//         using namespace utils;
//         using namespace benchmark;
//         auto keys = get_keys(Kind, Size);
//         auto [keys_insert, keys_query] = split(keys);
//         keys_insert = sorted(keys_insert);
//         yas::Surf<suffix::SuffixSuperBloom<Bits, 64>> filter(keys_insert);
//         size_t leaf;

//         // BloomFilter filter(Bits);
//         // string filter_data;
//         // filter.CreateFilter(keys_insert, keys_insert.size(), &filter_data);

//         {
//             yas::Surf<suffix::SuffixArray<kind::Suffix>> db(keys_insert);
//             Stats stats{};
//             for(auto const & key : keys_query){
//                 if (filter.traverse(key, leaf)) {
//                     ++stats.p;
//                 }
//             }
//             for(auto const & key : keys_query){
//                 if (db.look_up(key)) {
//                     ++stats.tp;
//                 }
//             }
//             stats.ds_size_bytes = filter.get_memory_usage();
//             stats.keys_insert = keys_insert;
//             stats.keys_query = keys_query;
//             fill_counters(state, stats);
//         }
//         for (auto _ : state){
//             for (auto const & key : keys_query){
//                 auto found = filter.traverse(key, leaf);
//                 benchmark::DoNotOptimize(found);
//             }
//         }
//     };
//     template<size_t Bits, Dataset Kind, size_t Size>
//     static auto test_surf_hash(benchmark::State& state) -> void {
//         using namespace utils;
//         using namespace benchmark;
//         auto keys = get_keys(Kind, Size);
//         auto [keys_insert, keys_query] = split(keys);
//         keys_insert = sorted(keys_insert);
//         size_t leaf;
//         yas::Surf<suffix::SuffixArray<kind::Hash<Bits>>> filter(keys_insert);
//         {
//             yas::Surf<suffix::SuffixArray<kind::Suffix>> db(keys_insert);
//             Stats stats{};
//             for(auto const & key : keys_query){
//                 if (filter.traverse(key, leaf)) {
//                     ++stats.p;
//                 }
//             }
//             for(auto const & key : keys_query){
//                 if (db.look_up(key)) {
//                     ++stats.tp;
//                 }
//             }
//             stats.ds_size_bytes = filter.get_memory_usage();
//             stats.keys_insert = keys_insert;
//             stats.keys_query = keys_query;
//             fill_counters(state, stats);
//         }
        
//         keys_query = utils::shuffle(keys_query);
//         for (auto _ : state){
//             for (auto const & key : keys_query){
//                 auto found = filter.traverse(key, leaf);
//                 benchmark::DoNotOptimize(found);
//             }
//         }
//     };
// }



namespace benchmark::bloom{
    using namespace utils;
    using namespace std;
    using namespace benchmark;
    template<Dataset Kind, size_t Bits>
    static auto build(benchmark::State& state) -> void {
        auto keys = get_keys(Kind);
        for (auto _ : state){ 
            BloomFilter filter(Bits);
            string filter_data;
            filter.CreateFilter(keys, keys.size(), &filter_data);
            benchmark::DoNotOptimize(filter);
            benchmark::DoNotOptimize(filter_data);
            auto surf = yas::Surf<suffix::SuffixArray<kind::Hash<Bits>>>(keys);
        }
        BloomFilter filter(Bits);
        string filter_data;
        filter.CreateFilter(keys, keys.size(), &filter_data);
        Stats stats{};
        stats.keys_insert = keys;
        stats.keys_query = keys;
        stats.ds_size_bytes = filter_data.size() + sizeof(BloomFilter);
        stats.bits = Bits;
        fill_counters(state, stats);
    }

    template<Dataset Kind, size_t Bits>
    static auto point_query(benchmark::State& state) -> void {
        auto keys = get_keys(Kind);
        auto [keys_insert, keys_query] = split(keys);
        keys_insert = sorted(keys_insert);
        BloomFilter filter(Bits);
        string filter_data;
        filter.CreateFilter(keys, keys.size(), &filter_data);
        {
            Stats stats{};
            auto db = yas::Surf<suffix::SuffixArray<kind::Suffix>>(keys_insert);
            auto p = 0UL;
            for(auto const & key : keys_query){
                if (filter.KeyMayMatch(key, filter_data)) {
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
                auto found = filter.KeyMayMatch(key, filter_data);
                benchmark::DoNotOptimize(found);
            }
        }
    }
}




// Bloom filters Benchmark

// BENCHMARK(BM_ConstructionBloomFilter)
//     ->Name("BloomFilter/Load/YCSB")
//     ->ArgsProduct({
//         {Type::Int, Type::String}, 
//         {Distribution::Uniform},
//         {100'000, 1'000'000, 10'000'000},
//         {10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20}
//         });

// BENCHMARK(BM_ConstructionBloomFilter)
//     ->Name("BloomFilter/Load/Words")
//     ->ArgsProduct({
//         {Type::String}, 
//         {Distribution::Words},
//         {125'000, 250'000, 500'000},
//         {10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20}
//         });

// BENCHMARK(BM_PointQueryBloomFilter)
//     ->Name("BloomFilter/Query/YCSB")
//     ->ArgsProduct({
//         {Type::Int, Type::String}, 
//         {Distribution::Uniform, Distribution::Zipfian},
//         {100'000, 1'000'000, 10'000'000},
//         {10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20}
//         });

// BENCHMARK(BM_PointQueryBloomFilter)
//     ->Name("BloomFilter/Query/Words")
//     ->ArgsProduct({
//         {Type::String}, 
//         {Distribution::Words},
//         {125'000, 250'000, 500'000},
//         {10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20}
//         });

namespace bloom = benchmark::bloom;
using utils::Dataset;

BENCHMARK(bloom::build<Dataset::DNA, 18>);
BENCHMARK(bloom::build<Dataset::WIKI, 18>);
BENCHMARK(bloom::build<Dataset::GEO, 18>);
BENCHMARK(bloom::build<Dataset::ZIPFIAN, 18>);

BENCHMARK(bloom::point_query<Dataset::DNA, 18>);
BENCHMARK(bloom::point_query<Dataset::WIKI, 18>);
BENCHMARK(bloom::point_query<Dataset::GEO, 18>);
BENCHMARK(bloom::point_query<Dataset::ZIPFIAN, 18>);



BENCHMARK_MAIN();