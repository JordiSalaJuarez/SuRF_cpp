#include <benchmark/benchmark.h>
#include "../surf_paper/include/surf.hpp"
#include "utils.h"


template <surf::SuffixType Suffix = surf::SuffixType::kNone, size_t Size = 0UL>
static void BM_ConstructionSurfPaper(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    for (auto _ : state){ 
        auto surf = surf::SuRF(keys, Suffix, Size, 0);
    }
    auto surf = surf::SuRF(keys, Suffix, Size, 0);
    state.counters["Size(MB)"] = surf.getMemoryUsage() / 1024 / 1024 ;
}

template <surf::SuffixType Suffix = surf::SuffixType::kNone, size_t Size = 0UL>
static void BM_PointQuerySurfPaper(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    auto surf = surf::SuRF(keys, Suffix, Size, 0);
    for (auto _ : state){
        benchmark::DoNotOptimize(surf.lookupKey(keys[rand()%size(keys)]));
    }
    state.counters["Size(MB)"] = surf.getMemoryUsage() / 1024 / 1024 ;
}

template <surf::SuffixType Suffix = surf::SuffixType::kNone, size_t Size = 0UL>
static void BM_TraversalSurfPaper(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    auto surf = surf::SuRF(keys, Suffix, Size, 0);
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
    state.counters["Size(MB)"] = surf.getMemoryUsage() / 1024 / 1024 ;
}


template <size_t Size = 0UL>
void BM_FPRateSurfPaper(benchmark::State& state) {
    auto percentage = static_cast<double>(state.range(1)) / 100.0;
    auto n_train = std::round(percentage * static_cast<double>(state.range(0)));
    auto n_test = state.range(0) - n_train;
    auto [keys_train, keys_test] = get_input_data(n_train, n_test);
    auto surf = surf::SuRF(keys_train, surf::SuffixType::kHash, Size, 0);
    auto tp = 0UL;
    auto fp = 0UL;
    auto tn = 0UL;
    auto fn = 0UL;
    for (auto _ : state){ 
        for(auto const & key : keys_train){
            if (surf.lookupKey(key)) { ++tp; }
            else { ++fn; }
        }
        for(auto const & key : keys_test){
            if (surf.lookupKey(key)) { ++fp; }
            else { ++tn; }
        }
    }
    state.counters["Size(MB)"] = surf.getMemoryUsage() / 1024 / 1024 ;
    state.counters["FPR"] = static_cast<double>(fp) / static_cast<double>(tn + fp) ;
}