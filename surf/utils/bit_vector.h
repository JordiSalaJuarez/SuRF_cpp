#include <vector>
#include <bitset>
#include <x86intrin.h>
#include <bit>
#include <assert.h>
#include <ranges>
#include <cstddef>

inline uint64_t nthset(uint64_t x, unsigned n) {
    return _tzcnt_u64(_pdep_u64(1ULL << n, x));
}

template<typename T>
concept IterBool = 
    std::ranges::range<T> && 
    std::same_as<std::ranges::range_value_t<T>, bool>;

template<typename T>
concept IterULongLong = 
    std::ranges::range<T> && 
    std::same_as<std::ranges::range_value_t<T>, bool>;

template<size_t N, size_t M = 512>
class BitVector{
    public:
    static_assert( std::has_single_bit(N), "N has to be a power of two" );
    static_assert( std::has_single_bit(M), "M has to be a power of two" );
    static_assert( N <= M, "N <= M does not hold" );
    size_t n_bits;
    std::vector<std::bitset<N>> bit_vector;
    std::vector<size_t> lut_rank;
    std::vector<size_t> lut_select;

    size_t get_memory_usage(){
        return std::size(lut_rank) * sizeof(size_t) + std::size(lut_select) * sizeof(size_t) + std::size(bit_vector)*sizeof(std::bitset<N>);
    }
    BitVector(size_t n): bit_vector(n/N + (n&(N-1)? 1:0)), lut_rank(n/M + (n&(M-1)? 1:0)), lut_select(n/M + (n&(M-1)? 1:0)), n_bits(n) {}
    template <IterBool Iter>
    BitVector(const Iter &other, auto n): bit_vector(n/N + (n&(N-1)? 1:0)), lut_rank(n/M + (n&(M-1)? 1:0)), lut_select(n/M + (n&(M-1)? 1:0)), n_bits(n){
        auto i = 0;
        for(auto x: other){
            bit_vector[i/N].set(i&(N-1), x);
            ++i;
        }        
        compute_rank();
        compute_select();
    }

    BitVector(std::ranges::sized_range auto other): n_bits(std::size(other)),
        bit_vector(n_bits/N + (n_bits&(N-1)? 1:0)), 
        lut_rank(n_bits/M + (n_bits&(M-1)? 1:0)), 
        lut_select(n_bits/M + (n_bits&(M-1)? 1:0)) {
        if (n_bits == 0) return;
        auto i = 0;
        for(auto x: other){
            bit_vector[i/N].set(i&(N-1), x);
            ++i;
        }        
        compute_rank();
        compute_select();
    }
    void compute_rank(){
        assert(std::size(lut_rank) > 0);
        auto sum = 0;
        auto n_blocks = n_bits/M + (n_bits&(M-1)? 1:0);
        auto n_words_block = M/N;
        for (auto i = 0 ; i < n_blocks; ++i){
            lut_rank[i] = sum;
            for (auto j = i * n_words_block ; j < (i+1) * n_words_block; ++j){
                sum += bit_vector[j].count();
            }
        }
    }

    void compute_select(){
        auto n_set_bits = rank(n_bits - 1);
        auto n_blocks = n_set_bits/M + (n_set_bits&(M-1)? 1:0);
        auto n_words = n_set_bits/N + (n_set_bits&(N-1)? 1:0);
        lut_select.resize(n_blocks);
        lut_select.shrink_to_fit();
        if (std::size(lut_select) == 0) return;
        auto curr_count = 0;
        auto next_count = 1;
        auto j = 0;
        for(auto i = 0; i < n_bits/N; ++i){
            auto count = bit_vector[i].count(); 
            if(curr_count + count >= next_count){
                lut_select[j++] = i*N + nthset(bit_vector[i].to_ullong(), next_count - curr_count - 1);
                next_count += M; 
            }
            curr_count += count;
        }
    }

    auto next(size_t pos) -> size_t{
        auto offset = (pos & (N-1)) + 1;
        auto word = bit_vector[pos / N] >> offset << offset;
        if (word.any()) return (pos&~(N-1)) + nthset(word.to_ullong(), 0);
        for (auto i = pos/N + 1 ; i < std::size(bit_vector); ++i){
            word = bit_vector[i];
            if (word.any()) return i*N + nthset(word.to_ullong(), 0);
        }
        return n_bits;
    }

    auto succ(size_t pos) -> size_t{
        return next(pos);
    }


    size_t rank(size_t pos){
        assert(pos < n_bits);
        auto n_words_block = M / N;
        auto sum = lut_rank[pos / M];
        for (auto i = (pos / M) * n_words_block ; i < pos / N; ++i)
            sum += bit_vector[i].count();
        return sum + (bit_vector[pos / N] << ((N-1) - (pos & (N-1)))).count();
    }

    // size_t rank(size_t pos){
    //     assert(pos < n_bits);
    //     return lut_rank[pos / N] + (bit_vector[pos / N] << ((N-1) - (pos & (N-1)))).count();;
    // }

    size_t select(size_t count){
        if (count > rank(n_bits-1)) return n_bits; // can we remove this
        --count;
        auto pos = lut_select[count/M];
        // remaining count after LUT
        count = count&(M-1); 
         // if all counted return pos (LUT contains pos)
        if (!count) return pos; 
        auto i = pos/N;
        // offset of the last bit counted bit LUT
        auto offset = (pos&(N-1))+1;
        // zero out bits already counted from the word
        auto word = bit_vector[i] >> offset << offset; 
        while(word.count() < count){
            count -= word.count();
            word = bit_vector[++i];
        }
        return i*N + nthset(word.to_ullong(), count-1);
    }


    // size_t select(size_t pos){
    //     // assert(pos <= rank(n_bits-1));
    //     --pos;
    //     if (pos / N >= std::size(lut_select)) return n_bits;
    //     auto i = pos / N;
    //     auto rem = pos & (N-1);
    //     auto bit_pos = lut_select[i];
    //     auto bitset = bit_vector[bit_pos / N] >> (bit_pos & (N-1));
    //     auto count = bitset.count() - 1;
    //     if (count >= rem){
    //         return bit_pos + nthset(bitset.to_ullong(), rem);
    //     }else{
    //         rem -= count;
    //     }
    //     for (auto i = bit_pos / N + 1; i < std::size(bit_vector); ++i){
    //         auto count = bit_vector[i].count();
    //         if (count >= rem){
    //             if (rem == 0) return i*N;
    //             else return i*N + nthset(bit_vector[i].to_ullong(), rem-1);
    //         }else{
    //             rem -= count;
    //         }
    //     }
    //     return n_bits;
    //     // assert(false); // Failed to find position
    // }

    const size_t size() const {
        return n_bits;
    }

    size_t size() {
        return n_bits;
    }

    inline bool operator[] (size_t i) const noexcept {
        return bit_vector[i/N][i&(N-1)];
    }

};

// template<typename SB, typename B>
// struct SuperBlock{
//     enum : size_t{n_bytes = sizeof(SB), n_blocks = sizeof(SB)/ sizeof(B), n_bits_block = sizeof(B)*8, n_bits_super = sizeof(SB)*8 };
//     union {
//         SB m_block;
//         B m_data[n_blocks];
//     };
//     auto count() -> size_t;
//     auto block() -> SB&;
//     auto data() -> B*;
//     auto rank(size_t) -> size_t;
//     auto select(size_t) -> size_t;
//     auto operator[](size_t) -> bool;
// };

// struct Foo :SuperBlock <__m512i, std::byte>{
//     auto operator[](size_t bit) -> bool {
//         return static_cast<bool>(m_data[bit / n_bits_block] & static_cast<std::byte>(0x1 << (bit & (n_bits_block-1))));
//     }
//     auto count() -> size_t {
//         return _mm512_popcnt_epi64(m_block);
//     }
// };


