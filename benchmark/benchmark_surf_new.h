#include <algorithm>
#include <benchmark/benchmark.h>
#include <cstddef>
#include <iterator>
#include <random>
#include "utils.h"
#include <execution>
#include "../surf/surf.h"

template <class Suffix = suffix::Hash<8>>
static void BM_ConstructionSurf(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    for (auto _ : state){ 
        auto surf = yas::Surf<Suffix>(keys, 4);
    }
    auto surf = yas::Surf<Suffix>(keys, 4);
    state.counters["Size(MB)"] = surf.get_memory_usage() / 1024 / 1024 ;

}

template <class Suffix = suffix::Hash<8>>
static void BM_PointQuerySurf(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    auto surf = yas::Surf<Suffix>(keys, 4);
    for (auto _ : state){
        size_t leaf;
        auto found = surf.traverse(keys[rand()%size(keys)], leaf);
        benchmark::DoNotOptimize(found);
    }
    state.counters["Size(MB)"] = surf.get_memory_usage() / 1024 / 1024 ;
}

template<std::size_t Size>
static void BM_FPRateSurf(benchmark::State& state) {
    auto percentage = static_cast<double>(state.range(1)) / 100.0;
    auto n_train = std::round(percentage * static_cast<double>(state.range(0)));
    auto n_test = state.range(0) - n_train;
    auto [keys_train, keys_test] = get_input_data(n_train, n_test);
    auto surf = yas::Surf<suffix::Hash<Size>>(keys_train, 4);
    auto tp = 0UL;
    auto fp = 0UL;
    auto tn = 0UL;
    auto fn = 0UL;
    size_t leaf;
    for (auto _ : state){ 
        for(auto const & key : keys_train){
            if (surf.traverse(key, leaf)) { ++tp; }
            else { ++fn; }
        }
        for(auto const & key : keys_test){
            if (surf.traverse(key, leaf)) { ++fp; }
            else { ++tn; }
        }
    }
    state.counters["Size(MB)"] = surf.get_memory_usage() / 1024 / 1024 ;
    state.counters["FPR"] = static_cast<double>(fp) / static_cast<double>(tn + fp) ;
}