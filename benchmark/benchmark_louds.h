#include <cstddef>
#include "utils.h"
#include <benchmark/benchmark.h>
// #include <gperftools/malloc_extension.h>
#define private public 
#include <tx/tx.hpp>
#undef private


static void BM_ConstructionLouds(benchmark::State& state) {
    auto keys = get_input_data(state.range()); 
    std::string fileName = std::tmpnam(nullptr);   
    for (auto _ : state){ 
        tx_tool::tx trie{};
        trie.build(keys, fileName.c_str());
        trie.read(fileName.c_str());
        benchmark::DoNotOptimize(trie);
    }
    tx_tool::tx trie{};
    trie.build(keys, fileName.c_str());
    trie.read(fileName.c_str());
    auto n_bytes = (trie.loud.getSize() / 8) + (trie.terminal.getSize() / 8) + trie.loud.getSize(); 
    state.counters["Size(MB)"] = static_cast<double>(n_bytes / 1024 / 1024);
}

static void BM_PointQueryLouds(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    std::string fileName = std::tmpnam(nullptr);   
    tx_tool::tx trie{};
    trie.build(keys, fileName.c_str());
    trie.read(fileName.c_str());
    for (auto _ : state){
        size_t retLen;
        auto key = keys[rand()%size(keys)];
        auto ret = trie.prefixSearch(key.data(),std::size(key), retLen);
        benchmark::DoNotOptimize(ret);
    }
    auto n_bytes = (trie.loud.getSize() / 8) + (trie.terminal.getSize() / 8) + trie.loud.getSize(); 
    state.counters["Size(MB)"] = static_cast<double>(n_bytes / 1024 / 1024);
}