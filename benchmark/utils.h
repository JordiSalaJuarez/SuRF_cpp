#pragma once
#include <limits>
#define FMT_HEADER_ONLY

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>
#include <bit>
#include <span>
#include <filesystem>
#include <fmt/core.h>
#include <ranges>
#include <bit>
#include <benchmark/benchmark.h>

namespace utils{
    using namespace std;
    using fmt::format;
    using std::filesystem::exists;
    using std::filesystem::current_path;

    auto get_input_data(auto n_keys) -> std::vector<std::string>{
        auto keys = [] (size_t n_keys) {
            std::ifstream file("data/keys_100M");
            std::string key;
            std::vector<std::string> keys;
            while(std::getline(file, key) && 0 < n_keys){
                keys.push_back(key);
                --n_keys;
            }
            file.close();
            return keys;
        }(n_keys);
        return keys;
    }

    auto to_string(size_t key) -> std::string;

    enum Type: int64_t { String, Int };
    enum Kind: int64_t { Insert, Query };
    enum Distribution: int64_t { Zipfian, Uniform, Words };

    auto to_string(Distribution dist) -> char const *;

    auto to_string(Type type) -> char const *;

    auto to_string(Kind kind) -> char const *;

    static auto NSTRINGS = 25'000'000UL;
    static auto NINTS = 50'000'000UL;


    enum Dataset {
        DNA,
        GEO,
        WIKI,
        ZIPFIAN,
    };

    enum Pattern{ SCAN, RANDOM } ;

    auto path(Dataset kind) -> string;

    auto get_keys(Dataset kind, size_t size) -> vector<string>;

    auto get_keys(Dataset kind) -> vector<string>;

    auto split(vector<string> &) -> tuple<span<string>, span<string>>;

    auto shuffle(span<string>, size_t = {}) -> span<string>;

    auto sorted(span<string> keys) -> span<string>;
    
    auto sort(vector<string>& keys) -> void;

    auto get_data(std::size_t size, Kind kind, Type type, Distribution dist)
        -> std::vector<std::string>;

    auto get_input_data(auto n_train, auto n_test) -> auto {
        auto keys = get_input_data(n_train + n_test);
        std::shuffle(std::begin(keys), std::end(keys), std::mt19937(std::random_device()()));
        auto keys_train = std::vector<std::string>(std::begin(keys), std::begin(keys) + n_train);
        auto keys_test = std::vector<std::string>(std::begin(keys)+ n_train, std::begin(keys) + (n_train + n_test));
        return std::tuple{keys_train, keys_test};
    }
    auto reduce(auto range, auto bin_func, auto acc){
        for(auto && x : range){
            acc += bin_func(acc, x);
        }
        return acc;
    }
    auto range_key(std::string key, size_t size, string & from, string & to) -> void;

    struct Stats { 
        size_t ds_size_bytes{};
        span<string> keys_insert{};
        span<string> keys_query{};
        size_t p{};
        size_t tp{};
        size_t bits{};
    };
    auto fill_counters(benchmark::State &state, Stats stats) -> void;
}


