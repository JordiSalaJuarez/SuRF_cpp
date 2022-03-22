#include <tlx/container/btree_set.hpp>
#include "../surf/surf.h"
#include "../surf_paper/include/surf.hpp"
#include <fstream>
#include <string>
#include <benchmark/benchmark.h>
#include <random>
#include <algorithm>


std::vector<std::string> get_input_data(size_t n_keys){
    std::ifstream file("data/keys_100M");
    std::string key;
    std::vector<std::string> keys;
    while(std::getline(file, key) && 0 < n_keys){
        keys.push_back(key);
        --n_keys;
    }
    // std::random_device rd;
    // std::mt19937 g(rd());
    // std::shuffle(begin(keys), end(keys), g);
    // keys.resize(n_keys);
    std::sort(begin(keys), end(keys));
    return std::move(keys);
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
    state.SetLabel(std::string("MB used ") + std::to_string(size / 1024 / 1024));
}

static void BM_ConstructionSurf(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    for (auto _ : state){ 
        auto builder = yas::LoudsBuilder::from_vector(keys);
        auto surf = yas::Surf::from_builder(builder, 1);
    }
}

static void BM_ConstructionSurfPaper(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    for (auto _ : state){ 
        auto surf = surf::SuRF(keys);
    }
    auto surf = surf::SuRF(keys);
    state.SetLabel(std::string("MB used ") + std::to_string(surf.getMemoryUsage() / 1024 / 1024));
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
    auto surf = yas::Surf::from_builder(builder, 1);
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


static void BM_PointQueryLoudsSparsePaper(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    tlx::btree_set<std::string> tree;
    surf::SuRFBuilder builder(false, 0, surf::SuffixType::kNone, 0, 0);
    builder.build(keys);
    auto louds_sparse = surf::LoudsSparse(&builder);
    for (auto _ : state){
        benchmark::DoNotOptimize(louds_sparse.lookupKey(keys[rand()%size(keys)], 0));
    }
}

static void BM_PointQueryLoudsSparse(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    auto builder = yas::LoudsBuilder::from_vector(keys);
    auto louds_sparse = yas::LoudsSparse::from_builder(builder);
    for (auto _ : state){
        benchmark::DoNotOptimize(louds_sparse.look_up(keys[rand()%size(keys)], 0));
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

// BENCHMARK(BM_ConstructionBTree)->Arg(N);
// BENCHMARK(BM_ConstructionSurf)->Arg(N);
// BENCHMARK(BM_ConstructionSurfPaper)->Arg(N);

// BENCHMARK(BM_PointQueryBTree)->Arg(N);
// BENCHMARK(BM_PointQuerySurf)->Arg(N);
// BENCHMARK(BM_PointQuerySurfPaper)->Arg(N);
// BENCHMARK(BM_PointQueryLoudsSparsePaper)->Arg(N);
// BENCHMARK(BM_PointQueryLoudsSparse)->Arg(N);
// BENCHMARK(BM_AccessBitLoudsSparsePaper)->Arg(N);
// BENCHMARK(BM_AccessBitLoudsSparse)->Arg(N);
BENCHMARK(BM_TraversalSurfPaper)
    ->Arg(1000)
    ->Arg(10000)
    ->Arg(100000)
    ->Arg(1000000);

BENCHMARK_MAIN();
