#include <algorithm>
#include <benchmark/benchmark.h>
#include <cstddef>
#include <iterator>
#include <random>
#include "utils.h"
#include <execution>
#include "../surf/surf.h"

namespace benchmark::FST{
    using namespace utils;
    using namespace std;
    using namespace benchmark;
    template<Dataset Kind>
    static auto build(benchmark::State& state) -> void {
        auto keys = get_keys(Kind);
        sort(keys);
        for (auto _ : state){ 
            auto surf = yas::Surf<suffix::SuffixArray<kind::Suffix>>(keys);
        }
        auto surf = yas::Surf<suffix::SuffixArray<kind::Suffix>>(keys);
        Stats stats{};
        stats.keys_insert = keys;
        stats.keys_query = keys;
        stats.ds_size_bytes = surf.get_memory_usage();
        fill_counters(state, stats);
    }

    template<Dataset Kind>
    static auto point_query(benchmark::State& state) -> void {
        auto keys = get_keys(Kind);
        auto [keys_insert, keys_query] = split(keys);
        keys_insert = sorted(keys_insert);
        auto surf = yas::Surf<suffix::SuffixArray<kind::Suffix>>(keys_insert);
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
            fill_counters(state, stats);
        }
        for (auto _ : state) {
            for (auto const & key : keys_query){
                auto found = surf.traverse(key, leaf);
                benchmark::DoNotOptimize(found);
            }
        }

    }

    template<Dataset Kind>
    static auto range_query(benchmark::State& state) -> void {
        auto keys = get_keys(Kind);
        auto [keys_insert, keys_query] = split(keys);
        keys_insert = sorted(keys_insert);
        auto surf = yas::LoudsSparse<suffix::SuffixArray<kind::Suffix>>(keys_insert);
        size_t leaf;
        string from{};
        string to{};
        {
            Stats stats{};
            
            auto db = yas::LoudsSparse<suffix::SuffixArray<kind::Suffix>>(keys_insert);
            auto p = 0UL;
            for(auto const & key : keys_query){
                range_key(key, 1, from, to);
                auto ans = surf.ub(from);
                if ( from <= ans && ans <= to) {
                    ++stats.p;
                }
            }
            auto tp = 0UL;
            for(auto const & key : keys_query){
                range_key(key, 1, from, to);
                auto ans = surf.ub(from);
                if ( from <= ans && ans <= to) {
                    ++stats.tp;
                }
            }
            stats.ds_size_bytes = surf.get_memory_usage();
            stats.keys_insert = keys_insert;
            stats.keys_query = keys_query;
            fill_counters(state, stats);
        }
        for (auto _ : state) {
            for (auto const & key : keys_query){
                range_key(key, 1, from, to);
                auto ans = surf.ub(from);
                auto found = from <= ans && ans <= to;
                benchmark::DoNotOptimize(found);
            }
        }
    }
}

namespace FST = benchmark::FST;
using utils::Dataset;


BENCHMARK(FST::build<Dataset::DNA>);
BENCHMARK(FST::build<Dataset::WIKI>);
BENCHMARK(FST::build<Dataset::GEO>);
BENCHMARK(FST::build<Dataset::ZIPFIAN>);

BENCHMARK(FST::point_query<Dataset::DNA>);
BENCHMARK(FST::point_query<Dataset::WIKI>);
BENCHMARK(FST::point_query<Dataset::GEO>);
BENCHMARK(FST::point_query<Dataset::ZIPFIAN>);

BENCHMARK(FST::range_query<Dataset::DNA>);
BENCHMARK(FST::range_query<Dataset::WIKI>);
BENCHMARK(FST::range_query<Dataset::GEO>);
BENCHMARK(FST::range_query<Dataset::ZIPFIAN>);

BENCHMARK_MAIN();