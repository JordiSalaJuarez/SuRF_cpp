#include <benchmark/benchmark.h>
#include "utils.h"
#define private public
#define protected public
#include <tlx/container/btree_set.hpp>
#undef private
#undef protected

namespace benchmark::btree {
    using namespace utils;
    using namespace benchmark;
    using btree = tlx::btree_set<string>;

    auto size_bytes(btree::tree_stats stats) -> size_t{
        return stats.inner_nodes * sizeof(btree::btree_impl::InnerNode) + // Size Inner nodes
            stats.leaves * sizeof(btree::btree_impl::LeafNode); // Size leaf nodes
    }

    template<Dataset Kind>
    static auto build(State& state) -> void {
        auto keys = utils::get_keys(Kind);
        for (auto _ : state){ 
            btree tree;
            tree.bulk_load(begin(keys), end(keys));
        }
        btree tree;
        tree.bulk_load(begin(keys), end(keys));
        auto tree_stats = tree.get_stats();
        Stats stats;
        stats.ds_size_bytes = size_bytes(tree_stats);
        stats.keys_insert = keys;
        stats.keys_query = keys;
        fill_counters(state, stats);
    }

    template<Dataset Kind>
    static auto point_query(State& state) -> void {
        auto keys = utils::get_keys(Kind);
        auto [keys_insert, keys_query] = split(keys);
        btree tree;
        tree.bulk_load(begin(keys_insert), end(keys_insert));
        for (auto _ : state){
            for (auto const & key : keys_query){
                DoNotOptimize(tree.find(key));
            }
        }
        auto tree_stats = tree.get_stats();
        Stats stats;
        stats.ds_size_bytes = size_bytes(tree_stats);
        stats.keys_insert = keys_insert;
        stats.keys_query = keys_query;
        fill_counters(state, stats);
    }

    template<Dataset Kind>
    static auto range_query(State& state) -> void {
        auto keys = utils::get_keys(Kind);
        auto [keys_insert, keys_query] = split(keys);
        btree tree;
        tree.bulk_load(begin(keys_insert), end(keys_insert));
        string from, to;
        for (auto _ : state){
            for (auto const & key : keys_query){
                range_key(key, 1, from, to);
                auto low = tree.lower_bound(from);
                auto up = tree.upper_bound(from);
                bool found = low != tree.end() && up != tree.end() && *low <= key && key <= *up;
                DoNotOptimize(found);
            }
        }
        auto tree_stats = tree.get_stats();
        Stats stats;
        stats.ds_size_bytes = size_bytes(tree_stats);
        stats.keys_insert = keys_insert;
        stats.keys_query = keys_query;
        fill_counters(state, stats);
    }
}

namespace btree = benchmark::btree;
using Dataset = utils::Dataset;

// Benchmark on build
BENCHMARK(btree::build<Dataset::DNA>);
BENCHMARK(btree::build<Dataset::GEO>);
BENCHMARK(btree::build<Dataset::WIKI>);
BENCHMARK(btree::build<Dataset::ZIPFIAN>);

// Benchmark on point query
BENCHMARK(btree::point_query<Dataset::DNA>);
BENCHMARK(btree::point_query<Dataset::GEO>);
BENCHMARK(btree::point_query<Dataset::WIKI>);
BENCHMARK(btree::point_query<Dataset::ZIPFIAN>);

// Benchmark on range query
BENCHMARK(btree::range_query<Dataset::DNA>);
BENCHMARK(btree::range_query<Dataset::GEO>);
BENCHMARK(btree::range_query<Dataset::WIKI>);
BENCHMARK(btree::range_query<Dataset::ZIPFIAN>);

// Runs Benchmarks
BENCHMARK_MAIN();