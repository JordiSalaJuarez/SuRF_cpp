#pragma once
#include <cassert>
#include <cstddef>
#include <iterator>
#include <limits>
#include <numeric>
#include <string_view>
#include <vector>
#include <string>
#include <ranges>
#include "utils.h"
#include <deque>

using std::vector;

struct LoudsBuilder {
    public:

    vector<vector<char>> labels{};
    vector<vector<bool>> has_child{};
    vector<vector<bool>> louds{};
    vector<vector<bool>> is_prefix{};
    vector<vector<char const *>> values{};
    vector<std::size_t> n_nodes_level{};
    vector<vector<std::string>> suffixes{};
    std::size_t n_levels{};
    std::size_t max_size_key{};
    std::size_t n_leafs{};
    std::size_t n_nodes{};
    std::size_t cutoff_level{};


    void insert_key(const std::string& key, std::size_t start, std::size_t end){
        max_size_key = std::max(max_size_key, std::size(key));
        if (end+1 > n_levels){
            n_levels = end+1;
            labels.resize(n_levels);
            has_child.resize(n_levels);
            louds.resize(n_levels);
            is_prefix.resize(n_levels);
            suffixes.resize(n_levels);
            values.resize(n_levels);
        }
        for (auto level = start; level < end; level++) { // insert new non leaf and nodes 
            labels[level].emplace_back(key[level]);
            has_child[level].push_back(true);
            is_prefix[level].push_back(false);
            louds[level].push_back(start != level);
        }
        if(end < std::size(key)){
            // insert new leaf node
            labels[end].emplace_back(key[end]);
            has_child[end].push_back(false);
            is_prefix[end].push_back(false);
            louds[end].push_back(start != end);
            // store suffix
            auto key_suffix = key.substr(end+1);
            suffixes[end].push_back(key_suffix);
            values[end].push_back(key.data());
        } else {
            labels[end].emplace_back('\0');
            has_child[end].push_back(false);
            is_prefix[end].push_back(true);
            louds[end].push_back(start != end);
            suffixes[end].emplace_back("");
            values[end].push_back(key.data());
        }
    }

    void add_dollar(const auto& i){
        if (i+2 > n_levels){
            n_levels = i+2;
            labels.resize(n_levels);
            has_child.resize(n_levels);
            louds.resize(n_levels);
        }
        has_child[i][std::size(has_child[i])] = true;
        labels[i+1].emplace_back('\0');
        louds[i+1].push_back(true);
        has_child[i+1].push_back(false);
    }

    auto lowest_common_ancestor_prev(const auto& x){
        auto i = 0;
        while(i < size(x) && i < n_levels && x[i] == labels[i].back() && has_child[i].back()) i++; // treat antecesors
        if(i < size(x) && i < n_levels && x[i] == labels[i].back()){ // treat leaf
            add_dollar(i);
            i++; 
        }
        return i;
    }

    auto lowest_common_ancestor_next(const auto& x, const auto& y){
        auto i = 0;
        while(i < size(x) && i < size(y) && x[i] == y[i]) i++;
        return i;
    }

    template<range_value_string Container>
    LoudsBuilder(Container && input){
        std::size_t n = 0;
        auto first = std::cbegin(input);
        auto second = first + 1;
        auto end = std::cend(input);
        for(;second != end; ++first, ++second){
            std::string const& key= *first;
            std::string const& next_key = *second;
            if (first == second) continue;
            assert(key < next_key);
            auto lca_prev = lowest_common_ancestor_prev(key);
            auto lca_next = lowest_common_ancestor_next(key, next_key);
            insert_key(key, lca_prev, std::max(lca_prev, lca_next));    
            n++;
        }
        if(n > 0){
            std::string const& key = *first;
            auto lca_prev = lowest_common_ancestor_prev(key);
            insert_key(key, lca_prev, lca_prev);
            louds[0][0] = true;
        }
        n_leafs = n;
        n_nodes_level.reserve(n_levels);
        for(auto const& louds_level: louds){
            n_nodes_level.push_back(count(louds_level, true));
        }
        n_nodes = std::accumulate(std::begin(n_nodes_level), std::end(n_nodes_level), 0UL);
        cutoff_level = [&](){
            size_t cutoff_level = 0;
            size_t dense_mem = 0;
            size_t sparse_mem = [&](){
                auto bits = 0UL;
                for(auto const & row : labels){
                    bits += 10*std::size(row); 
                }
                return bits;
            }();
            while ((cutoff_level < n_levels) && (dense_mem * 16 < sparse_mem)) {
                dense_mem += 2 * n_nodes_level[cutoff_level] * 256;
                sparse_mem -= (10*std::size(labels[cutoff_level]));
                cutoff_level++;
            }
            return cutoff_level--;
        }();
    }
};


auto lca(const std::string& x, const std::string& y) -> size_t{
    size_t i = 0;
    while(i < size(x) && i < size(y) && x[i] == y[i]) i++;
    return i;
}

auto prune(std::span<std::string> words, size_t prev_lca = 0) -> size_t{
    if (words.size() < 2) return 0;
    auto curr_lca = lca(words[0], words[1]);
    if (curr_lca <= prev_lca) return 0;
    size_t safe_slice = 0;
    for (size_t i = 2 ; i < words.size(); ++i){
        auto next_lca = lca(words[0], words[i]);
        if (curr_lca > next_lca){
            safe_slice = i;
        }
        curr_lca = next_lca;
        if (curr_lca <= prev_lca) return safe_slice;
    }
    return safe_slice;
}

struct LoudsBuilderPrunning : LoudsBuilder{
    size_t bucket_size;
    std::vector<std::pair<std::span<std::string>, size_t>> suffix_sets{};
    

    auto lowest_common_ancestor_prev(const std::string& x)-> size_t{
        size_t i = 0;
        while(i < size(x) && i < n_levels && x[i] == labels[i].back() && has_child[i].back()) i++; // treat antecesors
        if(i < size(x) && i < n_levels && x[i] == labels[i].back()){ // treat leaf
            add_dollar(i);
            i++; 
        }
        return i;
    }

    static auto lowest_common_ancestor_next(const std::string& x, const std::string& y) -> size_t{
        size_t i = 0;
        while(i < size(x) && i < size(y) && x[i] == y[i]) i++;
        return i;
    }
    void add_dollar(const auto& i){
        if (i+2 > n_levels){
            n_levels = i+2;
            labels.resize(n_levels);
            has_child.resize(n_levels);
            louds.resize(n_levels);
        }
        has_child[i][std::size(has_child[i])] = true;
        labels[i+1].emplace_back('\0');
        louds[i+1].push_back(true);
        has_child[i+1].push_back(false);
    }

    void insert_key(const std::string& key, std::size_t start, std::size_t end){
        max_size_key = std::max(max_size_key, std::size(key));
        if (end+1 > n_levels){
            n_levels = end+1;
            labels.resize(n_levels);
            has_child.resize(n_levels);
            louds.resize(n_levels);
            is_prefix.resize(n_levels);
            suffixes.resize(n_levels);
            values.resize(n_levels);
        }
        for (auto level = start; level < end; level++) { // insert new non leaf and nodes 
            labels[level].emplace_back(key[level]);
            has_child[level].push_back(true);
            is_prefix[level].push_back(false);
            louds[level].push_back(start != level);
        }
        if(end < std::size(key)){
            // insert new leaf node
            labels[end].emplace_back(key[end]);
            has_child[end].push_back(false);
            is_prefix[end].push_back(false);
            louds[end].push_back(start != end);
            // store suffix
            std::string key_suffix = key.substr(end+1);
            suffixes[end].push_back(key_suffix);
            values[end].push_back(key.data());
        } else {
            labels[end].emplace_back('\0');
            has_child[end].push_back(false);
            is_prefix[end].push_back(true);
            louds[end].push_back(start != end);
            suffixes[end].emplace_back("");
            values[end].push_back(key.data());
        }
    }
    template<range_value_string Container>
    LoudsBuilderPrunning(Container && input, size_t _bucket_size): bucket_size{_bucket_size} {
        std::size_t n = 0;
        std::deque<std::string> bucket{};
        std::deque<std::size_t> lca{};
        size_t prev_lca = 0;
        size_t i = 0;
        for (size_t i = 0; i < input.size();){
            auto slice = i + bucket_size <= input.size() ? 
                std::span{input.data() + i, bucket_size} : 
                std::span{input.data() + i, input.data() + input.size()};
            auto offset = prune(slice, prev_lca);
            if (offset == 0){
                suffix_sets.push_back({
                    std::span{slice.data(), 1},
                    std::max(prev_lca, lca(slice[0], slice[1]))
                    });
                auto next_lca = lca(slice[0], slice[1]);
                insert_key(slice[0], prev_lca, max(prev_lca, next_lca));
                prev_lca = next_lca;
                ++i;
            } else {
                suffix_sets.push_back({
                    std::span{slice.data(), offset},
                    std::max(prev_lca, lca(slice[0], slice[offset-1]))
                });
                auto next_lca = lca(slice[offset-1], slice[offset]);
                insert_key(slice[0], prev_lca, max(prev_lca, next_lca));
                prev_lca = next_lca;
                i += offset;
            }
        }
        n_leafs = n;
        n_nodes_level.reserve(n_levels);
        for(auto const& louds_level: louds){
            n_nodes_level.push_back(count(louds_level, true));
        }
        n_nodes = std::accumulate(std::begin(n_nodes_level), std::end(n_nodes_level), 0UL);
        cutoff_level = [&](){
            size_t cutoff_level = 0;
            size_t dense_mem = 0;
            size_t sparse_mem = [&](){
                auto bits = 0UL;
                for(auto const & row : labels){
                    bits += 10*std::size(row); 
                }
                return bits;
            }();
            while ((cutoff_level < n_levels) && (dense_mem * 16 < sparse_mem)) {
                dense_mem += 2 * n_nodes_level[cutoff_level] * 256;
                sparse_mem -= (10*std::size(labels[cutoff_level]));
                cutoff_level++;
            }
            return cutoff_level--;
        }();
    }
};
template<typename T>
    concept is_builder = std::is_same_v<std::remove_cvref_t<T>, LoudsBuilder>;
