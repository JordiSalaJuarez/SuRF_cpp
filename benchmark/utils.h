#pragma once
#include <fstream>
#include <random>
#include <string>
#include <tuple>
#include <vector>

std::vector<std::string> get_input_data(auto n_keys){
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

auto get_input_data(auto n_train, auto n_test) -> auto {
    auto keys = get_input_data(n_train + n_test);
    std::shuffle(std::begin(keys), std::end(keys), std::mt19937(std::random_device()()));
    auto keys_train = std::vector<std::string>(std::begin(keys), std::begin(keys) + n_train);
    auto keys_test = std::vector<std::string>(std::begin(keys)+ n_train, std::begin(keys) + (n_train + n_test));
    return std::tuple{keys_train, keys_test};
}