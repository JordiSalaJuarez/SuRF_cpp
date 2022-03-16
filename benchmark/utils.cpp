#include<iostream>
#include <vector>
#include <benchmark/benchmark.h>
#include <fstream>
#include <string>




enum Kind { Fragmented, NotFragmented };

std::vector<std::string> get_input_data(size_t n_keys, Kind kind){
    std::ifstream file;
    if (kind == Kind::Fragmented){
        file = std::ifstream("data/load_randint_slow");
    }else{
        file = std::ifstream("data/load_randint");
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

template<class B>
class BitVector{
    public:
    enum { n_bits_block = sizeof(B)*__CHAR_BIT__};
    B *v;
    size_t n;
    explicit BitVector(size_t n): v{new B[n]}, n{n} {
        assert(sizeof(B)*n  == 512/8 * n);
    }
    ~BitVector(){
        delete[] v;
    }
    bool operator [](size_t pos){
        return v[pos/sizeof(B)][pos%sizeof(B)];
    }
    void set(size_t pos){
        v[pos/sizeof(B)].set(pos%sizeof(B));
    }
    void clear(size_t pos){
        v[pos/sizeof(B)].clear(pos%sizeof(B));
    }
};


static void BM_SetBitsStd(benchmark::State& state) {
    vector<bool> bit_vector_std(state.range());
    for (auto _ : state){ 
        state.PauseTiming();
        auto i = rand() % state.range();
        state.ResumeTiming();
        auto v = true;
        benchmark::DoNotOptimize(bit_vector_std[i] = v);
    }
}
static const auto B = 512;

static void BM_SetBitsNoStd(benchmark::State& state) {
    BitVector<Block<B>> bit_vector(state.range()/B);
    for (auto _ : state){ 
        state.PauseTiming();
        auto i = rand() % state.range();
        state.ResumeTiming();
        bit_vector.set(i);
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
            file = std::ifstream("data/load_randint_slow");
        }else{
            file = std::ifstream("data/load_randint");
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

BENCHMARK(BM_ReadVector)
    ->ArgPair(Kind::Fragmented, 1000)
    ->ArgPair(Kind::Fragmented, 10000)
    ->ArgPair(Kind::Fragmented, 100000)
    ->ArgPair(Kind::Fragmented, 1000000);
BENCHMARK(BM_ReadFile)
    ->ArgPair(Kind::Fragmented, 1000)
    ->ArgPair(Kind::Fragmented, 10000)
    ->ArgPair(Kind::Fragmented, 100000)
    ->ArgPair(Kind::Fragmented, 1000000);
BENCHMARK(BM_ReadVector)
    ->ArgPair(Kind::NotFragmented, 1000)
    ->ArgPair(Kind::NotFragmented, 10000)
    ->ArgPair(Kind::NotFragmented, 100000)
    ->ArgPair(Kind::NotFragmented, 1000000);
BENCHMARK(BM_ReadFile)
    ->ArgPair(Kind::NotFragmented, 1000)
    ->ArgPair(Kind::NotFragmented, 10000)
    ->ArgPair(Kind::NotFragmented, 100000)
    ->ArgPair(Kind::NotFragmented, 100000);
// BENCHMARK(BM_ReadVector)->Arg(Kind::NotFragmented);
// BENCHMARK(BM_ReadFile)->Arg(Kind::NotFragmented);

// BENCHMARK(BM_SetBitsNoStd)->Arg(B*20);
// BENCHMARK(BM_SetBitsStd)->Arg(B*20);

BENCHMARK_MAIN();
