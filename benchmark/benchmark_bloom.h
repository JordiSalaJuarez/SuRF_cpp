#include "bloom.hpp"
#include <iterator>
#include <tlx/container/btree_set.hpp>
#include <benchmark/benchmark.h>
#include "utils.h"

template<std::size_t Size=10>
void BM_ConstructionBloomFilter(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    for (auto _ : state){ 
        BloomFilter filter(Size);
        string filter_data;
        filter.CreateFilter(keys, std::size(keys), &filter_data);
        benchmark::DoNotOptimize(filter);
        benchmark::DoNotOptimize(filter_data);
    }
    BloomFilter filter(Size);
    string filter_data;
    filter.CreateFilter(keys, std::size(keys), &filter_data);
    state.counters["Size(MB)"] = (std::size(filter_data) + sizeof(BloomFilter)) / 1024 / 1024 ;
}

template<std::size_t Size=10>
static void BM_PointQueryBloomFilter(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    BloomFilter filter(Size);
    string filter_data;
    filter.CreateFilter(keys, std::size(keys), &filter_data);
    for (auto _ : state){
        auto found = filter.KeyMayMatch(keys[rand()%size(keys)],filter_data);
        benchmark::DoNotOptimize(found);
    }
    state.counters["Size(MB)"] = (std::size(filter_data) + sizeof(BloomFilter)) / 1024 / 1024 ;
}

template<std::size_t Size=10>
void BM_FPRateBloomFilter(benchmark::State& state) {
    auto percentage = static_cast<double>(state.range(1)) / 100.0;
    auto n_train = std::round(percentage * static_cast<double>(state.range(0)));
    auto n_test = state.range(0) - n_train;
    auto [keys_train, keys_test] = get_input_data(n_train, n_test);
    auto tp = 0UL;
    auto fp = 0UL;
    auto tn = 0UL;
    auto fn = 0UL;
    
    BloomFilter filter(Size);
    string filter_data;
    filter.CreateFilter(keys_train, std::size(keys_train), &filter_data);
    for (auto _ : state){ 
        for(auto const & key : keys_train){
            if (filter.KeyMayMatch(key, filter_data)) { ++tp; }
            else { ++fn; }
        }
        for(auto const & key : keys_test){
            if (filter.KeyMayMatch(key, filter_data)) { ++fp; }
            else { ++tn; }
        }
    }
    state.counters["Size(MB)"] = (std::size(filter_data) + sizeof(BloomFilter)) / 1024 / 1024 ;
    state.counters["FPR"] = static_cast<double>(fp) / static_cast<double>(tn + fp) ;
}

