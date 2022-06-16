#include <cstddef>
#include "utils.h"
#include <benchmark/benchmark.h>
// #include <gperftools/malloc_extension.h>
#define private public 
#include <tx/tx.hpp>
#undef private
#include <unistd.h>



namespace benchmark::louds {
    using namespace utils;
    using namespace benchmark;

    template<Dataset Kind>
    static auto build(State& state) -> void {
        auto keys = utils::get_keys(Kind);
        char filename[] = "/tmp/loudsXXXXXX";
        int fd = mkstemp(filename);
        for (auto _ : state){ 
            tx_tool::tx trie{};
            trie.build(keys, filename);
            trie.read(filename);
            benchmark::DoNotOptimize(trie);
        }
        close(fd);
        tx_tool::tx trie{};
        trie.build(keys, filename);
        trie.read(filename);
        FILE* infp = fopen(filename,"rb");
        fseek(infp,0,SEEK_END);
        size_t size_bytes = ftell(infp); 
        fclose(infp);
        remove(filename);
        Stats stats;
        stats.ds_size_bytes = size_bytes;
        stats.keys_insert = keys;
        stats.keys_query = keys;
        fill_counters(state, stats);
    }

    template<Dataset kind, Pattern pattern = SCAN>
    static auto point_query(State& state) -> void {
        auto keys = utils::get_keys(kind);
        auto [keys_insert, keys_query] = split(keys);
        tx_tool::tx trie{};
        char filename[] = "/tmp/loudsXXXXXX";
        int fd = mkstemp(filename);
        vector<string> keys_insert_vec(keys_insert.data(), keys_insert.data() + keys_insert.size());
        trie.build(keys_insert_vec, filename);
        trie.read(filename);
        for (auto _ : state){
            if constexpr(pattern == Pattern::SCAN){
                for (auto const & key : keys_query){
                    size_t retLen;
                    auto ret = trie.prefixSearch(key.data(),std::size(key), retLen);
                    benchmark::DoNotOptimize(ret);
                }
            } else {
                for (size_t i = 0; i < keys_query.size(); ++i){
                    state.PauseTiming();
                    auto key = keys[rand() % keys_query.size()];
                    state.ResumeTiming();
                    size_t retLen;
                    auto ret = trie.prefixSearch(key.data(),std::size(key), retLen);
                    benchmark::DoNotOptimize(ret);
                }
            }
        }
        close(fd);
        FILE* infp = fopen(filename,"rb");
        fseek(infp,0,SEEK_END);
        size_t size_bytes = ftell(infp); 
        fclose(infp);
        remove(filename);
        Stats stats;
        stats.ds_size_bytes = size_bytes;
        stats.keys_insert = keys_insert;
        stats.keys_query = keys_query;
        fill_counters(state, stats);
    }

    // template<Dataset Kind>
    // static auto range_query(State& state) -> void {
    //     auto keys = utils::get_keys(Kind);
    //     auto [keys_insert, keys_query] = split(keys);
    //     btree tree;
    //     tree.bulk_load(begin(keys_insert), end(keys_insert));
    //     string from, to;
    //     for (auto _ : state){
    //         for (auto const & key : keys_query){
    //             range_key(key, 1, from, to);
    //             auto low = tree.lower_bound(from);
    //             auto up = tree.upper_bound(from);
    //             bool found = low != tree.end() && up != tree.end() && *low <= key && key <= *up;
    //             DoNotOptimize(found);
    //         }
    //     }
    //     auto tree_stats = tree.get_stats();
    //     Stats stats;
    //     stats.ds_size_bytes = size_bytes(tree_stats);
    //     stats.keys_insert = keys_insert;
    //     stats.keys_query = keys_query;
    //     fill_counters(state, stats);
    // }
}



namespace louds = benchmark::louds;
using Dataset = utils::Dataset;
using utils::Pattern;

// Benchmark on build
BENCHMARK(louds::build<Dataset::DNA>);
BENCHMARK(louds::build<Dataset::GEO>);
BENCHMARK(louds::build<Dataset::WIKI>);
BENCHMARK(louds::build<Dataset::ZIPFIAN>);

// Benchmark on point query
BENCHMARK(louds::point_query<Dataset::DNA>);
BENCHMARK(louds::point_query<Dataset::GEO>);
BENCHMARK(louds::point_query<Dataset::WIKI>);
BENCHMARK(louds::point_query<Dataset::ZIPFIAN>);
// // Benchmark on range query
// BENCHMARK(louds::range_query<Dataset::DNA>);
// BENCHMARK(louds::range_query<Dataset::GEO>);
// BENCHMARK(louds::range_query<Dataset::WIKI>);


BENCHMARK(louds::point_query<Dataset::DNA, Pattern::RANDOM>);
BENCHMARK(louds::point_query<Dataset::GEO, Pattern::RANDOM>);
BENCHMARK(louds::point_query<Dataset::WIKI, Pattern::RANDOM>);

BENCHMARK_MAIN();