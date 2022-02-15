#include<vector>
#include<bitset>
// #include<iterator>
#include<algorithm>
#include<span>
#include<string.h>
using namespace std;

inline constexpr std::size_t failed_query = std::numeric_limits<std::size_t>::max();

struct LoudsBuilder {
    public:
    vector<vector<char>> labels{};
    vector<vector<bool>> has_child{};
    vector<vector<bool>> louds{};
    vector<vector<bool>> is_prefix{};
    vector<vector<string>> suffix{};
    size_t n_levels{};

    static LoudsBuilder from_stream(istream &input){
        LoudsBuilder result{};
        auto &labels = result.labels;
        auto &has_child = result.has_child;
        auto &louds = result.louds;
        auto &n_levels = result.n_levels;
        auto &suffix = result.suffix;
        auto &is_prefix = result.is_prefix;


        auto insert_key = [&](const string& key, size_t start, size_t end){
            if (end+1 > n_levels){
                n_levels = end+1;
                resize<char>(labels, n_levels);
				resize<bool>(has_child, n_levels);
				resize<bool>(louds, n_levels);
                resize<bool>(is_prefix, n_levels);
                resize<string>(suffix, n_levels);
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
                labels[end].emplace_back('$');
                has_child[end].emplace_back(false);
                is_prefix[end].emplace_back(true);
                louds[end].emplace_back(start != end);
                suffix[end].emplace_back("");
            }
        };


        auto add_dollar = [&](const auto& i){
            if (i+2 > n_levels){
                n_levels = i+2;
                resize<char>(labels, n_levels);
				resize<bool>(has_child, n_levels);
				resize<bool>(louds, n_levels);
            }
            has_child[i][size(has_child[i])] = true;
            labels[i+1].emplace_back('$');
            louds[i+1].emplace_back(true);
            has_child[i+1].emplace_back(false);
        };


        auto lowest_common_ancestor_prev = [&](const auto& x){
            auto i = 0;
            while(i < size(x) && i < n_levels && x[i] == labels[i].back() && has_child[i].back()) i++; // treat antecesors
            if(i < size(x) && i < n_levels && x[i] == labels[i].back()){ // treat leaf
                add_dollar(i);
                i++; 
            }
            return i;
        };
        auto lowest_common_ancestor_next = [&](const auto& x, const auto& y){
            auto i = 0;
            while(i < size(x) && i < size(y) && x[i] == y[i]) i++;
            return i;
        };

        string key{}, next_key{};
        if (!getline(input, key)) return result;
        while(getline(input, next_key)){
            auto lca_prev = lowest_common_ancestor_prev(key);
            auto lca_next = lowest_common_ancestor_next(key, next_key);
            insert_key(key, lca_prev, max(lca_prev, lca_next));
            key = next_key; // is this going to be a copy? move?
		}
        auto lca_prev = lowest_common_ancestor_prev(key);
        insert_key(key, lca_prev, lca_prev);
        louds[0][0] = true;

        return result;
    }
    private:
    template <class T>
    static void resize(std::vector<std::vector<T>> &vs, size_t n) {
        auto diff = n - size(vs);
        for(auto i=0; i<diff; i++) 
            vs.emplace_back();
    }
    static size_t lowest_common_ancestor(std::vector<std::vector<char>>& labels, std::string word) {
        auto i = 0;
        while (!empty(labels[i]) && i < size(word) && word[i] == labels[i].back()) i++;
        return i;
    }
};


struct LoudsDense {
    vector<std::bitset<256>> labels{};
    vector<std::bitset<256>> has_child{};
    vector<bool> is_prefix_key{};
    vector<string> values{};
    size_t n_levels{};
    size_t n_trailing_children{};

    static void merge_and_insert(const auto & vs, auto &r, size_t capacity){
        r.reserve(capacity);
        for (const auto &v: vs) r.insert(end(r), begin(v), end(v)); 
    }

    static LoudsDense from_builder(const LoudsBuilder &builder, size_t from_level = 0, size_t to_level = -1, size_t n_trailing_children=0){
        
        auto nested_size = [](auto vs){
            auto sum = 0;
            for (const auto& v: vs) sum += size(v);
            return sum;
        };
        
        if (to_level == -1) to_level = builder.n_levels;
        LoudsDense result{};
        result.n_levels = to_level - from_level;
        result.n_trailing_children = n_trailing_children;
        auto builder_louds = span(begin(builder.louds)+from_level, begin(builder.louds) + to_level);
        auto builder_suffix = span(begin(builder.suffix)+from_level, begin(builder.suffix) + to_level);
        auto sum_size = 0;
        for (const auto &v: builder_louds) sum_size += std::count(begin(v), end(v), true);
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
        return std::count(begin(is_prefix_key), begin(is_prefix_key)+pos+1, true);
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
                    .found = labels[child(pos)/256]['$'],
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

struct LoudsSparse {

    vector<char> labels;
    vector<bool> has_child;
    vector<bool> louds;
    vector<string> values;
    size_t n_levels{};
    size_t n_trailing_children{};

    // decltype(this)
    static LoudsSparse from_builder(const LoudsBuilder &builder, size_t from_level = 0, size_t to_level = -1, size_t n_trailing_children = 0){
        if (to_level == -1) to_level = builder.n_levels;
        LoudsSparse result{};
        result.n_levels = to_level - from_level;
        result.n_trailing_children = n_trailing_children;
        auto builder_labels = span(begin(builder.labels)+from_level, begin(builder.labels)+to_level);
        auto builder_has_child = span(begin(builder.has_child)+from_level, begin(builder.has_child)+to_level);
        auto builder_louds = span(begin(builder.louds)+from_level, begin(builder.louds)+to_level);
        auto builder_suffix = span(begin(builder.suffix)+from_level, begin(builder.suffix)+to_level);
        
        auto sum_size = 0;
        for (const auto &v: builder_labels) sum_size += size(v);
        
        merge_and_insert(builder_labels, result.labels, sum_size);
        merge_and_insert(builder_has_child, result.has_child, sum_size);
        merge_and_insert(builder_louds, result.louds, sum_size);

        auto sum_size_values = 0;
        for (const auto &v: builder_suffix) sum_size_values += size(v);
        merge_and_insert(builder_suffix, result.values, sum_size_values);
        return result;
    }

    static void merge_and_insert(const auto & vs, auto &r, size_t capacity){
        r.reserve(capacity);
        for (const auto &v: vs) r.insert(end(r), begin(v), end(v)); 
    }

    size_t rank_c(size_t pos) __attribute__((const)) {
        return std::count(begin(has_child), begin(has_child)+pos+1, true);
	}
    size_t rank_l(size_t pos) __attribute__((const)) {
        return std::count(begin(louds), begin(louds)+pos+1, true);
	}

    size_t select_c(size_t count) __attribute__((const)){
		for (auto i = 0, curr = 0; i < size(has_child); i++){
			curr += has_child[i];
			if (curr >= count) return i;
		}
		return size(has_child);
	}

	size_t select_l(size_t count) __attribute__((const)){
		for (auto i = 0, curr = 0; i < size(louds); i++){
			curr += louds[i];
			if (curr >= count) return i;
		}
		return size(louds);
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
        return select_c(rank_l(pos) - 1);
    }

    size_t value(size_t pos){
        return pos - rank_c(pos);
    }

    // auto look_up(const std::string& word, size_t node = 0, size_t from_level = 0) {
    //     struct Query{ bool found; size_t pos;}; 
    //     auto to_level = from_level+n_levels;
    //     for (auto level = from_level; level < to_level; level++){
    //         if (level == size(word)){
    //             auto pos = find(node, '$');
    //             return Query {
    //                 .found = pos != -1,
    //                 .pos = 0
    //             };
    //         }else{
    //             auto pos = find(node, word[level]);
    //             if (pos == -1) return Query { .found = false, .pos = failed_query }; // failed to find char
    //             else if (!has_child[pos]){
    //                 auto suffix = values[value(pos)];
    //                 auto next_node = rank_c(pos);
    //                 return Query {
    //                     .found = strcmp(word.c_str() + level+1, suffix.c_str()) == 0,
    //                     .pos = next_node
    //                 }; // chech if the suffix is the same
    //             }
    //             node = rank_c(pos);
    //         }
    //     }
	// 	return Query {
    //         .found = false,
    //         .pos = 0
    //     };
	// }


	auto look_up(const std::string& word, size_t node = 0, size_t from_level = 0) {
        struct Query{ bool found; size_t node;}; 
        auto to_level = from_level+n_levels;
        auto pos_begin = select_l(node+1);
        auto pos_end = select_l(node+2);
        for (auto level = from_level; level < to_level; level++){
            if (level == size(word)){
                return Query {
                    .found = find(pos_begin, pos_end, '$') != -1,
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
    unique_ptr<LoudsDense> ld;
    unique_ptr<LoudsSparse> ls;
    static Surf from_builder(const LoudsBuilder &builder, size_t n_dense_levels){
        auto n_nodes_level = [&](auto level) {
            return count(begin(builder.louds[level]), end(builder.louds[level]), true);
        };
        return Surf{
            make_unique<LoudsDense>(LoudsDense::from_builder(builder,0, n_dense_levels)), 
            make_unique<LoudsSparse>(LoudsSparse::from_builder(builder, n_dense_levels, -1, n_nodes_level(n_dense_levels) - 1))     
        };
    }

    bool look_up(const std::string& word) {
        auto [found_d, node_s] = ld->look_up(word);
        if (found_d) return true;
        if (node_s == failed_query) return false;
        auto [found_l, pos_l] = ls->look_up(word, node_s, ld->n_levels);
        return found_l;
	}

};