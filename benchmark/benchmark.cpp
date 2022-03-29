#include <tlx/container/btree_set.hpp>
#include "../surf/surf.h"
#include "../surf_paper/include/surf.hpp"
#include <fstream>
#include <string>
#include <benchmark/benchmark.h>
#include <random>
#include <algorithm>
#include <gperftools/profiler.h>
#include <art.h>

std::vector<std::string> get_input_data(auto n_keys){
    static auto keys = [] (size_t n_keys) {
        std::ifstream file("data/keys_100M");
        std::string key;
        std::vector<std::string> keys;
        while(std::getline(file, key) && 0 < n_keys){
            keys.push_back(key);
            --n_keys;
        }
        file.close();
        return keys;
    }(n_keys);
    return keys;
}

static void BM_ConstructionART(benchmark::State & state){
    auto keys = get_input_data(state.range());
    for (auto _ : state){
        art_tree t;
        int res = art_tree_init(&t);
        assert(res == 0);
        auto line = 1;
        for (const auto& key: keys){
            art_insert(&t, (const unsigned char *) key.c_str(),std::size(key), (void *) line);
            ++line;
        }
        res = art_tree_destroy(&t);
        assert(res == 0);
    }

    // state.counters["Size(MB)"] = size / 1024 / 1024 / state.iterations();
}

static void BM_ConstructionBTree(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    for (auto _ : state){ 
        tlx::btree_set<std::string> tree;
        tree.bulk_load(begin(keys), end(keys));
    }
    tlx::btree_set<std::string> tree;
    tree.bulk_load<decltype(begin(keys))>(begin(keys), end(keys));
    auto stats = tree.get_stats();
    auto size = stats.inner_nodes * sizeof(decltype(tree)::btree_impl::InnerNode) + stats.leaves * sizeof(decltype(tree)::btree_impl::LeafNode);
    state.counters["Size(MB)"] = size / 1024 / 1024 ;
}

static void BM_ConstructionSurf(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    for (auto _ : state){ 
        auto builder = yas::LoudsBuilder::from_vector(keys);
        auto surf = yas::Surf::from_builder(builder, 1);
    }
    auto builder = yas::LoudsBuilder::from_vector(keys);
    auto surf = yas::Surf::from_builder(builder, 1);
    state.counters["Size(MB)"] = surf.get_memory_usage() / 1024 / 1024 ;

}

static void BM_ConstructionSurfPaper(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    for (auto _ : state){ 
        auto surf = surf::SuRF(keys);
    }
    auto surf = surf::SuRF(keys);
    state.counters["Size(MB)"] = surf.getMemoryUsage() / 1024 / 1024 ;
}


static void BM_PointQueryBTree(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    tlx::btree_set<std::string> tree;
    tree.bulk_load(begin(keys), end(keys));
    for (auto _ : state){
        benchmark::DoNotOptimize(tree.find(keys[rand()%size(keys)]));
    }
}

static void BM_PointQuerySurf(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    tlx::btree_set<std::string> tree;
    auto builder = yas::LoudsBuilder::from_vector(keys);
    auto surf = yas::Surf::from_builder(builder, 4);
    for (auto _ : state){
        benchmark::DoNotOptimize(surf.look_up(keys[rand()%size(keys)]));
    }
}

static void BM_PointQuerySurfPaper(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    tlx::btree_set<std::string> tree;
    auto surf = surf::SuRF(keys);
    for (auto _ : state){
        benchmark::DoNotOptimize(surf.lookupKey(keys[rand()%size(keys)]));
    }
}

static void BM_PointQueryART(benchmark::State & state){
    auto keys = get_input_data(state.range());
    art_tree t;
    int res = art_tree_init(&t);
    assert(res == 0);
    auto line = 1;
    for (const auto& key: keys){
        art_insert(&t, (const unsigned char *) key.c_str(),std::size(key), (void *) line);
        ++line;
    }
    for (auto _ : state){
        auto key = keys[rand()%size(keys)];
        uintptr_t val = (uintptr_t)art_search(&t, (unsigned char*)key.c_str(), std::size(key));
        benchmark::DoNotOptimize(val);
    }
    res = art_tree_destroy(&t);
    assert(res == 0);
}


static void BM_PointQueryLoudsSparsePaper(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    tlx::btree_set<std::string> tree;
    surf::SuRFBuilder builder(false, 0, surf::SuffixType::kNone, 0, 0);
    builder.build(keys);
    auto louds_sparse = surf::LoudsSparse(&builder);
    for (auto _ : state){
        benchmark::DoNotOptimize(louds_sparse.lookupKey(keys[rand()%size(keys)], 0));
    }
    state.counters["Size(MB)"] = louds_sparse.getMemoryUsage() / 1024 / 1024 ;
}

static void BM_PointQueryLoudsSparse(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    auto builder = yas::LoudsBuilder::from_vector(keys);
    auto louds_sparse = yas::LoudsSparse::from_builder(builder);
    for (auto _ : state){
        benchmark::DoNotOptimize(louds_sparse.look_up(keys[rand()%size(keys)], 0));
    }
    state.counters["Size(MB)"] = louds_sparse.get_memory_usage() / 1024 / 1024 ;
}


static void BM_PointQueryLoudsDensePaper(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    surf::SuRFBuilder builder(true, 0, surf::SuffixType::kNone, 0, 0);
    builder.build(keys);
    auto louds_dense = surf::LoudsDense(&builder);
    ProfilerStart("BM_PointQueryLoudsDensePaper.prof");
    for (auto _ : state){
        surf::position_t pos = 0;
        auto key = keys[rand()%size(keys)];
        auto found = louds_dense.lookupKey(key, pos);
        benchmark::DoNotOptimize(found);
        benchmark::DoNotOptimize(louds_dense);
    }
    ProfilerStop();
}

static void BM_PointQueryLoudsDense(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    auto builder = yas::LoudsBuilder::from_vector(keys);
    auto louds_dense = yas::LoudsDense::from_builder(builder);
    ProfilerStart("BM_PointQueryLoudsDense.prof");
    for (auto _ : state){
        auto key = keys[rand()%size(keys)];
        auto found = louds_dense.look_up(key, 0);
        benchmark::DoNotOptimize(found);
        benchmark::DoNotOptimize(louds_dense);
    }
    ProfilerStop();
}

static void BM_AccessBitLoudsDensePaper(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    surf::SuRFBuilder builder(true, 0, surf::SuffixType::kNone, 0, 0);
    builder.build(keys);
    auto louds_dense = surf::LoudsDense(&builder);
    for (auto _ : state){
        surf::position_t pos = rand()%100000;
        auto found = louds_dense.child_indicator_bitmaps_->readBit(pos);
        benchmark::DoNotOptimize(found);
        benchmark::DoNotOptimize(louds_dense);
    }
}

static void BM_AccessBitLoudsDense(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    auto builder = yas::LoudsBuilder::from_vector(keys);
    auto louds_dense = yas::LoudsDense::from_builder(builder);
    for (auto _ : state){
        std::size_t pos = rand()%100000;
        auto found = louds_dense.has_child[pos / 255][pos & (255-1)];
        benchmark::DoNotOptimize(found);
        benchmark::DoNotOptimize(louds_dense);
    }
}


static void BM_RankLoudsDensePaper(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    surf::SuRFBuilder builder(true, 0, surf::SuffixType::kNone, 0, 0);
    builder.build(keys);
    auto louds_dense = surf::LoudsDense(&builder);
    for (auto _ : state){
        surf::position_t pos = rand()%10000000;
        auto found = louds_dense.child_indicator_bitmaps_->rank(pos);
        benchmark::DoNotOptimize(found);
        benchmark::DoNotOptimize(louds_dense);
    }
}

static void BM_RankLoudsDense(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    auto builder = yas::LoudsBuilder::from_vector(keys);
    auto louds_dense = yas::LoudsDense::from_builder(builder);
    for (auto _ : state){
        std::size_t pos = rand()%10000000;
        auto found = louds_dense.rank_c(pos);
        benchmark::DoNotOptimize(found);
        benchmark::DoNotOptimize(louds_dense);
    }
}


static void BM_AccessBitLoudsSparsePaper(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    tlx::btree_set<std::string> tree;
    surf::SuRFBuilder builder(false, 0, surf::SuffixType::kNone, 0, 0);
    builder.build(keys);
    auto louds_sparse = surf::LoudsSparse(&builder);
    for (auto _ : state){
        benchmark::DoNotOptimize(louds_sparse.child_indicator_bits_->readBit(rand()%100000));
    }
}

static void BM_AccessBitLoudsSparse(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    auto builder = yas::LoudsBuilder::from_vector(keys);
    auto louds_sparse = yas::LoudsSparse::from_builder(builder);
    for (auto _ : state){
        benchmark::DoNotOptimize(louds_sparse.has_child[rand()%100000]);
    }
}

static void BM_TraversalSurfPaper(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    surf::SuRF surf(keys);
    for (auto _ : state){
        std::vector<std::string> words;
        auto it = surf.moveToFirst();
        auto end = surf.moveToLast();
        while(it++){
            words.push_back(it.getKey());
        }
        benchmark::DoNotOptimize(words);
    }
    state.SetBytesProcessed(state.range() * size(keys[0]) * state.iterations());
}

const static auto MAX_N = 100000000;
const static auto N = 1000000;


// BENCHMARK(BM_ConstructionBTree)->Arg(N)->Unit(benchmark::kMillisecond);;
// BENCHMARK(BM_ConstructionSurf)->Arg(N)->Unit(benchmark::kMillisecond);;
// BENCHMARK(BM_ConstructionSurfPaper)->Arg(N)->Unit(benchmark::kMillisecond);;
// BENCHMARK(BM_ConstructionART)->Arg(N)->Unit(benchmark::kMillisecond);;

// BENCHMARK(BM_PointQueryBTree)->Arg(N);
// BENCHMARK(BM_PointQuerySurf)->Arg(N);
// BENCHMARK(BM_PointQuerySurfPaper)->Arg(N);
BENCHMARK(BM_PointQueryLoudsSparsePaper)->Arg(N);
BENCHMARK(BM_PointQueryLoudsSparse)->Arg(N);

// BENCHMARK(BM_PointQueryLoudsDensePaper)->Arg(N);
// BENCHMARK(BM_PointQueryLoudsDense)->Arg(N);

// BENCHMARK(BM_AccessBitLoudsDensePaper)->Arg(N);
// BENCHMARK(BM_AccessBitLoudsDense)->Arg(N);

// BENCHMARK(BM_RankLoudsDensePaper)->Arg(N);
// BENCHMARK(BM_RankLoudsDense)->Arg(N);

// BENCHMARK(BM_AccessBitLoudsSparsePaper)->Arg(N);
// BENCHMARK(BM_AccessBitLoudsSparse)->Arg(N);
// BENCHMARK(BM_TraversalSurfPaper)
//     ->Arg(1000)
//     ->Arg(10000)
//     ->Arg(100000)
//     ->Arg(1000000);

BENCHMARK_MAIN();
