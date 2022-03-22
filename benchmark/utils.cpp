#include<iostream>
#include <vector>
#include <benchmark/benchmark.h>
#include <fstream>
#include <string>
#include "../surf_paper/include/rank.hpp"
#include <bitset>
#include <algorithm>
#include <bit>
#include "../surf/utils.h"

enum Kind { Fragmented, NotFragmented };

std::vector<std::string> get_input_data(size_t n_keys, Kind kind){
    std::ifstream file;
    if (kind == Kind::Fragmented){
        file = std::ifstream("data/keys_100M");
    }else{
        file = std::ifstream("data/keys_100M");
    }
    std::string key;
    std::vector<std::string> keys;
    while(std::getline(file, key) && 0 < n_keys){
        keys.push_back(key);
        --n_keys;
    }
    return std::move(keys);
}


using namespace std;

template <size_t B>
class Block{
    public:
    enum { n_chars = B / __CHAR_BIT__, n_bytes = B / sizeof(char), n_bits_char = __CHAR_BIT__};
    char values[n_chars]{};
    bool operator[](size_t pos){
        return (values[pos/n_bits_char] >> pos%n_bits_char) & 0x1;
    }
    void set(size_t pos){
        values[pos/n_bits_char] |= (1 << pos%n_bits_char);
    }
    void clear(size_t pos){
        values[pos/n_bits_char] &= ~(1 << pos%n_bits_char);
    }
};


class U64 {
    public:
    enum { mask = 0x8000000000000000, bit_size = 64};
    inline bool readBit (size_t pos) const noexcept {
        assert(pos < 64);
        return v & (mask >> pos);
    }
    inline void setBit(size_t pos) noexcept{
        v |= (mask >> pos);
    }
    inline void clearBit(size_t pos) noexcept{
        v &= ~(mask >> pos);
    }
    private:
    uint64_t v;
};

template<class B>
class BitVector{
    public:
    enum { n_bits_block = sizeof(B)*__CHAR_BIT__};
    B *v;
    size_t n;
    explicit BitVector(size_t n): v{new B[n]}, n{n} {};
    ~BitVector(){
        delete[] v;
    }
    bool readBit(size_t pos) const noexcept{
        return v[pos / B::bit_size].readBit(pos & (B::bit_size - 1));
    }
    void setBit(size_t pos) noexcept{
        v[pos / B::bit_size].setBit(pos & (B::bit_size - 1));
    }
    void clearBit(size_t pos) noexcept{
        v[pos / B::bit_size].clearBit(pos & (B::bit_size - 1));
    }
};


static void BM_SetBitsStd(benchmark::State& state) {
    vector<bool> bit_vector_std(state.range());
    for (auto _ : state){ 
        state.PauseTiming();
        auto i = rand() % state.range();
        state.ResumeTiming();
        auto v = true;
        bit_vector_std[i] = v;
        benchmark::DoNotOptimize(bit_vector_std);
    }
}

static void BM_ReadBitsStd(benchmark::State& state) {
    vector<bool> bit_vector_std(state.range());
    for (auto _ : state){ 
        state.PauseTiming();
        auto i = rand() % state.range();
        bit_vector_std[i] = true;
        state.ResumeTiming();
        auto v = bit_vector_std[i];
        benchmark::DoNotOptimize(v);
    }
}


static const auto B = 512;

static void BM_ReadBitsNoStd(benchmark::State& state) {
    BitVector<U64> bit_vector(state.range()/U64::bit_size);
    for (auto _ : state){ 
        state.PauseTiming();
        auto i = rand() % state.range();
        bit_vector.setBit(i);
        state.ResumeTiming();
        auto b = bit_vector.readBit(i);
        benchmark::DoNotOptimize(b);
    }
}

static void BM_SetBitsNoStd(benchmark::State& state) {
    BitVector<U64> bit_vector(state.range()/U64::bit_size);
    for (auto _ : state){ 
        state.PauseTiming();
        auto i = rand() % state.range();
        state.ResumeTiming();
        bit_vector.setBit(i);
        benchmark::DoNotOptimize(bit_vector);
    }
}


static void BM_RankPaper(benchmark::State& state) {
    vector<uint64_t> v(state.range()/U64::bit_size, 0);
    for (auto i = 0; i < state.range()/U64::bit_size; ++i)
        v[i] = rand();
    surf::BitvectorRank bit_vector(U64::bit_size, {v}, {size(v)*U64::bit_size});
    for (auto _ : state){ 
        state.PauseTiming();
        auto i = rand() % state.range();
        state.ResumeTiming();
        auto b = bit_vector.rank(i);
        benchmark::DoNotOptimize(b);
    }
}

static void BM_Rank(benchmark::State& state) {
    vector<std::bitset<256>> bit_vector(state.range()/256);
    auto rank_c = [&](size_t pos)  {
        auto n = (pos + 1) / 256; 
        auto m = (pos + 1) & 255;
        auto count = 0;
        for (auto i = 0; i < n; i++)
            count += bit_vector[i].count();
        count += (bit_vector[n] >> (255 - m)).count();
        return count;
    };
    for (auto i = 0; i < state.range()/256; ++i)
        bit_vector[i] = rand();
    for (auto _ : state){ 
        state.PauseTiming();
        auto i = rand() % state.range();
        state.ResumeTiming();
        auto b = rank_c(i);
        benchmark::DoNotOptimize(b);
    }
}

static void BM_RankLUT(benchmark::State& state) {
    vector<std::bitset<256>> bit_vector(state.range()/256);
    vector<size_t> lut(state.range()/256);
    auto rank_c = [&](size_t pos)  {
        return lut[pos / 256] + (bit_vector[pos / 256] >> (255 - (pos & 255))).count();
    };
    for (auto i = 0; i < state.range()/256; ++i)
        bit_vector[i] = rand();
    for (auto i = 0; i < state.range()/256; ++i)
        lut[i] = i == 0 ? 0: (lut[i-1] + bit_vector[i].count());
    for (auto _ : state){ 
        state.PauseTiming();
        auto i = rand() % state.range();
        state.ResumeTiming();
        auto b = rank_c(i);
        benchmark::DoNotOptimize(b);
    }
}

static void BM_RankBool(benchmark::State& state) {
    vector<bool> bit_vector(state.range());
    auto rank_c = [&](size_t pos)  {
        return count(bit_vector.begin(), bit_vector.begin() + pos + 1, true);
    };
    for (auto i = 0; i < state.range(); ++i)
        bit_vector[i] = (bool)rand()%2;
    for (auto _ : state){ 
        state.PauseTiming();
        auto i = rand() % state.range();
        state.ResumeTiming();
        auto b = rank_c(i);
        benchmark::DoNotOptimize(b);
    }
}


static void BM_ReadVector(benchmark::State& state){
    std::ifstream file;
    std::vector<std::string> keys = get_input_data(state.range(1), Kind(state.range(0)));
    for (auto _ : state){
        state.PauseTiming();
        std::vector<std::string> keys2; 
        state.ResumeTiming();
        auto n = state.range(1);
        for (auto i = 0; i < n; ++i){
            auto key = keys[i];
            keys2.push_back(key);
            benchmark::DoNotOptimize(key);
        }
        benchmark::DoNotOptimize(keys2);
    }
    state.SetBytesProcessed(state.range(1) * size(keys[0]) * state.iterations());

}


static void BM_ReadFile(benchmark::State& state){
    std::ifstream file;
    std::string key;
    for (auto _ : state){
        state.PauseTiming();
        std::vector<std::string> keys;
        if (state.range(0) == Kind::Fragmented){
            file = std::ifstream("data/keys_100M");
        }else{
            file = std::ifstream("data/keys_100M");
        }
        auto n = state.range(1);
        state.ResumeTiming();
        while(std::getline(file, key) && n-- > 0) {
            keys.push_back(key);
        }
        benchmark::DoNotOptimize(keys);
    }
    state.SetBytesProcessed(state.range(1) * size(key) * state.iterations());
} 

// BENCHMARK(BM_ReadVector)
//     ->ArgPair(Kind::Fragmented, 1000)
//     ->ArgPair(Kind::Fragmented, 10000)
//     ->ArgPair(Kind::Fragmented, 100000)
//     ->ArgPair(Kind::Fragmented, 1000000);
// BENCHMARK(BM_ReadFile)
//     ->ArgPair(Kind::Fragmented, 1000)
//     ->ArgPair(Kind::Fragmented, 10000)
//     ->ArgPair(Kind::Fragmented, 100000)
//     ->ArgPair(Kind::Fragmented, 1000000);
// BENCHMARK(BM_ReadVector)
//     ->ArgPair(Kind::NotFragmented, 1000)
//     ->ArgPair(Kind::NotFragmented, 10000)
//     ->ArgPair(Kind::NotFragmented, 100000)
//     ->ArgPair(Kind::NotFragmented, 1000000);
// BENCHMARK(BM_ReadFile)
//     ->ArgPair(Kind::NotFragmented, 1000)
//     ->ArgPair(Kind::NotFragmented, 10000)
//     ->ArgPair(Kind::NotFragmented, 100000)
//     ->ArgPair(Kind::NotFragmented, 1000000);
// BENCHMARK(BM_ReadVector)->Arg(Kind::NotFragmented);
// BENCHMARK(BM_ReadFile)->Arg(Kind::NotFragmented);

// BENCHMARK(BM_SetBitsNoStd)->Arg(U64::bit_size*20);
// BENCHMARK(BM_SetBitsStd)->Arg(U64::bit_size*20);
// BENCHMARK(BM_ReadBitsNoStd)->Arg(U64::bit_size*20);
// BENCHMARK(BM_ReadBitsStd)->Arg(U64::bit_size*20);

BENCHMARK(BM_RankPaper)->Arg(256*1000);
BENCHMARK(BM_Rank)->Arg(256*1000);
BENCHMARK(BM_RankBool)->Arg(256*1000);
BENCHMARK(BM_RankLUT)->Arg(256*1000);




BENCHMARK_MAIN();
