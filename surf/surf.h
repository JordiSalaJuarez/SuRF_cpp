#include<vector>
#include<bitset>
#include<algorithm>
#include<span>
#include<string.h>
#include<optional>
#include<iostream>
#include<limits>
#include "utils/bit_vector.h"

namespace yas{

    using namespace std;

    inline constexpr size_t failed_query = numeric_limits<size_t>::max();

    struct LoudsBuilder {
        public:
        vector<vector<char>> labels{};
        vector<vector<bool>> has_child{};
        vector<vector<bool>> louds{};
        vector<vector<bool>> is_prefix{};
        vector<vector<string>> suffix{};
        size_t n_levels{};
        size_t max_size_key{};


        void insert_key(const string& key, size_t start, size_t end){
            max_size_key = std::max(max_size_key, size(key));
            if (end+1 > n_levels){
                n_levels = end+1;
                labels.resize(n_levels);
                has_child.resize(n_levels);
                louds.resize(n_levels);
                is_prefix.resize(n_levels);
                suffix.resize(n_levels);
            }
            for (auto level = start; level < end; level++) { // insert new non leaf and nodes 
                labels[level].emplace_back(key[level]);
                has_child[level].emplace_back(true);
                is_prefix[level].emplace_back(false);
                louds[level].emplace_back(start != level);
            }
            if(end < size(key)){
                // insert new leaf node
                labels[end].emplace_back(key[end]);
                has_child[end].emplace_back(false);
                is_prefix[end].emplace_back(false);
                louds[end].emplace_back(start != end);
                // store suffix
                suffix[end].emplace_back(key.substr(end+1));
            } else {
                labels[end].emplace_back('\0');
                has_child[end].emplace_back(false);
                is_prefix[end].emplace_back(true);
                louds[end].emplace_back(start != end);
                suffix[end].emplace_back("");
            }
        }

        void add_dollar(const auto& i){
            if (i+2 > n_levels){
                n_levels = i+2;
                labels.resize(n_levels);
                has_child.resize(n_levels);
                louds.resize(n_levels);
            }
            has_child[i][size(has_child[i])] = true;
            labels[i+1].emplace_back('\0');
            louds[i+1].emplace_back(true);
            has_child[i+1].emplace_back(false);
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

        static LoudsBuilder from_stream(istream &input){
            LoudsBuilder ans{};
            string key{}, next_key{};
            if (!getline(input, key)) return ans;
            while(getline(input, next_key)){
                auto lca_prev = ans.lowest_common_ancestor_prev(key);
                auto lca_next = ans.lowest_common_ancestor_next(key, next_key);
                ans.insert_key(key, lca_prev, max(lca_prev, lca_next));
                key = next_key; // is this going to be a copy? move?
            }
            auto lca_prev = ans.lowest_common_ancestor_prev(key);
            ans.insert_key(key, lca_prev, lca_prev);
            ans.louds[0][0] = true;

            return ans;
        }
        static LoudsBuilder from_vector(const vector<string> &input){
            LoudsBuilder ans{};
            string key{}, next_key{};
            if (size(input) == 0) return std::move(ans);
            for(const auto &next_key: input){
                auto lca_prev = ans.lowest_common_ancestor_prev(key);
                auto lca_next = ans.lowest_common_ancestor_next(key, next_key);
                ans.insert_key(key, lca_prev, max(lca_prev, lca_next));
                key = next_key; // is this going to be a copy? move?
            }
            auto lca_prev = ans.lowest_common_ancestor_prev(key);
            ans.insert_key(key, lca_prev, lca_prev);
            ans.louds[0][0] = true;

            return std::move(ans);
        }
    };

    struct Positions {
        vector<size_t> positions;
        size_t level;
        public:
        decltype(auto) current () const {return positions[level];}
        decltype(auto) operator[] (size_t i) const { return positions[i];};
        decltype(auto) operator[] (size_t i) { return positions[i];};
        inline bool operator== (const Positions& other) {return current() == other.current();}
        size_t size() const {return positions.size();}
        Positions(size_t n_pos): positions(n_pos, 0), level{0}{}
    };

    struct LoudsDense {
        public:
        struct Iter {
            size_t from_level;
            size_t to_level;
            size_t n_roots;
            size_t node;
            size_t pos;
            bool has_child;
            int level;
            shared_ptr<Positions> idxs;
            LoudsDense *louds;

            Iter(LoudsDense *louds, shared_ptr<Positions> positions, size_t node = 0, size_t from_level = 0, size_t n_roots = 0): 
                from_level{from_level},
                to_level{from_level + louds->n_levels},
                n_roots{n_roots},
                node{node},
                pos{0},
                has_child{false},
                level{from_level}, 
                idxs{positions}, 
                louds{louds}{}

            Iter& begin(){
                auto pos_node_first = [&](auto node, auto level) {
                    if (level == from_level)
                        return 256 * node;
                    else
                        return 256 * (node + n_roots);
                };
                for(level = from_level; level < to_level; ++level){
                    auto begin_ = pos_node_first(node, level);
                    auto end_ = begin_ + 256;
                    for (pos = begin_; pos != end_; ++pos){
                        if (louds->labels[pos/256][pos&255]){
                            (*idxs)[level] = pos;
                            if (louds->has_child[pos/256][pos&255]){
                                node = louds->rank_c(pos); // get child node
                                break;
                            } else {
                                idxs->level = level;
                                return *this;
                            }
                        }    
                    }
                }
                has_child = louds->has_child[pos/256][pos&255];
                level = has_child? level-1:level;
                idxs->level = level;
                return *this;
            }
            Iter& end(){
                auto pos_node_last = [&](auto node, auto level) {
                    if (level == from_level)
                        return 256 * (node + 1) - 1;
                    else
                        return 256 * (node + n_roots + 1) - 1;
                };
                level = from_level;
                while(true){
                    auto begin_ = pos_node_last(node, level);
                    auto end_ = begin_ - 256;
                    for (pos = begin_; pos != end_; --pos){
                        if (louds->labels[pos/256][pos&255]){
                            (*idxs)[level] = pos;
                            if (louds->has_child[pos/256][pos&255]){
                                ++level;
                                node = louds->rank_c(pos); // get child node
                                break;
                            } else {
                                idxs->level = level;
                                return *this;
                            }
                        }    
                    }
                }
                has_child = louds->has_child[pos/256][pos&255];
                level = has_child? level-1:level;
                idxs->level = level;
                return *this;
            }

            auto operator *(){ // only necessary for testing
                struct PrefixSuffix {
                    string prefix;
                    string suffix;
                };
                string prefix{};
                prefix.reserve(level-from_level);
                for (auto i = from_level; i <= level; ++i){
                    prefix.push_back((*idxs)[i]%256);
                }
                if (prefix.back() == '\0') prefix.pop_back();
                string suffix{has_child? "":louds->values[louds->value(pos)]};
                
                return PrefixSuffix{prefix, suffix};
            }

            Iter& operator ++(){
                auto next_pos = [&](auto pos){
                    auto begin = pos+1;
                    auto end = ((pos >> 8) + 1) << 8; //same as: 256*(pos/256 + 1)  but faster (asm: lea)
                    for (auto i = begin; i < end; ++i){
                        if (louds->labels[i/256][i&255]){
                            return i;
                        }
                    }
                    return end;
                };
                auto pos_node_first = [&](auto node, auto level) {
                    auto begin = level == from_level? 256 * node: 256 * (node + n_roots);
                    auto end = level == from_level? 256 * (node + 1): 256 * (node + n_roots + 1); 
                    for (auto i = begin; i != end; ++i){
                        if (louds->labels[i/256][i&255]) return i;
                    }
                    return end;
                };
                
                if ((*idxs)[level]+1 == 256*size(louds->labels)) --level; // bounds check
                (*idxs)[level] = next_pos((*idxs)[level]);
                while(from_level < level && ((*idxs)[level]&255) == 0){ // traversing upwards
                    --level;
                    (*idxs)[level] = next_pos((*idxs)[level]);
                }
                while(level + 1 < to_level && louds->has_child[(*idxs)[level]/256][(*idxs)[level]&255]){ // traversing downwards
                    (*idxs)[level+1] = pos_node_first(louds->rank_c((*idxs)[level]), level + 1);
                    ++level;
                }
                pos = (*idxs)[level];
                // pos is leaf
                has_child = louds->has_child[pos/256][pos&255];
                idxs->level = level;
                return *this;
            }

            Iter& operator --(){
                auto prev_pos = [&](auto pos){
                    auto begin = pos-1;
                    auto end = (pos >> 8) << 8; //same as: 256*(pos/256 + 1)  but faster (asm: lea)
                    for (auto i = begin; i >= end; --i){
                        if (louds->labels[i/256][i&255]){
                            return i;
                        }
                    }
                    return end-1;
                };
                auto pos_node_last = [&](auto node, auto level) {
                    auto begin = level == from_level? 256 * (node + 1) - 1: 256 * (node + n_roots + 1)-1;
                    auto end = level == from_level? 256 * node - 1: 256 * (node + n_roots)-1; 
                    for (auto i = begin; i != end; --i){
                        if (louds->labels[i/256][i&255]) return i;
                    }
                    return end;
                };
                (*idxs)[level] = prev_pos((*idxs)[level]);
                while(from_level < level && ((*idxs)[level]&255) == 255){ // traversing upwards
                    --level;
                    (*idxs)[level] = prev_pos((*idxs)[level]);
                }
                while(level + 1 < to_level && louds->has_child[(*idxs)[level]/256][(*idxs)[level]&255]){ // traversing downwards
                    (*idxs)[level+1] = pos_node_last(louds->rank_c((*idxs)[level]), level + 1);
                    ++level;
                }
                pos = (*idxs)[level];
                // pos is leaf
                has_child = louds->has_child[pos/256][pos&255];
                idxs->level = level;
                return *this;
            }
            Iter& lb(string_view key){
                auto find = [&](auto &container, size_t begin, size_t end){
                    auto i = begin;
                    while(i != end && !container[i/256][i&255]) ++i;
                    return i;
                };
                auto pos_node_first = [&](auto node, auto level) {
                    if (level == from_level)
                        return 256 * node;
                    else
                        return 256 * (node + n_roots);
                };
                auto pos_node_last = [&](auto node, auto level) {
                    if (level == from_level)
                        return 256 * (node+1);
                    else
                        return 256 * (node + n_roots + 1);
                };
                auto pos_node_ge = [&](auto node, auto level, auto value) {
                    auto begin = pos_node_first(node, level);
                    auto end = pos_node_last(node, level);
                    return find(louds->labels, begin + value , end);
                };
                auto on_boundary = true;
                level = from_level;
                for(level = from_level; level < to_level; ++level){
                    if (on_boundary){ // get closest value to boundary
                        pos = pos_node_ge(node, level, key[level]);
                        on_boundary = pos%256 == key[level] && level+1 < size(key);
                    }else{ // get left-most value
                        pos = pos_node_ge(node, level, 0);
                    }
                    (*idxs)[level] = pos; // set cursor to last pos
                    if (louds->has_child[pos/256][pos&255] && pos/256 == (level == from_level? node:node+n_roots)){
                        node = louds->rank_c(pos); // get child node
                    } else {
                        idxs->level = level;
                        return *this;
                    }
                }
                has_child = louds->has_child[pos/256][pos&255];
                level = has_child? level-1:level;
                idxs->level = level;
                return *this;
            }

            Iter& ub(string_view key){
                auto find_reverse = [&](auto &container, size_t begin, size_t end){
                    auto i = begin;
                    while(i != end && !container[i/256][i&255]) --i;
                    return i;
                };
                auto pos_node_first = [&](auto node, auto level) {
                    if (level == from_level)
                        return 256 * node;
                    else
                        return 256 * (node + n_roots);
                };
                auto pos_node_last = [&](auto node, auto level) {
                    if (level == from_level)
                        return 256 * (node+1) - 1;
                    else
                        return 256 * (node + n_roots + 1) - 1;
                };
                auto pos_node_le = [&](auto node, auto level, auto key) {
                    auto end = pos_node_first(node, level);
                    return find_reverse(louds->labels, end+key, end-1);
                };
                auto on_boundary = true;
                for(level = from_level; level < to_level; ++level){
                    if (on_boundary){ // get closest value to boundary
                        pos = pos_node_le(node, level, key[level]);
                        on_boundary = pos%256 == key[level] && level+1 < size(key);
                    }else { // get left-most value
                        pos = pos_node_le(node, level, 255);
                    }
                    (*idxs)[level] = pos; // set cursor to last pos
                    if (louds->has_child[pos/256][pos&255] && pos/256 == (level == from_level? node:node+n_roots)){
                        node = louds->rank_c(pos); // get child node
                    } else {
                        break;
                    }
                }
                has_child = louds->has_child[pos/256][pos&255];
                level = has_child? level-1:level;
                idxs->level = level;
                return *this;
            }

            friend bool operator==(const Iter& l, const Iter& r) {
                assert(l.louds == r.louds);
                return (*l.idxs)[l.idxs->level] == (*r.idxs)[r.idxs->level];
            }

    
        };

        Iter begin(){
            return Iter{this, make_shared<Positions>(this->to_level)}.begin();
        }
        Iter begin(shared_ptr<Positions> pos){
            return Iter{this, make_shared<Positions>(this->to_level)}.begin();
        }

        Iter end(){
            return Iter{this, make_shared<Positions>(this->to_level)}.end();
        }
        Iter end(shared_ptr<Positions> pos){
            return Iter{this, pos}.end();
        }

        Iter lb(string_view key){
            return Iter{this, make_shared<Positions>(this->to_level)}.lb(key);
        }

        Iter lb(string_view key, shared_ptr<Positions> pos){
            return Iter{this, pos}.lb(key);
        }

        Iter ub(string_view key){
            return Iter{this, make_shared<Positions>(this->to_level)}.ub(key);
        }

        Iter ub(string_view key, shared_ptr<Positions> pos){
            return Iter{this, pos}.ub(key);
        }

        vector<std::bitset<256>> labels{};
        vector<std::bitset<256>> has_child{};
        vector<bool> is_prefix_key{};
        vector<string> values{};
        size_t n_levels{};
        size_t from_level{};
        size_t to_level{};
        size_t n_trailing_children{};
        size_t max_size_key{};

        static void merge_and_insert(const auto & vs, auto &r, size_t capacity){
            r.reserve(capacity);
            for (const auto &v: vs) r.insert(std::end(r), std::begin(v), std::end(v)); 
        }

        static LoudsDense from_builder(const LoudsBuilder &builder, size_t from_level = 0, size_t to_level = -1, size_t n_trailing_children=0){
            
            auto nested_size = [](auto vs){
                auto sum = 0;
                for (const auto& v: vs) sum += size(v);
                return sum;
            };
            
            if (to_level == -1) to_level = builder.n_levels;
            LoudsDense result{};
            result.from_level = from_level;
            result.to_level = to_level;
            result.n_levels = to_level - from_level;
            result.n_trailing_children = n_trailing_children;
            result.max_size_key = builder.max_size_key;
            auto builder_louds = span(std::begin(builder.louds)+from_level, std::begin(builder.louds) + to_level);
            auto builder_suffix = span(std::begin(builder.suffix)+from_level, std::begin(builder.suffix) + to_level);
            auto sum_size = 0;
            for (const auto &v: builder_louds) sum_size += std::count(std::begin(v), std::end(v), true);
            auto sum_size_values = nested_size(builder_suffix);
            merge_and_insert(builder_suffix, result.values, sum_size_values);  
            result.labels.reserve(sum_size);
            result.has_child.reserve(sum_size);
            result.is_prefix_key.reserve(sum_size);
            auto n = builder.n_levels;
            for(auto level=from_level; level < to_level; level++){
                auto m = size(builder.louds[level]);
                for(auto i=0; i < m; i++){
                    if (builder.louds[level][i]){
                        result.labels.emplace_back();
                        result.has_child.emplace_back();
                        result.is_prefix_key.emplace_back(builder.is_prefix[level][i]);
                    }
                    auto c = builder.labels[level][i];
                    result.labels.back().set(c, true);
                    result.has_child.back().set(c, builder.has_child[level][i]);
                }
            }
            return result;
        }  

        size_t rank_c(size_t pos) __attribute__((const)) {
            auto n = (pos + 1) / 256; 
            auto m = (pos + 1) % 256;
            auto count = 0;
            auto i = 0;
            for (; i < n; i++)
                count += has_child[i].count();
            for(auto j = 0; j < m; j++)
                count += has_child[n][j];
            return count;
        }

        size_t rank_l(size_t pos) __attribute__((const)) {
            auto n = (pos + 1) / 256; 
            auto m = (pos + 1) % 256;
            auto count = 0;
            for (auto i = 0; i < n; i++)
                count += labels[i].count();
            for(auto j = 0; j < m; j++)
                count += labels[n][j];
            return count;
        }

        size_t rank_p(size_t pos) __attribute__((const)) {
            return std::count(std::begin(is_prefix_key), std::begin(is_prefix_key)+pos+1, true);
        }


        size_t select_c(size_t count) __attribute__ ((const)){
            auto curr = 0, pos = 0;
            while (curr + has_child[pos/256].count() < count && pos < size(has_child)*256){
                curr += has_child[pos/256].count();
                pos += 256;
            }
            while (curr < count && pos < size(has_child)*256){
                curr += has_child[pos/256][pos%256];
                pos += 1;
            }
            return pos;
        }  

        size_t child(size_t pos){
            return 256 * rank_c(pos);
        }

        size_t parent(size_t pos){
            return select_c(pos/256);
        }

        size_t value(size_t pos){
            return rank_l(pos) - rank_c(pos) - 1; // + rank_p(pos/256)
        }



        auto look_up(const std::string& word, size_t pos = -1, size_t from_level = 0) { 
            struct Query{ bool found; size_t node;};
            auto to_level = from_level + n_levels;
            for (auto level = 0; level < to_level; level++){
                if (level == size(word)){
                    return Query{
                        .found = labels[child(pos)/256]['\0'],
                        .node = failed_query
                    };
                }
                pos = child(pos) + word[level];
                if (labels[pos/256][word[level]]) {
                    if (!has_child[pos/256][word[level]]){
                        auto suffix = values[value(pos)];
                        return Query {
                            .found = strcmp(word.c_str() + level+1, suffix.c_str()) == 0,
                            .node = failed_query
                        }; // chech if the suffix is the same
                    }
                } else
                    return Query{.found = false, .node = failed_query};
            }
            return Query{
                .found = false,
                .node = (child(pos) - 256*size(labels))/256
            };
        }

    };

    struct State {
        vector<size_t> positions;
        size_t level;

        decltype(auto) operator[] (auto i){
            return positions[i];
        }
    };


    struct LoudsSparse {
        public:
        struct Iter {
            Iter(LoudsSparse *louds, shared_ptr<Positions> positions, size_t node = 0): 
                from_level{louds->from_level},
                to_level{louds->to_level},
                n_roots{louds->n_trailing_children},
                node{node},
                pos{0},
                level{from_level}, 
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

            Iter& lb(string_view key){
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

            Iter& ub(string_view& key){
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
            shared_ptr<Positions> idxs;
            LoudsSparse *louds;
        };


        Iter begin(size_t node=0) {
            return Iter{this, make_shared<Positions>(this->to_level), node}.begin();
        }
        Iter begin(shared_ptr<Positions> pos, size_t node=0) {
            return Iter{this, pos, node}.begin();
        }

        Iter end(size_t node=0) {
            return Iter{this, make_shared<Positions>(this->to_level), node}.end();
        }

        Iter end(shared_ptr<Positions> pos, size_t node=0) {
            return Iter{this, pos, node}.end();
        }
        
        Iter lb(string_view key, size_t node=0) {
            return Iter{this, make_shared<Positions>(this->to_level), node}.lb(key);
        }

        Iter lb(string_view key, shared_ptr<Positions> pos, size_t node=0) {
            return Iter{this, pos, node}.lb(key);
        }

        Iter ub(string_view key, shared_ptr<Positions> pos, size_t node=0) {
            return Iter{this, pos, node}.ub(key);
        }

        Iter ub(string_view key, size_t node=0) {
            return Iter{this, make_shared<Positions>(this->to_level), node}.ub(key);
        }

        vector<char> labels;
        // vector<bool> has_child;
        // vector<bool> louds;
        BitVector<64> has_child;
        BitVector<64> louds;
        
        vector<string> values;
        size_t n_levels{};
        size_t from_level{};
        size_t to_level{};
        size_t n_trailing_children{};
        size_t max_size_key{};

        static LoudsSparse from_builder(LoudsBuilder &builder, size_t from_level = 0, size_t to_level = -1, size_t n_trailing_children = 0){
            if (to_level == -1) to_level = builder.n_levels;

            auto builder_labels = span(std::begin(builder.labels)+from_level, std::begin(builder.labels)+to_level);
            auto builder_has_child = span(std::begin(builder.has_child)+from_level, std::begin(builder.has_child)+to_level);
            auto builder_louds = span(std::begin(builder.louds)+from_level, std::begin(builder.louds)+to_level);
            auto builder_suffix = span(std::begin(builder.suffix)+from_level, std::begin(builder.suffix)+to_level);
            
            auto sum_size = 0;
            for (const auto &v: builder_labels) sum_size += size(v);
            auto sum_size_values = 0;
            for (const auto &v: builder_suffix) sum_size_values += size(v);

            return LoudsSparse {
                .labels = merge(builder_labels, sum_size),
                .has_child = BitVector<64>(builder_has_child, sum_size),
                .louds = BitVector<64>(builder_louds, sum_size),
                .values = merge(builder_suffix, sum_size_values),
                .n_levels = to_level - from_level,
                .from_level = from_level,
                .to_level = to_level,
                .n_trailing_children = n_trailing_children,
                .max_size_key = builder.max_size_key
            };
        }
        template<class T>
        static vector<T> merge(span<vector<T>> xs, size_t capacity){
            vector<T> ans;
            ans.reserve(capacity);
            for (const auto &x: xs) ans.insert(std::end(ans), std::begin(x), std::end(x)); 
            return ans;
        }

        size_t rank_c(size_t pos) __attribute__((const)){
            // return std::count(std::begin(has_child), std::begin(has_child)+pos+1, true);
            return has_child.rank(pos);
        }
        size_t rank_l(size_t pos) __attribute__((const)){
            // return std::count(std::begin(louds), std::begin(louds)+pos+1, true);
            return louds.rank(pos);
        }

        size_t select_c(size_t count) __attribute__((const)){
            // for (auto i = 0, curr = 0; i < size(has_child); i++){
            //     curr += has_child[i];
            //     if (curr >= count) return i;
            // }
            // return size(has_child);
            return has_child.select(count);
        }

        size_t select_l(size_t count) __attribute__((const)){
            // for (auto i = 0, curr = 0; i < size(louds); i++){
            //     curr += louds[i];
            //     if (curr >= count) return i;
            // }
            // return size(louds);
            return louds.select(count);
        }

        size_t find(size_t node, char c){
            for (auto i = select_l(node+1); i < select_l(node+2); i++)
                if (labels[i] == c) return i;
            return -1;
        }

        size_t find(size_t begin, size_t end, char c){
            for (auto i = begin; i < end; i++)
                if (labels[i] == c) return i;
            return -1;
        }

        size_t child_begin(size_t pos){
            return select_l(rank_c(pos) + 1 + n_trailing_children);
        }

        size_t child(size_t pos){
            return select_l(rank_c(pos) + 1 + n_trailing_children);
        }

        size_t child_end(size_t pos){
            return select_l(rank_c(pos) + 2 + n_trailing_children);
        }

        size_t parent(size_t pos){
            return select_c(rank_l(pos) - 1 + n_trailing_children);
        }

        size_t value(size_t pos){
            return pos - rank_c(pos);
        }

        auto look_up(const std::string& word, size_t node = 0, size_t from_level = 0) {
            struct Query{ bool found; size_t node;}; 
            auto to_level = from_level+n_levels;
            auto pos_begin = select_l(node+1);
            auto pos_end = select_l(node+2);
            for (auto level = from_level; level < to_level; level++){
                if (level == size(word)){
                    return Query {
                        .found = find(pos_begin, pos_end, '\0') != -1,
                        .node = failed_query
                    };
                }
                auto pos = find(pos_begin, pos_end, word[level]);
                if (pos == -1) return Query { .found = false, .node = failed_query }; // failed to find char
                else if (!has_child[pos]){
                    auto suffix = values[value(pos)];
                    return Query {
                        .found = strcmp(word.c_str() + level+1, suffix.c_str()) == 0,
                        .node = failed_query
                    }; // chech if the suffix is the same
                }
                auto node = rank_c(pos); // get child node
                pos_begin = select_l(node+1 + n_trailing_children); // get start pos node
                pos_end = select_l(node+2 + n_trailing_children); // get end pos node
            }
            return Query {
                .found = false,
                .node = node - rank_l(size(louds)-1) + 1
            };
        }
    };

    struct Surf {
        public:
        shared_ptr<LoudsDense> ld;
        shared_ptr<LoudsSparse> ls;
        size_t n_dense_levels;
        size_t n_nodes_dense_levels;
        static Surf from_builder(LoudsBuilder &builder, size_t n_dense_levels){
            auto n_nodes_level = [&](auto level) {
                return count(begin(builder.louds[level]), end(builder.louds[level]), true);
            };
            auto n_nodes_dense_levels = n_nodes_level(n_dense_levels);
            return Surf{
                make_shared<LoudsDense>(LoudsDense::from_builder(builder,0, n_dense_levels)), 
                make_shared<LoudsSparse>(LoudsSparse::from_builder(builder, n_dense_levels, -1, n_nodes_dense_levels - 1)),
                n_dense_levels,
                (size_t) n_nodes_dense_levels     
            };
        }

        bool look_up(const std::string& word) {
            auto [found_d, node_s] = ld->look_up(word);
            if (found_d) return true;
            if (node_s == failed_query) return false;
            auto [found_l, pos_l] = ls->look_up(word, node_s, n_dense_levels);
            return found_l;
        }

        bool range_query(string_view lb, string_view ub){
            auto pos_begin = make_shared<Positions>(ld->n_levels + ls->n_levels);
            auto pos_end = make_shared<Positions>(ld->n_levels + ls->n_levels);
            auto it_ld = ld->lb(lb, pos_begin);
            auto end_ld = ld->ub(ub, pos_end);
            auto node_end_ls = ld->rank_c(end_ld.pos) - size(ld->labels);
            auto end_ls = ls->ub(ub, pos_end, node_end_ls);
            while(true){
                auto [prefix_ld, suffix_ld] = *it_ld;
                if (it_ld.has_child){
                    auto node_ls = ld->rank_c(it_ld.pos) - size(ld->labels);
                    auto it_ls = ls->lb(lb,pos_begin,node_ls);
                    auto end_ls = ls->ub(ub,node_ls);
                    for (;it_ls != end_ls; ++it_ls){
                        auto [prefix_ls, suffix_ls] = *it_ls;
                        cout << prefix_ld << prefix_ls << suffix_ls << endl;
                    }
                }else{
                    cout << prefix_ld << suffix_ld << endl;
                }
                if (it_ld != end_ld){
                    ++it_ld;
                }
                else break;
            }
            return true;
        }

    };
   
};