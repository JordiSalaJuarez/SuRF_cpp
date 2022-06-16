#include <benchmark/benchmark.h>
#include "../surf_paper/include/surf.hpp"
#include "../surf/surf.h"
#include "utils.h"


// static auto BM_ConstructionSurfPaper(benchmark::State& state) -> void {
//     Type type{state.range(0)};
//     Distribution dist{state.range(1)};
//     size_t size(state.range(2));
//     auto suffix_type = static_cast<surf::SuffixType>(state.range(3));
//     size_t bits_per_key(state.range(4));
//     auto keys_insert = get_data(size, Kind::Insert, type, dist);
//     for (auto _ : state){ 
//         auto surf = surf::SuRF(keys_insert, suffix_type, bits_per_key, 0);
//         surf.destroy();
//     }
//     auto surf = surf::SuRF(keys_insert, suffix_type, bits_per_key, 0);
//     state.SetLabel(fmt::format("Type={},Distribution={},Size={},Bits={}", to_string(type), to_string(dist), size, bits_per_key).c_str());
//     state.counters["Size"] = benchmark::Counter(surf.getMemoryUsage(), benchmark::Counter::kDefaults, benchmark::Counter::kIs1024) ;
//     state.counters["NKeys"] = benchmark::Counter(size , benchmark::Counter::kDefaults, benchmark::Counter::kIs1000);
//     state.counters["Distribution"] = dist ;
//     state.counters["Type"] = type ;
//     state.counters["Bits"] = bits_per_key;
//     state.counters["OriginalSize"] = [&](){
//         auto sum = 0UL;
//         for (auto const & key : keys_insert) sum += std::size(key);
//         return sum;   
//     }();
//     state.counters["Ops"] = benchmark::Counter(std::size(keys_insert), benchmark::Counter::kIsIterationInvariantRate);
//     size_t bytes_iter = 0UL;
//     for(auto const & key : keys_insert) bytes_iter += std::size(key);
//     state.counters["Bytes"] = benchmark::Counter(bytes_iter, benchmark::Counter::kIsIterationInvariantRate, benchmark::Counter::kIs1024);
//     surf.destroy();
// }

// static auto BM_PointQuerySurfPaper(benchmark::State& state) -> void {
//     Type type{state.range(0)};
//     Distribution dist{state.range(1)};
//     size_t size(state.range(2));
//     auto suffix_type = static_cast<surf::SuffixType>(state.range(3));
//     size_t bits_per_key(state.range(4));
//     auto keys_insert = get_data(size, Kind::Insert, type, dist);
//     auto keys_query = get_data(size, Kind::Query, type, dist);
//     auto surf = surf::SuRF(keys_insert, suffix_type, bits_per_key, 0);
//     for (auto _ : state){
//         for (auto const & key : keys_query){
//             benchmark::DoNotOptimize(surf.lookupKey(key));
//         }
//     }
//     state.SetLabel(fmt::format("Type={},Distribution={},Size={},Bits={}", to_string(type), to_string(dist), size, bits_per_key).c_str());
//     auto db = yas::Surf<suffix::Suffix>(keys_insert);
//     auto p = 0UL;
//     for(auto const & key : keys_query){
//         if (surf.lookupKey(key)) {
//             ++p;
//         }
//     }
//     auto tp = 0UL;
//     for(auto const & key : keys_query){
//         if (db.look_up(key)) {
//             ++tp;
//         }
//     }
//     auto fp = p - tp;
//     auto tn = keys_query.size() - p;
//     auto fp_rate = fp / (tn + fp + 0.0);
//     state.counters["Size"] = benchmark::Counter(surf.getMemoryUsage(), benchmark::Counter::kDefaults, benchmark::Counter::kIs1024) ;
//     state.counters["NKeys"] = benchmark::Counter(size , benchmark::Counter::kDefaults, benchmark::Counter::kIs1000);
//     state.counters["Distribution"] = dist ;
//     state.counters["Type"] = type ;
//     state.counters["Bits"] = bits_per_key;
//     state.counters["OriginalSize"] = [&](){
//         auto sum = 0UL;
//         for (auto const & key : keys_insert) sum += std::size(key);
//         return sum;   
//     }();
//     state.counters["FPR"] = fp / (tn + fp + 0.0);
//     state.counters["Ops"] = benchmark::Counter(std::size(keys_query), benchmark::Counter::kIsIterationInvariantRate);
//     size_t bytes_iter = 0UL;
//     for(auto const & key : keys_query) bytes_iter += std::size(key);
//     state.counters["Bytes"] = benchmark::Counter(bytes_iter, benchmark::Counter::kIsIterationInvariantRate, benchmark::Counter::kIs1024);
//     surf.destroy();
// }

// namespace benchmark::SuRFPaper{
//     using namespace utils;
//     template<size_t Bits, Dataset Kind, size_t Size>
//     static auto query(benchmark::State& state) -> void {
//         auto keys = get_keys(Kind, Size);
//         auto [keys_insert, keys_query] = split(keys);
//         keys_insert = sorted(keys_insert);
//         vector<string> keys_insert_vec{keys_insert.data(), keys_insert.data()+keys_insert.size()};
//         auto surf = surf::SuRF(keys_insert_vec, surf::SuffixType::kHash, Bits, 0);
//         keys_insert_vec.clear();
//         for (auto _ : state){
//             for (auto const & key : keys_query){
//                 benchmark::DoNotOptimize(surf.lookupKey(key));
//             }
//         }
//         auto db = yas::Surf<suffix::SuffixArray<kind::Suffix>>(keys_insert);
//         auto p = 0UL;
//         for(auto const & key : keys_query){
//             if (surf.lookupKey(key)) {
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
//         auto fp_rate = fp / (tn + fp + 0.0);
//         state.counters["Size"] = benchmark::Counter(surf.getMemoryUsage(), benchmark::Counter::kDefaults, benchmark::Counter::kIs1024) ;
//         state.counters["NKeys"] = benchmark::Counter(keys_query.size(), benchmark::Counter::kDefaults, benchmark::Counter::kIs1000);
//         state.counters["Bits"] = Bits;
//         state.counters["OriginalSize"] = utils::reduce(keys_insert, [](auto acc, auto key){return acc + key.size();}, 0UL);
//         state.counters["FPR"] = fp / (tn + fp + 0.0);
//         state.counters["Ops"] = benchmark::Counter(keys_query.size(), benchmark::Counter::kIsIterationInvariantRate);
//         state.counters["Bytes"] = benchmark::Counter(utils::reduce(keys_query, [](auto acc, auto key){return acc + key.size();}, 0UL), 
//         benchmark::Counter::kIsIterationInvariantRate, benchmark::Counter::kIs1024);
//         surf.destroy();
//     }
// }


namespace benchmark::SuRFPaper {
    using namespace utils;
    using namespace std;
    using namespace benchmark;
    template<Dataset Kind, size_t Bits>
    static auto build(benchmark::State& state) -> void {
        auto keys = get_keys(Kind);
        sort(keys);
        for (auto _ : state){ 
            auto surf = surf::SuRF(keys, surf::SuffixType::kHash, Bits, 0);
            state.PauseTiming();
            surf.destroy();
            state.ResumeTiming();
        }
        auto surf = surf::SuRF(keys, surf::SuffixType::kHash, Bits, 0);
        Stats stats{};
        stats.keys_insert = keys;
        stats.keys_query = keys;
        stats.ds_size_bytes = surf.getMemoryUsage();
        stats.bits = Bits;
        fill_counters(state, stats);
        surf.destroy();
    }

    template<Dataset Kind, size_t Bits>
    static auto point_query(benchmark::State& state) -> void {
        auto keys = get_keys(Kind);
        auto [keys_insert, keys_query] = split(keys);
        keys_insert = sorted(keys_insert);
        vector<string> keys_insert_vec(keys_insert.data(), keys_insert.data() + keys_insert.size());
        auto surf = surf::SuRF(keys_insert_vec, surf::SuffixType::kHash, Bits, 0);
        size_t leaf;
        {
            Stats stats{};
            auto db = yas::LoudsSparse<suffix::SuffixArray<kind::Suffix>>(keys_insert);
            for(auto const & key : keys_query){
                if (surf.lookupKey(key)) {
                    ++stats.p;
                }
            }
            auto tp = 0UL;
            for(auto const & key : keys_query){
                if (db.look_up(key)) {
                    ++stats.tp;
                }
            }
            stats.keys_insert = keys;
            stats.keys_query = keys;
            stats.ds_size_bytes = surf.getMemoryUsage();
            stats.bits = Bits;
            fill_counters(state, stats);
        }
        for (auto _ : state) {
            for (auto const & key : keys_query){
                auto found = surf.lookupKey(key);
                benchmark::DoNotOptimize(found);
            }
        }
        surf.destroy();
    }

    template<Dataset Kind, size_t Bits>
    static auto range_query(benchmark::State& state) -> void {
        auto keys = get_keys(Kind);
        auto [keys_insert, keys_query] = split(keys);
        keys_insert = sorted(keys_insert);
        vector<string> keys_insert_vec(keys_insert.data(), keys_insert.data() + keys_insert.size());
        auto surf = surf::SuRF(keys_insert_vec, surf::SuffixType::kHash, Bits, 0);;
        size_t leaf;
        string from{};
        string to{};
        {
            Stats stats{};
            
            auto db = yas::LoudsSparse<suffix::SuffixArray<kind::Suffix>>(keys_insert);
            auto p = 0UL;
            for(auto const & key : keys_query){
                range_key(key, 1, from, to);
                if (surf.lookupRange(from, false, to, false)) {
                    ++stats.p;
                }
            }
            auto tp = 0UL;
            for(auto const & key : keys_query){
                range_key(key, 1, from, to);
                auto ans = db.ub(from);
                if ( from < ans && ans < to) {
                    ++stats.tp;
                }
            }
            stats.ds_size_bytes = surf.getMemoryUsage();
            stats.keys_insert = keys_insert;
            stats.keys_query = keys_query;
            stats.bits = Bits;
            fill_counters(state, stats);
        }
        for (auto _ : state) {
            for (auto const & key : keys_query){
                range_key(key, 1, from, to);
                auto found = surf.lookupRange(from, false, to, false);
                benchmark::DoNotOptimize(found);
            }
        }
        surf.destroy();
    }
}

namespace SuRFPaper = benchmark::SuRFPaper;
using utils::Dataset;

BENCHMARK(SuRFPaper::build<Dataset::DNA, 6>);
BENCHMARK(SuRFPaper::build<Dataset::WIKI, 6>);
BENCHMARK(SuRFPaper::build<Dataset::GEO, 6>);
BENCHMARK(SuRFPaper::build<Dataset::ZIPFIAN, 6>);

BENCHMARK(SuRFPaper::point_query<Dataset::DNA, 6>);
BENCHMARK(SuRFPaper::point_query<Dataset::WIKI, 6>);
BENCHMARK(SuRFPaper::point_query<Dataset::GEO, 6>);
BENCHMARK(SuRFPaper::point_query<Dataset::ZIPFIAN, 6>);

BENCHMARK(SuRFPaper::range_query<Dataset::DNA, 6>);
BENCHMARK(SuRFPaper::range_query<Dataset::WIKI, 6>);
BENCHMARK(SuRFPaper::range_query<Dataset::GEO, 6>);
BENCHMARK(SuRFPaper::range_query<Dataset::ZIPFIAN, 6>);


BENCHMARK_MAIN();




// constexpr static utils::Dataset KIND = utils::Dataset::ZIPFIAN;

// BENCHMARK(benchmark::SuRFPaper::query<8, KIND, 1'000'000>)
//     ->Name("SuRFPaper/Query/Words");


// template <surf::SuffixType Suffix = surf::SuffixType::kNone, size_t Size = 0UL>
// static void BM_TraversalSurfPaper(benchmark::State& state) {
//     auto keys = get_input_data(state.range());
//     auto surf = surf::SuRF(keys, Suffix, Size, 0);
//     for (auto _ : state){
//         std::vector<std::string> words;
//         auto it = surf.moveToFirst();
//         auto end = surf.moveToLast();
//         while(it++){
//             words.push_back(it.getKey());
//         }
//         benchmark::DoNotOptimize(words);
//     }
//     state.SetBytesProcessed(state.range() * size(keys[0]) * state.iterations());
//     state.counters["Size(MB)"] = surf.getMemoryUsage() / 1024 / 1024 ;
// }






// BENCHMARK(BM_ConstructionSurfPaper)
//     ->Name("SurfPaper/Load/YCSB")
//     ->ArgsProduct({
//         {Type::String, Type::Int}, 
//         {Distribution::Uniform, Distribution::Zipfian},
//         {100'000, 1'000'000, 10'000'000},
//         {surf::SuffixType::kHash},
//         {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}});

// BENCHMARK(BM_ConstructionSurfPaper)
//     ->Name("SurfPaper/Load/Words")
//     ->ArgsProduct({
//         {Type::String},
//         {Distribution::Words},
//         {125'000, 250'000, 500'000},
//         {surf::SuffixType::kHash},
//         {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}});


// BENCHMARK(BM_PointQuerySurfPaper)
//     ->Name("SurfPaper/Query/YCSB")
//     ->ArgsProduct({
//         {Type::Int, Type::String}, 
//         {Distribution::Uniform, Distribution::Zipfian},
//         {100'000, 1'000'000, 10'000'000},
//         {surf::SuffixType::kHash},
//         {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}
//         });

// BENCHMARK(BM_PointQuerySurfPaper)
//     ->Name("SurfPaper/Query/Words")
//     ->ArgsProduct({
//         {Type::String}, 
//         {Distribution::Words},
//         {125'000, 250'000, 500'000},
//         {surf::SuffixType::kHash},
//         {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}
//         });