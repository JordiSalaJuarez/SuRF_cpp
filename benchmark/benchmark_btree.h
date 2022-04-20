#include <benchmark/benchmark.h>
#include <tlx/container/btree_set.hpp>
#include "utils.h"

// template <class T>
// std::size_t n_bytes(btree_set<T> &tree){
//     tree.tree_

// }
// template <typename Key, typename Value,
//           typename KeyOfValue,
//           typename Compare,
//           typename Traits,
//           bool Duplicates,
//           typename Allocator >
// std::size_t n_bytes(BTree<Key, Value, KeyOfValue, Compare, Traits, Duplicates, Allocator> tree){
//     return sizeof(tree) + n_bytes(*tree.curr_leaf);
// }

// std::size_t n_bytes(BTree<Key, Value, KeyOfValue, Compare, Traits, Duplicates, Allocator>::LeafNode &node){
//     return sizeof(node) + (n_bytes.)
// }


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

static void BM_PointQueryBTree(benchmark::State& state) {
    auto keys = get_input_data(state.range());
    tlx::btree_set<std::string> tree;
    tree.bulk_load(begin(keys), end(keys));
    for (auto _ : state){
        benchmark::DoNotOptimize(tree.find(keys[rand()%size(keys)]));
    }
    auto stats = tree.get_stats();
    auto size = stats.inner_nodes * sizeof(decltype(tree)::btree_impl::InnerNode) + stats.leaves * sizeof(decltype(tree)::btree_impl::LeafNode);
    state.counters["Size(MB)"] = size / 1024 / 1024 ;
}