#include "utils.h"
#include <algorithm>
#include <span>
#include <numeric>
#include <execution>

namespace utils {
    
    auto get_keys(Dataset kind, size_t size) -> vector<string> {
        if (!exists(path(kind))) {
            throw std::invalid_argument(format("Failed to find file: {} from {}",
                                            path(kind), current_path().string()));
        }
        ifstream input(path(kind));
        vector<string> keys;
        string key;
        while (input >> key && size-- > 0) {
            keys.push_back(key);
        }
        return keys;
    }

    auto sorted(span<string> keys) -> span<string>{
        sort(execution::par,keys.begin(), keys.end());
        return {keys.begin(), unique(keys.begin(), keys.end())};
    }
    auto sort(vector<string>& keys) -> void {
        sort(execution::par,keys.begin(), keys.end());
    }

    auto path(Dataset kind) -> string {
      switch (kind) {
      case Dataset::DNA:
        return "../data/dna.txt";
      case Dataset::GEO:
        return "../data/geo_names.txt";
      case Dataset::WIKI:
        return "../data/wiki.txt";
      case Dataset::ZIPFIAN:
        return "../data/insert_zipfian";
      default:
        throw invalid_argument(format("Wrong Enum for Dataset {}", kind));
      }
    }

    auto split(vector<string> & keys) -> tuple<span<string>, span<string>>{
        auto *begin = keys.data();
        auto *end = keys.data() + keys.size();
        return {{begin, midpoint(begin, end)}, {begin, end}};
    }

    auto to_string(Type type) -> char const * {
      switch (type) {
      case Type::Int:
        return "int";
      case Type::String:
        return "string";
      default:
        throw std::invalid_argument("Not implemented");
      }
    }

    auto to_string(Kind kind) -> char const * {
      switch (kind) {
      case Kind::Insert:
        return "insert";
      case Kind::Query:
        return "query";
      default:
        throw std::invalid_argument("Not implemented");
      }
    }

    auto to_string(Distribution dist) -> char const * {
      switch (dist) {
      case Distribution::Zipfian:
        return "zipfian";
      case Distribution::Uniform:
        return "uniform";
      case Distribution::Words:
        return "words";
      default:
        throw std::invalid_argument("Not implemented");
      }
    }
    auto get_data(std::size_t size, Kind kind, Type type, Distribution dist)
        -> std::vector<std::string> {
      using std::filesystem::current_path;
      using std::filesystem::exists;
      using std::ranges::views::take;
      auto input_path =
          fmt::format("data/{}_{}", to_string(kind), to_string(dist));
      if (!exists(input_path)) {
        throw std::invalid_argument("Failed to find file: " + input_path +
                                    "\n from " + current_path().string());
      }
      std::vector<std::string> keys;
      std::ifstream input(input_path);
      if (type == Type::String) {
        std::string key;
        while (input >> key && size-- > 0) {
          keys.push_back(key);
        }
        if (kind == Kind::Insert) {
          std::sort(keys.begin(), keys.end());
          keys.erase(std::unique(keys.begin(), keys.end()), keys.end());
        }
      } else {
        size_t key;
        while (input >> key && size-- > 0) {
          auto string_key = to_string(key);
          if (!string_key.empty())
            keys.push_back(string_key);
        }
        if (kind == Kind::Insert) {
          std::sort(keys.begin(), keys.end());
          keys.erase(std::unique(keys.begin(), keys.end()), keys.end());
        }
      }
      return keys;
    };
    auto to_string(size_t key) -> std::string {
      size_t endian_swapped_key = __builtin_bswap64(key);
      std::string ans = {reinterpret_cast<const char *>(&endian_swapped_key),
                         sizeof(size_t)};
      return {ans.c_str()};
    }
    auto shuffle(span<string> keys, size_t seed) -> span<string> {
        shuffle (keys.begin(), keys.end(), std::default_random_engine(seed));
        return keys;
    }
    auto get_keys(Dataset kind) -> vector<string> {
      return get_keys(kind, std::numeric_limits<std::size_t>::max());
    }
    auto range_key(std::string key, size_t size, string & from, string & to) -> void {
      // uint64_t int_data, int_from, int_to;
      // memcpy(&int_data, key.data(), sizeof(uint64_t));
      // int_from = std::max(int_data - size/2, std::numeric_limits<uint64_t>::min());
      // int_to = std::min(int_data + size/2, std::numeric_limits<uint64_t>::max());
      // from = std::string(reinterpret_cast<char const *>(&int_from), sizeof(uint64_t));
      // to = std::string(reinterpret_cast<char const *>(&int_from), sizeof(uint64_t));
      from = key;
      to = key;
      for (size_t i = key.size()-size; i < key.size(); ++i){
        from[i] = '\1';
        to[i] = '\255';
      }
    }
    auto fill_counters(benchmark::State &state, Stats stats) -> void {
      using namespace benchmark;
      size_t fp = stats.p - stats.tp;
      size_t tn = stats.keys_query.size() - stats.p;
      state.counters["Size(B)"] =
          Counter(stats.ds_size_bytes, Counter::kDefaults, Counter::kIs1024);
      state.counters["#Keys"] = Counter(stats.keys_insert.size(),
                                        Counter::kDefaults, Counter::kIs1000);
      state.counters["Bits"] = stats.bits; 
      state.counters["InputSize(B)"] =
          Counter(reduce(
                      stats.keys_insert,
                      [](auto acc, auto key) { return acc + key.size(); }, 0UL),
                  Counter::kDefaults, Counter::kIs1024);
      state.counters["FPR"] = fp / (tn + fp + 0.0);
      state.counters["Ops/s"] = Counter(stats.keys_query.size(),
                  benchmark::Counter::kIsIterationInvariantRate);
      state.counters["Bytes/s"] = Counter(
          reduce(
              stats.keys_query,
              [](auto acc, auto key) { return acc + key.size(); }, 0UL),
          Counter::kIsIterationInvariantRate, benchmark::Counter::kIs1024);
    }
    } // namespace utils
