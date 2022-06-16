#include <benchmark/benchmark.h>
#include <art.h>
#include <cstring>
#include "utils.h"

namespace benchmark::art {
    using namespace utils;
    using namespace benchmark;

    auto size_bytes(art_node *n) -> size_t{
        if (n == nullptr) return 0;
        union {
            art_node4 *p1;
            art_node16 *p2;
            art_node48 *p3;
            art_node256 *p4;
        } p;
        std::size_t sum_bytes = 0;
        switch (n->type) {
            case NODE4:
                p.p1 = reinterpret_cast<art_node4*>(n);
                for (auto & i : p.p1->children) {
                        sum_bytes += size_bytes(i);
                }
                return sizeof(*p.p1) + sum_bytes;
            case NODE16:
                p.p2 = reinterpret_cast<art_node16*>(n);
                for (auto & i : p.p2->children) {
                        sum_bytes += size_bytes(i);
                }
                return sizeof(*p.p2) + sum_bytes;

            case NODE48:
                p.p3 = reinterpret_cast<art_node48*>(n);
                for (auto & i : p.p3->children) {
                        sum_bytes += size_bytes(i);
                }
                return sizeof(*p.p3) + sum_bytes;

            case NODE256:
                p.p4 = reinterpret_cast<art_node256*>(n);
                for (auto & i : p.p4->children) {
                        sum_bytes += size_bytes(i);
                }
                return sizeof(*p.p4) + sum_bytes;
            default:
                return sizeof(*n);
        }
    }

    auto size_bytes(art_tree *tree) -> std::size_t{
        return sizeof(*tree) + size_bytes(tree->root);
    }

    template<Dataset Kind>
    static void build(benchmark::State & state){
        auto keys = get_keys(Kind);
        for (auto _ : state){
            art_tree t;
            int res = art_tree_init(&t);
            auto line = 1;
            for (const auto& key: keys){
                void * value;
                memcpy(&value, &line, sizeof(void *));
                art_insert(&t, reinterpret_cast<const unsigned char *>(key.c_str()),key.size(), value);
                ++line;
            }
            res = art_tree_destroy(&t);
            DoNotOptimize(res);
            DoNotOptimize(t);
        }
        art_tree t;
        int res = art_tree_init(&t);
        assert(res == 0);
        auto line = 1;
        for (const auto& key: keys){
            void * value;
            memcpy(&value, &line, sizeof(void *));
            art_insert(&t, reinterpret_cast<const unsigned char *>(key.c_str()), key.size(), value);
            ++line;
        }
        Stats stats;
        stats.ds_size_bytes = size_bytes(&t);
        stats.keys_insert = keys;
        stats.keys_query = keys;
        fill_counters(state, stats);
        res = art_tree_destroy(&t);
    }

    template<Dataset Kind>
    static void point_query(benchmark::State & state){
        auto keys = get_keys(Kind);
        auto [keys_insert, keys_query] = split(keys);
        art_tree t;
        int res = art_tree_init(&t);
        assert(res == 0);
        auto line = 1;
        for (const auto& key: keys_insert){
            void * value;
            memcpy(&value, &line, sizeof(void *));
            art_insert(&t, (const unsigned char *) key.c_str(),std::size(key), value);
            ++line;
        }
        for (auto _ : state){
            for (auto const & key : keys_query){
                uintptr_t val = (uintptr_t)art_search(&t, (unsigned char*)key.c_str(), std::size(key));
                benchmark::DoNotOptimize(val);
            }
        }
        Stats stats;
        stats.ds_size_bytes = size_bytes(&t);
        stats.keys_insert = keys_insert;
        stats.keys_query = keys_query;
        fill_counters(state, stats);
        res = art_tree_destroy(&t);
        assert(res == 0);
    }

    template<Dataset Kind>
    static void range_query(benchmark::State & state){
        auto keys = get_keys(Kind);
        auto [keys_insert, keys_query] = split(keys);
        art_tree t;
        int res = art_tree_init(&t);
        assert(res == 0);
        auto line = 1;

        art_callback range_match = [](void *data, const unsigned char *key, uint32_t key_len, void *value) -> int{
            return strcmp(reinterpret_cast<char const *>(key), reinterpret_cast<char const *>(data)) >= 0;
        };

        for (const auto& key: keys_insert){
            void * value;
            memcpy(&value, &line, sizeof(void *));
            art_insert(&t, reinterpret_cast<const unsigned char *>(key.c_str()), key.size(), value);
            ++line;
        }
        string from, to;
        for (auto _ : state){
            for (auto const & key : keys_query){
                range_key(key, 1, from, to);
                // Range Query over last byte of key
                auto val = static_cast<uintptr_t>(art_iter_prefix(&t, (unsigned char*)from.c_str(), from.size(), range_match, (void *) key.c_str()));
                DoNotOptimize(val);
            }
        }
        Stats stats;
        stats.ds_size_bytes = size_bytes(&t);
        stats.keys_insert = keys_insert;
        stats.keys_query = keys_query;
        fill_counters(state, stats);
        res = art_tree_destroy(&t);
        assert(res == 0);
    }
}

namespace art = benchmark::art;
using Dataset = utils::Dataset;

BENCHMARK(art::build<Dataset::DNA>);
BENCHMARK(art::build<Dataset::WIKI>);
BENCHMARK(art::build<Dataset::GEO>);
BENCHMARK(art::build<Dataset::ZIPFIAN>);

BENCHMARK(art::point_query<Dataset::DNA>);
BENCHMARK(art::point_query<Dataset::WIKI>);
BENCHMARK(art::point_query<Dataset::GEO>);
BENCHMARK(art::point_query<Dataset::ZIPFIAN>);

BENCHMARK(art::range_query<Dataset::DNA>);
BENCHMARK(art::range_query<Dataset::WIKI>);
BENCHMARK(art::range_query<Dataset::GEO>);
BENCHMARK(art::range_query<Dataset::ZIPFIAN>);


BENCHMARK_MAIN();
