#pragma once
#include <cassert>
#include <compare>
#include <cstddef>
#include <gsl/pointers>
#include <gtest/gtest.h>
#include <numeric>
#include <string>
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
    using namespace std;
    using std::vector;

    struct FSTIter{
        std::string & data;
        std::vector<size_t>& idxs;
        size_t node;
        size_t leaf{};
        bool found{};
    };

    template <typename Suffix>
    struct LoudsDense
    {
        using Builder = LoudsBuilder;

    public:

        struct Iter: public FSTIter {
            LoudsDense & trie;
            

            Iter(LoudsDense& trie, std::string & data, std::vector<size_t>& idxs, size_t node = 0):
                FSTIter{data, idxs, node},
                trie{trie}
                {};

            Iter& leftmost(size_t node){
                assert(idxs.size() == data.size());
                assert(trie.from_level <= data.size() && data.size() < trie.from_level + trie.n_levels);
                found = true;
                auto to_level = trie.from_level + trie.n_levels;
                auto level = idxs.size();
                for(level = trie.from_level; level < to_level; ++level){
                    auto pos_from = node * 256;
                    auto pos_to= (node + 1) * 256;
                    for (auto pos = pos_from; pos != pos_to; ++pos){
                        if (trie.labels[pos/256][pos&255]){
                            idxs.push_back(pos);
                            data.push_back(pos&255);
                            if (trie.has_child[pos/256][pos&255]){
                                node = trie.rank_c(pos) + trie.n_trees - 1; // get child node
                                break;
                            } else {
                                leaf = trie.value(pos);
                                return *this;
                            }
                        }
                    }
                }
                found = false;
                return *this;
            }


            Iter& rightmost(size_t node){
                assert(idxs.size() == data.size());
                assert(trie.from_level <= data.size() && data.size() < trie.from_level + trie.n_levels);
                found = true;
                auto to_level = trie.from_level + trie.n_levels;
                auto level = idxs.size();
                for(level = trie.from_level; level < to_level; ++level){
                    auto pos_from = (node + 1) * 256;
                    auto pos_to= node * 256;
                    for (auto pos = pos_from; pos-->pos_to;){
                        if (trie.labels[pos/256][pos&255]){
                            idxs.push_back(pos);
                            data.push_back(pos&255);
                            if (trie.has_child[pos/256][pos&255]){
                                node = trie.rank_c(pos) + trie.n_trees - 1; // get child node
                                break;
                            } else {
                                leaf = trie.value(pos);
                                return *this;
                            }
                        }
                    }
                }
                found = false;
                return *this;
            }

            Iter& begin(){
                return leftmost(0UL);
            }
            Iter& end(){
                return rightmost(0UL);
            }
            auto operator *() -> string_view { // only necessary for testing
                auto ptr = trie.values[leaf];
                return {ptr, strlen(ptr)};
            }
            Iter& operator ++(){
                assert(idxs.size() == data.size());
                assert(trie.from_level <= data.size() && data.size() <= trie.from_level + trie.n_levels);
                found = false;
                auto to_level = trie.from_level + trie.n_levels;
                auto next_pos = [&](auto pos){
                    auto begin = pos+1;
                    auto end = ((pos/256) + 1)* 256; //same as: 256*(pos/256 + 1)  but faster (asm: lea)
                    for (auto i = begin; i < end; ++i){
                        if (trie.labels[i/256][i&255]){
                            return i;
                        }
                    }
                    return end;
                };
                auto pos_node_first = [&](auto from, auto to) {
                    for (auto i = from; i != to; ++i){
                        if (trie.labels[i/256][i&255]) return i;
                    }
                    return to;
                };
                if (idxs.back()+1 == 256*trie.labels.size()){
                    idxs.pop_back();
                    data.pop_back();
                } // bounds check
                idxs.back() = next_pos(idxs.back());
                data.back() = idxs.back()&255;
                while(trie.from_level < data.size() && data.size() <= trie.from_level + trie.n_levels && (idxs.back()&255) == 0){ // traversing upwards
                    idxs.pop_back();
                    data.pop_back();
                    idxs.back() = next_pos(idxs.back());  
                    data.back() = idxs.back()&255;
                }
                if (!(trie.from_level < data.size() && data.size() <= trie.from_level + trie.n_levels)) {
                    return *this;
                }
                while(trie.from_level < data.size() && data.size() < trie.from_level + trie.n_levels && trie.has_child[idxs.back()/256][idxs.back()&255]){ // traversing downwards
                    auto node = trie.rank_c(idxs.back()) + trie.n_trees - 1;
                    auto from = node * 256;
                    auto to = from + 256;
                    idxs.push_back(pos_node_first(from, to));
                    data.push_back(idxs.back()&255);
                }
                if (!(trie.from_level < data.size() && data.size() < trie.from_level + trie.n_levels) && trie.has_child[idxs.back()/256][idxs.back()&255]) {
                    return *this;
                }
                leaf = trie.value(idxs.back());
                found = true;
                return *this;
            }
            Iter& operator --(){
                assert(idxs.size() == data.size());
                assert(trie.from_level <= data.size() && data.size() <= trie.from_level + trie.n_levels);
                auto to_level = trie.from_level + trie.n_levels;
                found = false;
                auto prev_pos = [&](auto pos){
                    auto begin = pos-1;
                    auto end = (pos >> 8) << 8;
                    for (auto i = begin; i >= end; --i){
                        if (trie.labels[i/256][i&255]){
                            return i;
                        }
                    }
                    return end-1;
                };
                auto pos_node_last = [&](size_t from, size_t to) -> size_t {
                    for (auto i = from; i-->to;){
                        if (trie.labels[i/256][i&255]) return i;
                    }
                    assert(false);
                    return 0UL;
                };
                idxs.back() = prev_pos(idxs.back());
                data.back() = idxs.back()&255;
                while(trie.from_level <= idxs.size() && idxs.size() <= to_level && (idxs.back()&255) == 255){ // traversing upwards
                    idxs.pop_back();
                    data.pop_back();
                    idxs.back() = prev_pos(idxs.back());
                    data.back() = idxs.back()&255;
                }
                while(trie.from_level <= idxs.size() && idxs.size() < to_level && trie.has_child[idxs.back()/256][idxs.back()&255]){ // traversing downwards
                    auto node = trie.rank_c(idxs.back()) + trie.n_trees - 1;
                    auto to =  node * 256UL;
                    auto from = to + 256UL;
                    idxs.push_back(pos_node_last(from, to));
                    data.push_back(idxs.back() & 255);
                }
                 if (!(trie.from_level < data.size() && data.size() < trie.from_level + trie.n_levels) && trie.has_child[idxs.back()/256][idxs.back()&255]) {
                    return *this;
                }
                leaf = trie.value(idxs.back());
                found = true;
                return *this;
            }

            Iter& lb(std::string_view key, size_t node, std::weak_ordering& cmp){
                auto to_level = std::min(trie.from_level + trie.n_levels, key.size());
                found = true;
                while(trie.from_level <= idxs.size() && idxs.size() < to_level && cmp == std::weak_ordering::equivalent){
                    auto level = idxs.size();
                    size_t found;
                    auto pos =  (node + 1) * 256;
                    auto end = node * 256;
                    while(pos--> end){
                        if (trie.labels[pos/256][pos&255]){
                            found = pos;
                            cmp = key[level] <=> static_cast<char>(pos&255);
                            if (cmp != std::weak_ordering::less) break;
                        }
                    }
                    idxs.push_back(found);
                    data.push_back(idxs.back()&255);
                    if(!trie.has_child[idxs.back()/256][idxs.back()&255]){
                        leaf = trie.value(idxs.back());
                        return *this;
                    }
                    node = trie.rank_c(idxs.back()) + trie.n_trees - 1;
                }
                if (trie.from_level <= idxs.size() && idxs.size() < to_level){
                    if (cmp == std::weak_ordering::less){
                        return leftmost(node);
                    }if(cmp == std::weak_ordering::greater){
                        return rightmost(node);
                    }
                }
                found = false;
                return *this;
            }

            Iter& ub(std::string_view key, size_t node, std::weak_ordering& cmp){
                auto to_level = std::min(trie.from_level + trie.n_levels, key.size());
                auto found = true;
                while(trie.from_level <= idxs.size() && idxs.size() < to_level && cmp == std::weak_ordering::equivalent){
                    auto level = idxs.size();
                    size_t found;
                    auto pos = node * 256;
                    auto end = (node+1) * 256;
                    while(pos < end){
                        if (trie.labels[pos/256][pos&255]){
                            found = pos;
                            cmp = key[level] <=> static_cast<char>(pos&255);
                            if (cmp != std::weak_ordering::greater) break;
                        }
                        ++pos;
                    }
                    idxs.push_back(found);
                    data.push_back(idxs.back()&255);
                    if(!trie.has_child[idxs.back()/256][idxs.back()&255]){
                        leaf = trie.value(idxs.back());
                        return *this;
                    }
                    node = trie.rank_c(idxs.back()) + trie.n_trees - 1;
                }
                if (trie.from_level <= idxs.size() && idxs.size() < to_level){
                    if (cmp == std::weak_ordering::less){
                        return leftmost(node);
                    }if(cmp == std::weak_ordering::greater){
                        return rightmost(node);
                    }
                }
                found = false;
                return *this;
            }

            friend bool operator==(const Iter& l, const Iter& r) {
                return l.data == r.data;
            }
        };
        
        auto begin(std::string & data, std::vector<size_t>& idxs, size_t node=0) -> Iter {
            return Iter{*this, data, idxs, node}.begin();
        }

        Iter end(std::string & data, std::vector<size_t>& idxs, size_t node=0) {
            return Iter{*this, data, idxs, node}.end();
        }

        Iter lb_iter(std::string_view key, std::string & data, std::vector<size_t>& idxs, size_t node=0) {
            std::weak_ordering cmp = std::weak_ordering::equivalent;
            return Iter{*this, data, idxs, node}.lb(key, node, cmp);
        }

        Iter ub_iter(std::string_view key, std::string & data, std::vector<size_t>& idxs, size_t node=0) {
            std::weak_ordering cmp = std::weak_ordering::equivalent;
            return Iter{*this, data, idxs, node}.ub(key, node, cmp);
        }

        size_t n_levels{};
        size_t from_level{};
        size_t to_level{};
        size_t n_trees{1};
        size_t n_trailing_children{};
        size_t max_size_key{};
        vector<std::bitset<256>> labels{};
        vector<std::bitset<256>> has_child{};
        vector<char const *> values;
        Suffix suffixes;
        vector<std::size_t> lut_has_child{};
        vector<std::size_t> lut_labels{};
        
        size_t get_memory_usage()
        {
            return std::size(labels) * sizeof(std::bitset<256UL>) +
                   std::size(has_child) * sizeof(std::bitset<256UL>) +
                   std::size(lut_has_child) * sizeof(std::size_t) +
                   std::size(lut_labels) * sizeof(std::size_t) + 
                   suffixes.size_bytes();
        }
        static void merge_and_insert(const auto &vs, auto &r, size_t capacity)
        {
            r.reserve(capacity);
            for (const auto &v : vs)
                r.insert(std::end(r), std::begin(v), std::end(v));
        }

        LoudsDense(range_value_string auto &&input): LoudsDense(Builder(std::forward<decltype(input)>(input))){}

        LoudsDense(is_builder auto&& builder) : LoudsDense(std::forward<decltype(builder)>(builder), 0, builder.n_levels, 0){};

        LoudsDense(is_builder auto&& builder, size_t from_level, size_t to_level, size_t n_trailing_children) : 
            from_level(from_level),
            to_level(to_level),
            n_levels(to_level - from_level),
            n_trailing_children(n_trailing_children),
            max_size_key(builder.max_size_key),
            values(merge(span{builder.values.data() + from_level, n_levels}, builder.n_leafs)),
            suffixes(merge(span{builder.suffixes.data() + from_level, n_levels}, builder.n_leafs))
        {

            auto nested_size = [](auto vs)
            {
                auto sum = 0;
                for (const auto &v : vs) {
                    sum += size(v);
                }
                return sum;
            };
            span builder_louds{builder.louds.data() + from_level, n_levels};
            span builder_suffix{builder.suffixes.data() + from_level, n_levels};
            span builder_n_nodes_level{builder.n_nodes_level.data() + from_level, n_levels};
            size_t n_nodes = 0UL;
            for (auto n_nodes_level : builder_n_nodes_level) n_nodes += n_nodes_level;
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
                    unsigned char c = builder.labels[level][i];
                    labels.back().set(c, true);
                    has_child.back().set(c, builder.has_child[level][i]);
                }
            }
            lut_has_child.resize(std::size(has_child));
            lut_labels.resize(std::size(labels));
            compute_rank(has_child, lut_has_child);
            compute_rank(labels, lut_labels);
        }

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
            for (auto level = 0UL; level < to_level; level++)
            {
                unsigned char token = word[level];
                pos = 256UL * node + token;
                if (labels[node][token])
                {
                    if (!has_child[node][token])
                    {
                        auto suffix = string_view{word.begin()+level+1, word.end()};
                        leaf = value(pos);
                        return suffixes.contains(leaf, suffix);
                    }
                }
                else
                {
                    return false;
                }
                node = rank_c(pos);
            }
            if (to_level < from_level + n_levels){
                if (labels[node]['\0']){
                    leaf = value(node*256UL);
                    return true;
                } else {
                    return false;
                }
            } else {
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

        struct Iter : public FSTIter{
            LoudsSparse & trie;
            

            Iter(LoudsSparse& trie, std::string & data, std::vector<size_t>& idxs, size_t node = 0):
                FSTIter{data, idxs, node},
                trie{trie}{};

            Iter& leftmost(size_t node){
                assert(idxs.size() == data.size());
                auto to_level = trie.from_level + trie.n_levels;
                found = true;
                auto level = idxs.size();
                for(; level < to_level; ++level){
                    auto first_pos = trie.louds.select(node + 1);
                    idxs.push_back(first_pos);
                    data.push_back(trie.labels[first_pos]);
                    if (trie.has_child[first_pos]){
                        node = trie.has_child.rank(first_pos) + trie.n_trees - 1;
                    } else {
                        leaf = trie.value(first_pos);
                        return *this;
                    }
                }
                found = false;
                return *this;
            }


            Iter& rightmost(size_t node){
                assert(idxs.size() == data.size());
                auto to_level = trie.from_level + trie.n_levels;
                found = true;
                auto level = idxs.size();
                for(; level < to_level; ++level){
                    auto last_pos = trie.louds.select(node + 2)-1;
                    idxs.push_back(last_pos);
                    data.push_back(trie.labels[last_pos]);
                    if (trie.has_child[last_pos]){
                        node = trie.has_child.rank(last_pos) + trie.n_trees - 1;
                    } else {
                        leaf = trie.value(last_pos);
                        return *this;
                    }
                }
                found = false;
                return *this;
            }

            Iter& begin(){
                return leftmost(0);
            }
            Iter& end(){ 
                return rightmost(0);
            }
            auto lb(std::string_view word,size_t node, std::weak_ordering& cmp) -> Iter&{
                assert(idxs.size() == data.size());
                assert(trie.from_level <= data.size() && data.size() < trie.from_level + trie.n_levels);
                auto level=data.size();
                auto to_level = std::min(trie.from_level + trie.n_levels, size(word));
                size_t pos, safe_pos, safe_level;
                found = false;
                while(level < to_level && cmp == std::weak_ordering::equivalent){
                    auto pos_end = trie.louds.select(node + 1);
                    auto pos = trie.louds.next(pos_end)-1;
                    while ((cmp = word[level] <=> trie.labels[pos]) == std::strong_ordering::less && pos != pos_end) --pos;
                    if (cmp == std::strong_ordering::equal && pos != pos_end){
                        safe_pos = pos_end;
                        safe_level = level;
                    }
                    if (cmp == std::weak_ordering::less){
                        data.erase(safe_level);
                        data.push_back(trie.labels[safe_pos]);
                        if (!trie.has_child[safe_pos]){
                            leaf = trie.value(safe_pos);
                            return *this;
                        } 
                        node = trie.has_child.rank(safe_pos) + trie.n_trees-1;
                        return rightmost(node);
                    } 
                    idxs.push_back(pos);
                    data.push_back(trie.labels[pos]);
                    if (!trie.has_child[pos]){
                        found = true;
                        leaf = trie.value(pos);
                        return *this;
                    }
                    node = trie.has_child.rank(pos) + trie.n_trees-1; // get child node
                    ++level;     
                }

                if(cmp == std::weak_ordering::greater){
                    return rightmost(node);
                }               
                return leftmost(node);

            }


            auto ub(std::string_view word, size_t node, std::weak_ordering& cmp) -> Iter&{
                assert(idxs.size() == data.size());
                assert(trie.from_level <= data.size() && data.size() < trie.from_level + trie.n_levels);
                auto level=data.size();
                auto to_level = std::min(trie.from_level + trie.n_levels, size(word));
                size_t pos, safe_pos, safe_level;
                found = false;
                while(level < to_level && cmp == std::weak_ordering::equivalent){
                    auto pos = trie.louds.select(node + 1);
                    auto pos_end = trie.louds.next(pos);
                    while (pos != pos_end && (cmp = word[level] <=> trie.labels[pos]) == std::strong_ordering::greater) ++pos;
                    if (cmp == std::strong_ordering::equal && pos < pos_end-1){
                        safe_pos = pos_end-1;
                        safe_level = level;
                    }
                    if (cmp == std::weak_ordering::greater){
                        data.erase(safe_level);
                        data.push_back(trie.labels[safe_pos]);
                        if (!trie.has_child[safe_pos]){
                            leaf = trie.value(safe_pos);
                            found = true;
                            return *this;
                        } 
                        node = trie.has_child.rank(safe_pos) + trie.n_trees-1;
                        return leftmost(node);
                    } 
                    idxs.push_back(pos);
                    data.push_back(trie.labels[pos]);
                    if (!trie.has_child[pos]){
                        leaf = trie.value(pos);
                        found = true;
                        return *this;
                    }

                    node = trie.has_child.rank(pos) + trie.n_trees-1; // get child node
                    ++level;     
                }

                if(cmp == std::weak_ordering::greater){
                    return rightmost(node);
                }               
                return leftmost(node);
            }



            auto operator ++() -> Iter&{
                assert(idxs.size() == data.size());
                assert(trie.from_level < data.size() && data.size() <= trie.from_level + trie.n_levels);
                auto level = idxs.size()-1;
                found = false;
                if (idxs[level]+1 == trie.louds.size()){ // bounds check
                    --level;
                    idxs.pop_back();
                    data.pop_back();
                }
                while(trie.from_level < data.size() && data.size() <= trie.from_level + trie.n_levels && trie.louds[++idxs[level]]){ // traversing upwards
                    --level;
                    idxs.pop_back();
                    data.pop_back();
                }
                if (!(trie.from_level < data.size() && data.size() <= trie.from_level + trie.n_levels)) {
                    return *this;
                }
                data[level] = trie.labels[idxs.back()];
                while(trie.has_child[idxs[level]] && trie.from_level < data.size() && data.size() <= trie.from_level + trie.n_levels){ // traversing downwards
                    auto node = trie.has_child.rank(idxs[level++]) + trie.n_trees - 1;
                    auto pos = trie.louds.select(node + 1);
                    idxs.push_back(pos); // first label pos in node
                    data.push_back(trie.labels[pos]); // first character in node
                }
                if (!(trie.from_level < data.size() && data.size() <= trie.from_level + trie.n_levels)) {
                    return *this;
                }
                leaf = trie.value(idxs.back());
                found = true;
                return *this;
            }

            auto operator --() -> Iter&{
                assert(idxs.size() == data.size());
                assert(trie.from_level <= data.size() && data.size() <= trie.from_level + trie.n_levels);
                found = false;
                while(trie.from_level < data.size() && data.size() <= trie.from_level + trie.n_levels && trie.louds[--idxs.back()+1]){ // traversing upwards
                    idxs.pop_back();
                    data.pop_back();
                }
                if (!(trie.from_level < data.size() && data.size() <= trie.from_level + trie.n_levels)) {
                    return *this;
                }
                data.back() = trie.labels[idxs.back()];
                while(trie.has_child[idxs.back()] && trie.from_level <= data.size() && data.size() <= trie.from_level + trie.n_levels){ // traversing downwards
                    auto node = trie.has_child.rank(idxs.back()) + trie.n_trees - 1;
                    auto pos_last = trie.louds.select(node + 2)-1;
                    idxs.push_back(pos_last);
                    data.push_back(trie.labels[pos_last]);
                }
                if (!(trie.from_level < data.size() && data.size() <= trie.from_level + trie.n_levels)) {
                    return *this;
                }
                leaf = trie.value(idxs.back());
                found = true;
                return *this;
            }

            auto operator * () -> string_view {
                auto ptr = trie.values[leaf];
                return {ptr, strlen(ptr)};
            }

            friend auto operator==(const Iter& left, const Iter& right) -> bool {
                return left.data == right.data;
            }
        };

        Iter begin(std::string & data, std::vector<size_t>& idxs, size_t node=0) {
            return Iter{*this, data, idxs, node}.begin();
        }

        Iter end(std::string & data, std::vector<size_t>& idxs, size_t node=0) {
            return Iter{*this, data, idxs, node}.end();
        }

        Iter lb_iter(std::string_view key, std::string & data, std::vector<size_t>& idxs, size_t node=0) {
            std::weak_ordering cmp = std::weak_ordering::equivalent;
            return Iter{*this, data, idxs, node}.lb(key, node, cmp);
        }

        Iter ub_iter(std::string_view key, std::string & data, std::vector<size_t>& idxs, size_t node=0) {
            std::weak_ordering cmp = std::weak_ordering::equivalent;
            return Iter{*this, data, idxs, node}.ub(key, node, cmp);
        }

        size_t n_levels{};
        size_t from_level{};
        size_t to_level{};
        size_t n_trees{};
        size_t max_size_key{};
        vector<char> labels;
        BitVector<64> has_child;
        BitVector<64> louds;
        vector<char const *> values;
        Suffix suffixes;

        size_t get_memory_usage()
        {
            return std::size(labels) + has_child.get_memory_usage() + louds.get_memory_usage();
        }

        LoudsSparse(std::ranges::range auto&& input) : LoudsSparse(Builder(std::forward<decltype(input)>(input))){};
        LoudsSparse(is_builder auto&& builder) : LoudsSparse(std::forward<decltype(builder)>(builder), 0, builder.n_levels, 1, builder.n_nodes){};
        LoudsSparse(is_builder auto&& builder, size_t from_level, size_t to_level, size_t n_trees, size_t n_nodes) : 
            n_levels(to_level - from_level),
            from_level(from_level),
            to_level(to_level),
            n_trees([&](){
                if (from_level > 0UL) { return count(builder.has_child[from_level-1],true);
                } return 1UL; }()),
            max_size_key(builder.max_size_key),
            labels(merge(span{builder.labels.data() + from_level, n_levels}, n_nodes)),
            has_child(merge(span{builder.has_child.data() + from_level, n_levels}, n_nodes)),
            louds(merge(span{builder.louds.data() + from_level, n_levels}, n_nodes)),
            values(merge(span{builder.values.data() + from_level, n_levels}, builder.n_leafs)),
            suffixes(merge(span{builder.suffixes.data() + from_level, n_levels}, builder.n_leafs))
            {};

        size_t rank_c(size_t pos) __attribute__((const))
        {
            assert(pos < std::size(labels));
            return has_child.rank(pos);
        }
        size_t rank_l(size_t pos) __attribute__((const))
        {
            assert(pos < std::size(labels));
            return louds.rank(pos);
        }

        size_t select_c(size_t count) __attribute__((const))
        {
            assert(count <= rank_c(std::size(labels)));
            return has_child.select(count);
        }

        size_t select_l(size_t count) __attribute__((const))
        {
            assert(count <= rank_l(std::size(labels)));
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

        size_t child(size_t pos)
        {
            return select_l(rank_c(pos) + 1 + n_trees);
        }

        size_t child_end(size_t pos)
        {
            return select_l(rank_c(pos) + 2 + n_trees);
        }

        size_t parent(size_t pos)
        {
            return select_c(rank_l(pos) - 1 + n_trees);
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

        void traverse_lm(size_t &leaf, size_t node = 0, size_t from_level = 0) {
            auto to_level = from_level + n_levels;
            for(auto level = from_level; level < to_level; ++level){
                auto pos_first = louds.select(node + 1);
                if (has_child[pos_first]){
                    node = has_child.rank(pos_first) + n_trees-1; // get child node
                } else {
                    leaf = value(pos_first);
                    return;
                }
            }
        }
        void traverse_rm(size_t &leaf, size_t node = 0, size_t from_level = 0) {
            auto to_level = from_level + n_levels;
            for(auto level = from_level; level < to_level; ++level){
                auto pos_last = louds.select(node + 2) - 1;
                if (has_child[pos_last]){
                    node = has_child.rank(pos_last) + n_trees-1; // get child node
                } else {
                    leaf = value(pos_last);
                    return;
                }
            }
            assert(false);
        }

        auto lb(std::string_view word) -> std::string{
            size_t leaf;
            traverse_lb(word, leaf);
            return {values[leaf]};
        }

        auto traverse_lb(std::string_view word, size_t &leaf, size_t node = 0, size_t from_level = 0, std::strong_ordering cmp = std::strong_ordering::equal) -> bool{
            auto level=from_level;
            auto to_level = std::min(from_level + n_levels, size(word));
            size_t pos, safe_pos, safe_level;

            
            while(level < to_level && cmp == std::strong_ordering::equal){
                auto pos_end = louds.select(node + 1);
                auto pos = louds.next(pos_end)-1;
                while ((cmp = word[level] <=> labels[pos]) == std::strong_ordering::less && pos != pos_end) --pos;
                if (cmp == std::strong_ordering::equal && pos != pos_end){
                    safe_pos = pos_end;
                    safe_level = level;
                }
                if (cmp == std::strong_ordering::less){
                    if (!has_child[safe_pos]){
                        leaf = value(safe_pos);
                        return true;
                    } 
                    node = has_child.rank(safe_pos) + n_trees-1;
                    traverse_rm(leaf, node,safe_level+1);
                    return true;
                } 
                if (!has_child[pos]){
                    leaf = value(pos);
                    return cmp != std::strong_ordering::equal;
                }
                node = has_child.rank(pos) + n_trees-1; // get child node
                ++level;     
            }

            if(cmp == std::strong_ordering::greater){
                traverse_rm(leaf, node, level);
                return true;
            }               
            traverse_lm(leaf, node, level);
            return cmp != std::strong_ordering::equal;
           
        }

        auto ub(std::string_view word) -> std::string{
            size_t leaf;
            traverse_ub(word, leaf);
            return {values[leaf]};
        }

    
        auto look_up_range(std::string_view from, std::string_view to){
            return ub(from) <= lb(to);
        }


        auto traverse_ub(std::string_view word, size_t &leaf, size_t node = 0, size_t from_level = 0, std::strong_ordering cmp = std::strong_ordering::equal) -> bool{
            auto level=from_level;
            auto to_level = std::min(from_level + n_levels, size(word));
            size_t pos, safe_pos, safe_level;
            
            while(level < to_level && cmp == std::strong_ordering::equal){
                auto pos = louds.select(node + 1);
                auto pos_end = louds.next(pos);
                while (pos != pos_end && (cmp = word[level] <=> labels[pos]) == std::strong_ordering::greater) ++pos;
                if (cmp == std::strong_ordering::equal && pos != pos_end-1){
                    safe_pos = pos_end;
                    safe_level = level;
                }
                if (cmp == std::strong_ordering::greater){
                    if (!has_child[safe_pos]){
                        leaf = value(safe_pos);
                        return true;
                    } 
                    node = has_child.rank(safe_pos) + n_trees-1;
                    traverse_lm(leaf, node,safe_level+1);
                    return true;
                } 
                if (!has_child[pos]){
                    leaf = value(pos);
                    return cmp != std::strong_ordering::equal;
                }
                node = has_child.rank(pos) + n_trees-1; // get child node
                ++level;     
            }
     
            traverse_lm(leaf, node, level);
            return cmp != std::strong_ordering::equal;
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
                    string_view suffix{word.data()+level+1, word.data() + word.size()};
                    leaf = value(pos); 
                    return suffixes.contains(leaf, suffix); // chech if the suffix is the same
                } else {
                    auto node = has_child.rank(pos) + n_trees-1;                              // get child node
                    pos_begin = louds.select(node + 1); // get start pos node
                    pos_end = louds.next(pos_begin);                      // get end pos node
                }
            }
            auto pos = find(pos_begin, pos_end, '\0');
            if (pos != pos_end) {
                leaf = value(pos);
                return true;
            }
            return false;
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

        struct Iter{
            enum State {Dense, Sparse};
            typename LoudsDense<Suffix>::Iter iter_dense;
            typename LoudsSparse<Suffix>::Iter iter_sparse;
            State doing{Dense};
            auto get_node() -> size_t{
                auto node_dense = iter_dense.trie.rank_c(iter_dense.idxs.back()) + iter_dense.trie.n_trees - 1;
                auto node_sparse = ((node_dense - iter_dense.trie.labels.size()) * 256UL) / 256UL;
                return node_sparse;
            }

            auto begin() -> Iter&{
                iter_dense.leftmost(0);
                if(iter_dense.found) return *this;
                doing = State::Sparse;
                iter_sparse.leftmost(get_node());
                return *this;
            }

            auto end() -> Iter&{
                iter_dense.rightmost(0);
                if(iter_dense.found) return *this;
                doing = State::Sparse;
                iter_sparse.rightmost(get_node());
                return *this;
            }

            auto lb(std::string_view key) -> Iter&{
                auto cmp = std::weak_ordering::equivalent;
                iter_dense.lb(key, 0, cmp);
                if(iter_dense.found) return *this;
                doing = State::Sparse;
                auto node_sparse = get_node();
                if (cmp == std::weak_ordering::equivalent) { 
                    iter_sparse.lb(key, node_sparse, cmp);
                } else if (cmp ==  std::weak_ordering::greater) { 
                    iter_sparse.rightmost(node_sparse);
                } else {
                    iter_sparse.leftmost(node_sparse);
                }
                return *this;
            }

            auto ub(std::string_view key) -> Iter&{
                auto cmp = std::weak_ordering::equivalent;
                iter_dense.ub(key, 0, cmp);
                if(iter_dense.found) return *this;
                doing = State::Sparse;
                auto node_sparse = get_node();
                if (cmp == std::weak_ordering::equivalent) { 
                    iter_sparse.ub(key, node_sparse, cmp);
                } else if (cmp ==  std::weak_ordering::greater) { 
                    iter_sparse.rightmost(node_sparse);
                } else {
                    iter_sparse.leftmost(node_sparse);
                }
                return *this;
            }

            auto operator ++() -> Iter&{
                if (doing == State::Dense){
                    ++iter_dense;
                    if (iter_dense.found) return *this;
                    doing = State::Sparse;
                    iter_sparse.leftmost(get_node());
                    return *this;
                }
                ++iter_sparse;
                if(iter_sparse.found) return *this;
                doing = State::Dense;
                ++iter_dense;
                if(iter_dense.found) return *this;
                doing = State::Sparse;
                iter_sparse.leftmost(get_node());
                return *this;

            }
            auto operator --() -> Iter&{
                if (doing == State::Dense){
                    --iter_dense;
                    if (iter_dense.found) return *this;
                    doing = State::Sparse;
                    iter_sparse.rightmost(get_node());
                    return *this;
                } 
                --iter_sparse;
                if(iter_sparse.found) return *this;
                doing = State::Dense;
                --iter_dense;
                if(iter_dense.found) return *this;
                doing = State::Sparse;
                iter_sparse.rightmost(get_node());
                return *this; 
            }

            auto operator *() -> string_view{
                if (doing == State::Dense){
                    return *iter_dense;
                }
                return *iter_sparse;
            }


            friend auto operator==(const Iter& l, const Iter& r) -> bool {
                return l.iter_dense == r.iter_dense;
            }
        };
       
        Iter begin(std::string & data, std::vector<size_t>& idxs) {
            return Iter{.iter_dense={ld, data, idxs}, .iter_sparse={ls, data, idxs}}.begin();
        }

        Iter end(std::string & data, std::vector<size_t>& idxs) {
            return Iter{.iter_dense={ld, data, idxs}, .iter_sparse={ls, data, idxs}}.end();
        }

        Iter lb_iter(std::string_view key, std::string & data, std::vector<size_t>& idxs) {
            return Iter{.iter_dense={ld, data, idxs}, .iter_sparse={ls, data, idxs}}.lb(key);
        }

        Iter ub_iter(std::string_view key, std::string & data, std::vector<size_t>& idxs) {
            return Iter{.iter_dense={ld, data, idxs}, .iter_sparse={ls, data, idxs}}.ub(key);
        }



        auto get_memory_usage() -> size_t
        {
            return ld.get_memory_usage() + ls.get_memory_usage();
        }

        auto n_nodes_level(auto builder, auto level) -> std::size_t
        {
            return std::count(std::begin(builder.louds[level]), std::end(builder.louds[level]), true);
        };

        Surf(range_value_string auto&& input): Surf(Builder(std::forward<decltype(input)>(input))){}

        Surf(is_builder auto&& builder):
            n_dense_levels(builder.cutoff_level),
            n_sparse_levels(builder.n_levels - builder.cutoff_level),
            n_nodes_dense_levels(std::accumulate(builder.n_nodes_level.begin(), builder.n_nodes_level.begin()+n_dense_levels, 0UL,  std::plus<size_t>())),
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
                    return (n_sparse_levels > 0) && ls.traverse(word,leaf, node, level); };
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
                    { return (n_sparse_levels > 0) && ls.traverse(word,leaf, node, level); };
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
