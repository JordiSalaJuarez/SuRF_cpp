#pragma once
#include <cassert>
#include <cstddef>
#include <gsl/pointers>
#include <numeric>
#include <string_view>
#include <type_traits>
#include <vector>
#include <bitset>
#include <algorithm>
#include <span>
#include <cstring>
#include <optional>
#include <iostream>
#include <limits>
#include <cstdint>
#include <memory>
#include <string.h>
#include "utils/bit_vector.h"
#include "input.h"
#include "builder.h"
#include "suffix.h"
#include "utils.h"
#include <gsl/gsl>

namespace yas
{
    using std::vector;

    struct Positions
    {
        vector<size_t> positions;
        size_t level;

    public:
        decltype(auto) current() const { return positions[level]; }
        decltype(auto) operator[](size_t i) const { return positions[i]; };
        decltype(auto) operator[](size_t i) { return positions[i]; };
        inline bool operator==(const Positions &other) { return current() == other.current(); }
        size_t size() const { return positions.size(); }
        Positions(size_t n_pos) : positions(n_pos, 0), level{0} {}
    };

    template <typename Suffix>
    struct LoudsDense
    {
        using Builder = LoudsBuilder;

    public:
        // struct Iter {
        //     size_t from_level;
        //     size_t to_level;
        //     size_t n_roots;
        //     size_t node;
        //     size_t pos;
        //     bool has_child;
        //     int level;
        //     std::shared_ptr<Positions> idxs;
        //     LoudsDense *louds;
        //     Iter(LoudsDense *louds, std::shared_ptr<Positions> positions, size_t node = 0, size_t from_level = 0, size_t n_roots = 0):
        //         from_level{from_level},
        //         to_level{from_level + louds->n_levels},
        //         n_roots{n_roots},
        //         node{node},
        //         pos{0},
        //         has_child{false},
        //         level{(int) from_level},
        //         idxs{positions},
        //         louds{louds}{}
        //     Iter& begin(){
        //         auto pos_node_first = [&](auto node, auto level) {
        //             if (level == from_level)
        //                 return 256 * node;
        //             else
        //                 return 256 * (node + n_roots);
        //         };
        //         for(level = from_level; level < to_level; ++level){
        //             auto begin_ = pos_node_first(node, level);
        //             auto end_ = begin_ + 256;
        //             for (pos = begin_; pos != end_; ++pos){
        //                 if (louds->labels[pos/256][pos&255]){
        //                     (*idxs)[level] = pos;
        //                     if (louds->has_child[pos/256][pos&255]){
        //                         node = louds->rank_c(pos); // get child node
        //                         break;
        //                     } else {
        //                         idxs->level = level;
        //                         return *this;
        //                     }
        //                 }
        //             }
        //         }
        //         has_child = louds->has_child[pos/256][pos&255];
        //         level = has_child? level-1:level;
        //         idxs->level = level;
        //         return *this;
        //     }
        //     Iter& end(){
        //         auto pos_node_last = [&](auto node, auto level) {
        //             if (level == from_level)
        //                 return 256 * (node + 1) - 1;
        //             else
        //                 return 256 * (node + n_roots + 1) - 1;
        //         };
        //         level = from_level;
        //         while(true){
        //             auto begin_ = pos_node_last(node, level);
        //             auto end_ = begin_ - 256;
        //             for (pos = begin_; pos != end_; --pos){
        //                 if (louds->labels[pos/256][pos&255]){
        //                     (*idxs)[level] = pos;
        //                     if (louds->has_child[pos/256][pos&255]){
        //                         ++level;
        //                         node = louds->rank_c(pos); // get child node
        //                         break;
        //                     } else {
        //                         idxs->level = level;
        //                         return *this;
        //                     }
        //                 }
        //             }
        //         }
        //         has_child = louds->has_child[pos/256][pos&255];
        //         level = has_child? level-1:level;
        //         idxs->level = level;
        //         return *this;
        //     }
        //     auto operator *(){ // only necessary for testing
        //         struct PrefixSuffix {
        //             std::string prefix;
        //             std::string suffix;
        //         };
        //         std::string prefix{};
        //         prefix.reserve(level-from_level);
        //         for (auto i = from_level; i <= level; ++i){
        //             prefix.push_back((*idxs)[i]%256);
        //         }
        //         if (prefix.back() == '\0') prefix.pop_back();
        //         string suffix{has_child? "":louds->values[louds->value(pos)]};
        //         return PrefixSuffix{prefix, suffix};
        //     }
        //     Iter& operator ++(){
        //         auto next_pos = [&](auto pos){
        //             auto begin = pos+1;
        //             auto end = ((pos >> 8) + 1) << 8; //same as: 256*(pos/256 + 1)  but faster (asm: lea)
        //             for (auto i = begin; i < end; ++i){
        //                 if (louds->labels[i/256][i&255]){
        //                     return i;
        //                 }
        //             }
        //             return end;
        //         };
        //         auto pos_node_first = [&](auto node, auto level) {
        //             auto begin = level == from_level? 256 * node: 256 * (node + n_roots);
        //             auto end = level == from_level? 256 * (node + 1): 256 * (node + n_roots + 1);
        //             for (auto i = begin; i != end; ++i){
        //                 if (louds->labels[i/256][i&255]) return i;
        //             }
        //             return end;
        //         };
        //         if ((*idxs)[level]+1 == 256*size(louds->labels)) --level; // bounds check
        //         (*idxs)[level] = next_pos((*idxs)[level]);
        //         while(from_level < level && ((*idxs)[level]&255) == 0){ // traversing upwards
        //             --level;
        //             (*idxs)[level] = next_pos((*idxs)[level]);
        //         }
        //         while(level + 1 < to_level && louds->has_child[(*idxs)[level]/256][(*idxs)[level]&255]){ // traversing downwards
        //             (*idxs)[level+1] = pos_node_first(louds->rank_c((*idxs)[level]), level + 1);
        //             ++level;
        //         }
        //         pos = (*idxs)[level];
        //         // pos is leaf
        //         has_child = louds->has_child[pos/256][pos&255];
        //         idxs->level = level;
        //         return *this;
        //     }
        //     Iter& operator --(){
        //         auto prev_pos = [&](auto pos){
        //             auto begin = pos-1;
        //             auto end = (pos >> 8) << 8; //same as: 256*(pos/256 + 1)  but faster (asm: lea)
        //             for (auto i = begin; i >= end; --i){
        //                 if (louds->labels[i/256][i&255]){
        //                     return i;
        //                 }
        //             }
        //             return end-1;
        //         };
        //         auto pos_node_last = [&](auto node, auto level) {
        //             auto begin = level == from_level? 256 * (node + 1) - 1: 256 * (node + n_roots + 1)-1;
        //             auto end = level == from_level? 256 * node - 1: 256 * (node + n_roots)-1;
        //             for (auto i = begin; i != end; --i){
        //                 if (louds->labels[i/256][i&255]) return i;
        //             }
        //             return end;
        //         };
        //         (*idxs)[level] = prev_pos((*idxs)[level]);
        //         while(from_level < level && ((*idxs)[level]&255) == 255){ // traversing upwards
        //             --level;
        //             (*idxs)[level] = prev_pos((*idxs)[level]);
        //         }
        //         while(level + 1 < to_level && louds->has_child[(*idxs)[level]/256][(*idxs)[level]&255]){ // traversing downwards
        //             (*idxs)[level+1] = pos_node_last(louds->rank_c((*idxs)[level]), level + 1);
        //             ++level;
        //         }
        //         pos = (*idxs)[level];
        //         // pos is leaf
        //         has_child = louds->has_child[pos/256][pos&255];
        //         idxs->level = level;
        //         return *this;
        //     }
        //     Iter& lb(std::string_view key){
        //         auto find = [&](auto &container, size_t begin, size_t end){
        //             auto i = begin;
        //             while(i != end && !container[i/256][i&255]) ++i;
        //             return i;
        //         };
        //         auto pos_node_first = [&](auto node, auto level) {
        //             if (level == from_level)
        //                 return 256 * node;
        //             else
        //                 return 256 * (node + n_roots);
        //         };
        //         auto pos_node_last = [&](auto node, auto level) {
        //             if (level == from_level)
        //                 return 256 * (node+1);
        //             else
        //                 return 256 * (node + n_roots + 1);
        //         };
        //         auto pos_node_ge = [&](auto node, auto level, auto value) {
        //             auto begin = pos_node_first(node, level);
        //             auto end = pos_node_last(node, level);
        //             return find(louds->labels, begin + value , end);
        //         };
        //         auto on_boundary = true;
        //         level = from_level;
        //         for(level = from_level; level < to_level; ++level){
        //             if (on_boundary){ // get closest value to boundary
        //                 pos = pos_node_ge(node, level, key[level]);
        //                 on_boundary = pos%256 == key[level] && level+1 < size(key);
        //             }else{ // get left-most value
        //                 pos = pos_node_ge(node, level, 0);
        //             }
        //             (*idxs)[level] = pos; // set cursor to last pos
        //             if (louds->has_child[pos/256][pos&255] && pos/256 == (level == from_level? node:node+n_roots)){
        //                 node = louds->rank_c(pos); // get child node
        //             } else {
        //                 idxs->level = level;
        //                 return *this;
        //             }
        //         }
        //         has_child = louds->has_child[pos/256][pos&255];
        //         level = has_child? level-1:level;
        //         idxs->level = level;
        //         return *this;
        //     }
        //     Iter& ub(std::string_view key){
        //         auto find_reverse = [&](auto &container, size_t begin, size_t end){
        //             auto i = begin;
        //             while(i != end && !container[i/256][i&255]) --i;
        //             return i;
        //         };
        //         auto pos_node_first = [&](auto node, auto level) {
        //             if (level == from_level)
        //                 return 256 * node;
        //             else
        //                 return 256 * (node + n_roots);
        //         };
        //         auto pos_node_last = [&](auto node, auto level) {
        //             if (level == from_level)
        //                 return 256 * (node+1) - 1;
        //             else
        //                 return 256 * (node + n_roots + 1) - 1;
        //         };
        //         auto pos_node_le = [&](auto node, auto level, auto key) {
        //             auto end = pos_node_first(node, level);
        //             return find_reverse(louds->labels, end+key, end-1);
        //         };
        //         auto on_boundary = true;
        //         for(level = from_level; level < to_level; ++level){
        //             if (on_boundary){ // get closest value to boundary
        //                 pos = pos_node_le(node, level, key[level]);
        //                 on_boundary = pos%256 == key[level] && level+1 < size(key);
        //             }else { // get left-most value
        //                 pos = pos_node_le(node, level, 255);
        //             }
        //             (*idxs)[level] = pos; // set cursor to last pos
        //             if (louds->has_child[pos/256][pos&255] && pos/256 == (level == from_level? node:node+n_roots)){
        //                 node = louds->rank_c(pos); // get child node
        //             } else {
        //                 break;
        //             }
        //         }
        //         has_child = louds->has_child[pos/256][pos&255];
        //         level = has_child? level-1:level;
        //         idxs->level = level;
        //         return *this;
        //     }
        //     friend bool operator==(const Iter& l, const Iter& r) {
        //         assert(l.louds == r.louds);
        //         return (*l.idxs)[l.idxs->level] == (*r.idxs)[r.idxs->level];
        //     }
        // };
        // Iter begin(){
        //     return Iter{this, std::make_shared<Positions>(this->to_level)}.begin();
        // }
        // Iter begin(std::shared_ptr<Positions> pos){
        //     return Iter{this, std::make_shared<Positions>(this->to_level)}.begin();
        // }
        // Iter end(){
        //     return Iter{this, std::make_shared<Positions>(this->to_level)}.end();
        // }
        // Iter end(std::shared_ptr<Positions> pos){
        //     return Iter{this, pos}.end();
        // }
        // Iter lb(std::string_view key){
        //     return Iter{this, std::make_shared<Positions>(this->to_level)}.lb(key);
        // }
        // Iter lb(std::string_view key, std::shared_ptr<Positions> pos){
        //     return Iter{this, pos}.lb(key);
        // }
        // Iter ub(std::string_view key){
        //     return Iter{this, std::make_shared<Positions>(this->to_level)}.ub(key);
        // }
        // Iter ub(std::string_view key, std::shared_ptr<Positions> pos){
        //     return Iter{this, pos}.ub(key);
        // }

        vector<std::bitset<256>> labels{};
        vector<std::bitset<256>> has_child{};
        vector<char const *> values;
        vector<Suffix> suffixes;
        vector<std::size_t> lut_has_child{};
        vector<std::size_t> lut_labels{};
        size_t n_levels{};
        size_t from_level{};
        size_t to_level{};
        size_t n_trailing_children{};
        size_t max_size_key{};
        size_t get_memory_usage()
        {
            return std::size(labels) * sizeof(std::bitset<256UL>) +
                   std::size(has_child) * sizeof(std::bitset<256UL>) +
                   std::size(lut_has_child) * sizeof(std::size_t) +
                   std::size(lut_labels) * sizeof(std::size_t);
        }
        static void merge_and_insert(const auto &vs, auto &r, size_t capacity)
        {
            r.reserve(capacity);
            for (const auto &v : vs)
                r.insert(std::end(r), std::begin(v), std::end(v));
        }

        LoudsDense(range_value_string auto&& input): LoudsDense(Builder(std::forward<decltype(input)>(input))){}

        LoudsDense(is_builder auto&& builder) : LoudsDense(std::forward<decltype(builder)>(builder), 0, builder.n_levels, 0){};

        LoudsDense(is_builder auto&& builder, size_t from_level, size_t to_level, size_t n_trailing_children) : 
            values(merge(slice(builder.values, from_level, to_level), builder.n_leafs)),
            suffixes(merge<Suffix>(slice(builder.suffixes, from_level, to_level), builder.n_leafs)),
            from_level(from_level),
            to_level(to_level),
            n_levels(to_level - from_level),
            n_trailing_children(n_trailing_children),
            max_size_key(builder.max_size_key)
        {

            auto nested_size = [](auto vs)
            {
                auto sum = 0;
                for (const auto &v : vs) {
                    sum += size(v);
                }
                return sum;
            };
            auto builder_louds = slice(builder.louds, from_level, to_level);
            auto builder_suffix = slice(builder.suffixes, from_level, to_level);
            auto builder_n_nodes_level = slice(builder.n_nodes_level, from_level, to_level);
            auto n_nodes = std::accumulate(std::begin(builder_n_nodes_level), std::end(builder_n_nodes_level), 0UL);
            // assert(sum_size == builder.n_nodes);
            labels.reserve(n_nodes);
            has_child.reserve(n_nodes);
            for (auto level = from_level; level < to_level; level++)
            {
                auto m = std::size(builder.louds[level]);
                for (auto i = 0; i < m; i++)
                {
                    if (builder.louds[level][i])
                    {
                        labels.emplace_back();
                        has_child.emplace_back();
                    }
                    auto c = builder.labels[level][i];
                    labels.back().set(c, true);
                    has_child.back().set(c, builder.has_child[level][i]);
                }
            }
            lut_has_child.resize(std::size(has_child));
            lut_labels.resize(std::size(labels));
            compute_rank(has_child, lut_has_child);
            compute_rank(labels, lut_labels);
        }

        // static LoudsDense from_builder(const LoudsBuilder<Input, Hash> &builder, size_t from_level = 0, size_t to_level = -1, size_t n_trailing_children=0){

        //     auto nested_size = [](auto vs){
        //         auto sum = 0;
        //         for (const auto& v: vs) sum += size(v);
        //         return sum;
        //     };

        //     if (to_level == -1) to_level = builder.n_levels;
        //     LoudsDense result{};
        //     result.from_level = from_level;
        //     result.to_level = to_level;
        //     result.n_levels = to_level - from_level;
        //     result.n_trailing_children = n_trailing_children;
        //     result.max_size_key = builder.max_size_key;
        //     auto builder_louds = std::span(std::begin(builder.louds)+from_level, std::begin(builder.louds) + to_level);
        //     auto builder_suffix = std::span(std::begin(builder.suffix)+from_level, std::begin(builder.suffix) + to_level);
        //     auto sum_size = 0;
        //     for (const auto &v: builder_louds) sum_size += v.count();
        //     auto sum_size_values = nested_size(builder_values);
        //     result.labels.reserve(sum_size);
        //     result.has_child.reserve(sum_size);
        //     auto n = builder.n_levels;
        //     for(auto level=from_level; level < to_level; level++){
        //         auto m = size(builder.louds[level]);
        //         for(auto i=0; i < m; i++){
        //             if (builder.louds[level][i]){
        //                 result.labels.emplace_back();
        //                 result.has_child.emplace_back();
        //             }
        //             auto c = builder.labels[level][i];
        //             result.labels.back().set(c, true);
        //             result.has_child.back().set(c, builder.has_child[level][i]);
        //         }
        //     }
        //     result.lut_has_child.resize(std::size(result.has_child));
        //     result.lut_labels.resize(std::size(result.labels));
        //     compute_rank(result.has_child, result.lut_has_child);
        //     compute_rank(result.labels, result.lut_labels);
        //     return result;
        // }

        static void compute_rank(auto &blocks, auto &lut)
        {
            auto sum = 0;
            auto n_blocks = std::size(blocks);
            for (auto i = 0; i < n_blocks; ++i)
            {
                lut[i] = sum;
                sum += blocks[i].count();
            }
        }

        size_t rank_c(size_t pos) __attribute__((const))
        {
            assert(pos < std::size(has_child) * 256);
            return lut_has_child[pos / 256] + (has_child[pos / 256] << ((256 - 1) - (pos & (256 - 1)))).count();
        }

        auto rank_l(size_t pos) -> size_t
        {
            assert(pos < std::size(has_child) * 256);
            return lut_labels[pos / 256] + (labels[pos / 256] << ((256 - 1) - (pos & (256 - 1)))).count();
        }

        auto select_c(size_t count) -> size_t
        {
            auto curr = 0;
            auto pos = 0;
            while (curr + has_child[pos / 256].count() < count && pos < size(has_child) * 256)
            {
                curr += has_child[pos / 256].count();
                pos += 256;
            }
            while (curr < count && pos < size(has_child) * 256)
            {
                curr += has_child[pos / 256][pos % 256];
                pos += 1;
            }
            return pos;
        }

        size_t child(size_t pos)
        {
            return 256 * rank_c(pos);
        }

        size_t parent(size_t pos)
        {
            return select_c(pos / 256);
        }

        size_t value(size_t pos)
        {
            return rank_l(pos) - rank_c(pos) - 1; // + rank_p(pos/256)
        }

        bool look_up(const std::string &word)
        {
            size_t leaf;
            if (traverse(word, leaf))
            {
                return strcmp(values[leaf], word.c_str()) == 0;
            }
            return false;
        }

        bool traverse(const std::string &word, size_t &leaf, size_t node = 0, size_t from_level = 0)
        {
            return traverse(
                word, leaf, [](auto word, auto &node, size_t &leaf, auto level) -> bool
                { return false; },
                node, from_level);
        }

        bool traverse(const std::string &word, size_t &leaf, auto traverse_next, size_t node, size_t from_level)
        {
            auto to_level = std::min(from_level + n_levels, size(word));
            auto pos = 0UL;
            for (auto level = 0; level < to_level; level++)
            {
                unsigned char token = word[level];
                pos = 256UL * node + token;
                if (labels[node][token])
                {
                    if (!has_child[node][token])
                    {
                        return suffixes[leaf = value(pos)] == Suffix{std::string_view(std::begin(word)+level+1, std::end(word))};
                    }
                }
                else
                {
                    return false;
                }
                node = rank_c(pos);
            }
            if (labels[node]['\0'])
            {
                leaf = value(node*256UL);
                return true;
            }
            else
            {
                node = ((node - size(labels)) * 256UL) / 256UL;
                return traverse_next(word, leaf, node, to_level);
            }
        }
    };

    struct State
    {
        vector<size_t> positions;
        size_t level;

        decltype(auto) operator[](auto i)
        {
            return positions[i];
        }
    };

    template <typename Suffix>
    struct LoudsSparse
    {
        using Builder = LoudsBuilder;

    public:

     /* struct Iter {
            Iter(LoudsSparse *louds, std::shared_ptr<Positions> positions, size_t node = 0):
                from_level{louds->from_level},
                to_level{louds->to_level},
                n_roots{louds->n_trailing_children},
                node{node},
                pos{0},
                level{(int) from_level},
                idxs{positions},
                louds{louds}{}

            Iter& begin(){
                auto pos_node_first = [&](auto node, auto level) {
                    if (level == from_level)
                        return louds->select_l(node + 1);
                    else
                        return louds->select_l(node + 1 + n_roots);
                };
                level = from_level;
                while(true){
                    pos = pos_node_first(node, level);
                    (*idxs)[level] = pos; // set cursor to last pos
                    if (louds->has_child[pos]){
                        ++level;
                        node = louds->rank_c(pos); // get child node
                    } else {
                        break;
                    }
                }
                idxs->level = level;
                return *this;
            }
            Iter& end(){
                auto pos_node_last = [&](auto node, auto level) {
                    if (level == from_level)
                        return louds->select_l(node + 2) - 1;
                    else
                        return louds->select_l(node + 2 + n_roots) - 1;
                };
                level = from_level;
                while(true){
                    pos = pos_node_last(node, level);
                    (*idxs)[level] = pos; // set cursor to last pos
                    if (louds->has_child[pos]){
                        ++level;
                        node = louds->rank_c(pos); // get child node
                    } else {
                        break;
                    }
                }
                idxs->level = level;
                return *this;
            }
            Iter& lb(std::string_view key){
                auto find_if = [&](auto &container, size_t begin, size_t end, auto &&pred){
                    auto i = begin;
                    while(i != end && !pred(container[i])) ++i;
                    return i;
                };
                auto pos_node_first = [&](auto node, auto level) {
                    if (level == from_level)
                        return louds->select_l(node + 1);
                    else
                        return louds->select_l(node + 1 + n_roots);
                };
                auto pos_node_last = [&](auto node, auto level) {
                    if (level == from_level)
                        return louds->select_l(node + 2) - 1;
                    else
                        return louds->select_l(node + 2 + n_roots) - 1;
                };
                auto pos_node_ge = [&](auto node, auto level, auto& key) {
                    auto begin = pos_node_first(node, level);
                    auto end = pos_node_last(node, level) + 1;
                    return find_if(louds->labels, begin, end, [&](char c){ return key[level] <= c;});
                };
                level=from_level;
                auto on_boundary = level+1 < size(key);
                for(; level < to_level; ++level){
                    if (on_boundary){ // get closest value to boundary
                        pos = pos_node_ge(node, level, key);
                        on_boundary = louds->labels[pos] == key[level] && level+1 < size(key);
                    }else { // get left-most value
                        pos = pos_node_first(node, level);
                    }
                    (*idxs)[level] = pos; // set cursor to last pos
                    if (louds->has_child[pos]){
                        node = louds->rank_c(pos); // get child node
                    } else {
                        break;
                    }
                }
                idxs->level = level;
                return *this;
            }

            Iter& ub(std::string_view& key){
                auto find_if_reverse = [&](auto &container, size_t begin, size_t end, auto &&pred){
                    auto i = begin;
                    while(i != end && !pred(container[i])) --i;
                    return i;
                };
                auto pos_node_first = [&](auto node, auto level) {
                    if (level == from_level)
                        return louds->select_l(node + 1);
                    else
                        return louds->select_l(node + 1 + n_roots);
                };
                auto pos_node_last = [&](auto node, auto level) {
                    if (level == from_level)
                        return louds->select_l(node + 2) - 1;
                    else
                        return louds->select_l(node + 2 + n_roots) - 1;
                };
                auto pos_node_le = [&](auto node, auto level, auto& key) {
                    auto begin = pos_node_last(node, level);
                    auto end = pos_node_first(node, level) - 1;
                    return find_if_reverse(louds->labels, begin, end, [&](char c){ return c <= key[level];});
                };
                level=from_level;
                auto on_boundary = level+1 < size(key);
                for(; level < to_level; ++level){
                    if (on_boundary){ // get closest value to boundary
                        pos = pos_node_le(node, level, key);
                        on_boundary = louds->labels[pos] == key[level] && level < size(key);
                    }else { // get left-most value
                        pos = pos_node_last(node, level);
                    }
                    (*idxs)[level] = pos; // set cursor to last pos
                    if (louds->has_child[pos]){
                        node = louds->rank_c(pos); // get child node
                    } else {
                        break;
                    }
                }
                idxs->level = level;
                return *this;
            }

            auto operator *(){ // only necessary for testing
                string prefix{};
                prefix.reserve(level-from_level);
                for (auto i = from_level; i <= level; ++i){
                    prefix.push_back(louds->labels[(*idxs)[i]]);
                }
                if (prefix.back() == '\0') prefix.pop_back();
                string suffix{louds->values[louds->value(pos)]};
                struct PrefixSuffix {
                    string prefix;
                    string suffix;
                };
                return PrefixSuffix{prefix, suffix};
            }

            Iter& operator ++(){
                auto pos_node_first = [&](auto node, auto level) {
                    if (level == from_level)
                        return louds->select_l(node + 1);
                    else
                        return louds->select_l(node + 1 + n_roots);
                };
                if ((*idxs)[level]+1 == size(louds->louds)) --level; // bounds check
                while(louds->louds[++(*idxs)[level]] ){ // traversing upwards
                    --level;
                }
                while(louds->has_child[(*idxs)[level]]){ // traversing downwards
                    (*idxs)[level+1] = pos_node_first(louds->rank_c((*idxs)[level]), level + 1);
                    ++level;
                }
                pos = (*idxs)[level];
                // pos is leaf
                idxs->level = level;
                return *this;
            }

            Iter& operator --(){
                auto pos_node_last = [&](auto node, auto level) {
                    if (level == from_level)
                        return louds->select_l(node + 2) - 1;
                    else
                        return louds->select_l(node + 2 + n_roots) - 1;
                };
                while(louds->louds[--(*idxs)[level]+1]){ // traversing upwards
                    --level;
                }
                while(louds->has_child[(*idxs)[level]]){ // traversing downwards
                    (*idxs)[level+1] = pos_node_last(louds->rank_c((*idxs)[level]), level + 1);
                    ++level;
                }
                pos = (*idxs)[level];
                // pos is leaf
                idxs->level = level;
                return *this;
            }
            friend bool operator==(const Iter& l, const Iter& r) {
                assert(l.louds == r.louds);
                return (*l.idxs)[l.idxs->level] == (*r.idxs)[r.idxs->level];
            }
            size_t from_level;
            size_t to_level;
            size_t n_roots;
            size_t node;
            size_t pos;
            int level;
            std::shared_ptr<Positions> idxs;
            LoudsSparse *louds;
        };

        Iter begin(size_t node=0) {
            return Iter{this, std::make_shared<Positions>(this->to_level), node}.begin();
        }
        Iter begin(std::shared_ptr<Positions> pos, size_t node=0) {
            return Iter{this, pos, node}.begin();
        }

        Iter end(size_t node=0) {
            return Iter{this, std::make_shared<Positions>(this->to_level), node}.end();
        }

        Iter end(std::shared_ptr<Positions> pos, size_t node=0) {
            return Iter{this, pos, node}.end();
        }

        Iter lb(std::string_view key, size_t node=0) {
            return Iter{this, std::make_shared<Positions>(this->to_level), node}.lb(key);
        }

        Iter lb(std::string_view key, std::shared_ptr<Positions> pos, size_t node=0) {
            return Iter{this, pos, node}.lb(key);
        }

        Iter ub(std::string_view key, std::shared_ptr<Positions> pos, size_t node=0) {
            return Iter{this, pos, node}.ub(key);
        }

        Iter ub(std::string_view key, size_t node=0) {
            return Iter{this, std::make_shared<Positions>(this->to_level), node}.ub(key);
        } */

        vector<char> labels;
        BitVector<64> has_child;
        BitVector<64> louds;
        vector<char const *> values;
        vector<Suffix> suffixes;
        size_t n_levels{};
        size_t from_level{};
        size_t to_level{};
        size_t n_trailing_children{};
        size_t max_size_key{};

        size_t get_memory_usage()
        {
            return std::size(labels) + has_child.get_memory_usage() + louds.get_memory_usage();
        }
        LoudsSparse(std::ranges::range auto&& input) : LoudsSparse(Builder(std::forward<decltype(input)>(input))){};
        LoudsSparse(is_builder auto&& builder) : LoudsSparse(std::forward<decltype(builder)>(builder), 0, builder.n_levels, 0, builder.n_nodes){};
        LoudsSparse(is_builder auto&& builder, size_t from_level, size_t to_level, size_t n_trailing_children, size_t n_nodes) : 
            labels(merge(slice(builder.labels, from_level, to_level), n_nodes)),
            has_child(merge(slice(builder.has_child, from_level, to_level), n_nodes)),
            louds(merge(slice(builder.louds, from_level, to_level), n_nodes)),
            values(merge(slice(builder.values, from_level, to_level), builder.n_leafs)),
            suffixes(merge<Suffix>(slice(builder.suffixes, from_level, to_level), builder.n_leafs)),
            n_levels(to_level - from_level),
            from_level(from_level),
            to_level(to_level),
            n_trailing_children(n_trailing_children),
            max_size_key(builder.max_size_key){};

        size_t rank_c(size_t pos) __attribute__((const))
        {
            return has_child.rank(pos);
        }
        size_t rank_l(size_t pos) __attribute__((const))
        {
            return louds.rank(pos);
        }

        size_t select_c(size_t count) __attribute__((const))
        {
            return has_child.select(count);
        }

        size_t select_l(size_t count) __attribute__((const))
        {
            return louds.select(count);
        }

        size_t find(size_t node, char c)
        {
            for (auto i = select_l(node + 1); i < select_l(node + 2); i++)
                if (labels[i] == c)
                    return i;
            return -1;
        }
        size_t find(size_t begin, size_t end, char c)
        {
            for (auto i = begin; i < end; ++i)
            {
                if (labels[i] == c)
                    return i;
            }
            return end;
        }

        // Std::Size_t find(std::size_t begin, std::size_t end, char c){
        //     std::size_t ans = 0UL;
        //     for (char *i = &labels[begin]; i < &labels[end]; ++i){
        //         ans += (std::size_t)((*i == c)? i: 0UL);
        //     }
        //     return ans? (ans-(std::size_t)labels.data()) : end;
        // }
        // size_t find_ptr(char* begin, char* end, char c){
        //     auto ans = 0;
        //     for (auto i = begin; i < end; ++i){
        //         ans += (*i == c)? i: 0;
        //     }
        //     return ans;
        // }
        // size_t find(char* begin, char* end, char c){
        //     for (char* i = begin; i < end; ++i)
        //         if (*i == c) return (size_t)(i - begin);
        //     return (size_t)(end-begin);
        // }

        size_t child_begin(size_t pos)
        {
            return select_l(rank_c(pos) + 1 + n_trailing_children);
        }

        size_t child(size_t pos)
        {
            return select_l(rank_c(pos) + 1 + n_trailing_children);
        }

        size_t child_end(size_t pos)
        {
            return select_l(rank_c(pos) + 2 + n_trailing_children);
        }

        size_t parent(size_t pos)
        {
            return select_c(rank_l(pos) - 1 + n_trailing_children);
        }

        size_t value(size_t pos)
        {
            return pos - rank_c(pos);
        }

        bool traverse(const std::string &word, size_t &leaf, size_t node = 0, size_t from_level = 0)
        {
            return traverse(
                word, leaf, [](const std::string &word, size_t &leaf, size_t level) -> bool
                { return false; },
                node, from_level);
        }

        bool traverse(const std::string &word, size_t &leaf, auto traverse_next, size_t node, size_t from_level)
        {
            auto to_level = std::min(from_level + n_levels, size(word));
            auto pos_begin = louds.select(node + 1);
            auto pos_end = louds.next(pos_begin);
            for (auto level = from_level; level < to_level; level++)
            {
                auto pos = find(pos_begin, pos_end, word[level]);
                if (pos == pos_end) {
                    return false; // failed to find char
                } else if (!has_child[pos]) {
                    return suffixes[leaf = value(pos)] == Suffix{std::string_view(std::begin(word)+level+1, std::end(word))}; // chech if the suffix is the same
                } else {
                    auto node = has_child.rank(pos);                              // get child node
                    pos_begin = louds.select(node + 1 + n_trailing_children); // get start pos node
                    pos_end = louds.next(pos_begin);                      // get end pos node
                }
            }
            auto pos = find(pos_begin, pos_end, '\0');
            if (pos == pos_end) {
                return false;
            } else {
                leaf = value(pos);
                return true;
            }
        }

        bool look_up(const std::string &word)
        {
            size_t leaf;
            if (traverse(word, leaf))
            {
                return strcmp(values[leaf], word.c_str()) == 0;
            }
            return false;
        }
    };
    template <typename Suffix>
    struct Surf
    {
    public:
        using Builder = LoudsBuilder;
        size_t n_dense_levels;
        size_t n_sparse_levels;
        size_t n_nodes_dense_levels;
        size_t n_nodes_sparse_levels;
        LoudsDense<Suffix> ld;
        LoudsSparse<Suffix> ls;

        size_t get_memory_usage()
        {
            return ld.get_memory_usage() + ls.get_memory_usage();
        }

        auto n_nodes_level(auto builder, auto level) -> std::size_t
        {
            return std::count(std::begin(builder.louds[level]), std::end(builder.louds[level]), true);
        };

        Surf(range_value_string auto&& input, size_t n_dense_levels): Surf(Builder(std::forward<decltype(input)>(input)), n_dense_levels){}

        Surf(is_builder auto&& builder, size_t n_dense_levels):
            n_dense_levels(n_dense_levels),
            n_sparse_levels(builder.n_levels - n_dense_levels),
            n_nodes_dense_levels(n_nodes_level(builder,n_dense_levels)),
            n_nodes_sparse_levels(builder.n_nodes - n_nodes_dense_levels),
            ld(builder, 0, n_dense_levels, 0),
            ls(builder, n_dense_levels, builder.n_levels, n_nodes_dense_levels - 1, n_nodes_sparse_levels){}

        auto look_up(const std::string &word) -> bool
        {
            size_t leaf;
            // Callback in case louds dense matched the word but didn't reach the leaf
            auto in_sparse = false;
            auto next_look_up =
                [&](const std::string &word, size_t &leaf, size_t node, size_t level) -> bool
            { 
                    in_sparse = true;
                    return ls.traverse(word,leaf, node, level); };
            if (ld.traverse(word, leaf, next_look_up, 0, 0))
            {
                return strcmp(in_sparse? ls.values[leaf] : ld.values[leaf], word.c_str()) == 0;
            }
            return false;
        }

        auto traverse(const std::string &word, std::size_t& leaf) -> bool
        {
            // Callback in case louds dense matched the word but didn't reach the leaf
            auto in_sparse = false;
            auto next_look_up =
                [&](const std::string &word, size_t &leaf, size_t node, size_t level) -> bool
                    { return ls.traverse(word,leaf, node, level); };
            return ld.traverse(word, leaf, next_look_up, 0, 0);
        }

        // bool range_query(std::string_view lb, std::string_view ub){
        //     auto pos_begin = std::make_shared<Positions>(ld->n_levels + ls->n_levels);
        //     auto pos_end = std::make_shared<Positions>(ld->n_levels + ls->n_levels);
        //     auto it_ld = ld->lb(lb, pos_begin);
        //     auto end_ld = ld->ub(ub, pos_end);
        //     auto node_end_ls = ld->rank_c(end_ld.pos) - size(ld->labels);
        //     auto end_ls = ls->ub(ub, pos_end, node_end_ls);
        //     while(true){
        //         auto [prefix_ld, suffix_ld] = *it_ld;
        //         if (it_ld.has_child){
        //             auto node_ls = ld->rank_c(it_ld.pos) - size(ld->labels);
        //             auto it_ls = ls->lb(lb,pos_begin,node_ls);
        //             auto end_ls = ls->ub(ub,node_ls);
        //             for (;it_ls != end_ls; ++it_ls){
        //                 auto [prefix_ls, suffix_ls] = *it_ls;
        //                 std::cout << prefix_ld << prefix_ls << suffix_ls << endl;
        //             }
        //         }else{
        //             std::cout << prefix_ld << suffix_ld << endl;
        //         }
        //         if (it_ld != end_ld){
        //             ++it_ld;
        //         }
        //         else break;
        //     }
        //     return true;
        // }
    };

};
