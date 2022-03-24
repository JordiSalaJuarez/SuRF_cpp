#include <vector>
#include <bitset>
#include <x86intrin.h>
#include <bit>
#include <assert.h>

inline uint64_t nthset(uint64_t x, unsigned n) {
    return _tzcnt_u64(_pdep_u64(1ULL << n, x));
}


template<typename T>
concept NestedIterBool = 
std::ranges::range<T> && 
std::ranges::range<std::ranges::range_value_t<T>> && 
std::same_as<std::ranges::range_value_t<std::ranges::range_value_t<T>>, bool>;

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
    static_assert(std::has_single_bit(N), "N has to be a power of two" );
    static_assert(std::has_single_bit(M), "M has to be a power of two" );
    static_assert(N <= M, "N <= M does not hold" );
    typedef std::vector<std::vector<bool>> LevelBitVec; 
    std::vector<std::bitset<N>> bit_vector;
    std::vector<size_t> lut_rank;
    std::vector<size_t> lut_select;
    size_t n_bits;
    BitVector(size_t n): bit_vector(n/N + (n&(N-1)? 1:0)), lut_rank(n/M + (n&(M-1)? 1:0)), lut_select(n/M + (n&(M-1)? 1:0)), n_bits(n) {}
    BitVector(const NestedIterBool auto &other, auto n): bit_vector(n/N + (n&(N-1)? 1:0)), lut_rank(n/M + (n&(M-1)? 1:0)), lut_select(n/M + (n&(M-1)? 1:0)), n_bits(n){
        auto i = 0;
        for(const auto &xs: other)
            for(auto x: xs){
                bit_vector[i/N].set(i&(N-1), x);
                ++i;
            }        
        compute_rank();
        compute_select();
    }
    BitVector(const IterBool auto &other, auto n): bit_vector(n/N + (n&(N-1)? 1:0)), lut_rank(n/M + (n&(M-1)? 1:0)), lut_select(n/M + (n&(M-1)? 1:0)), n_bits(n){
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

    // void compute_select(){
    //     assert(std::size(lut_rank) > 0);
    //     auto n_set_bits = rank(n_bits - 1);
    //     lut_select.resize(n_set_bits/N + (n_set_bits&(N-1)?1:0));
    //     lut_select.shrink_to_fit();
    //     if (std::size(lut_select) == 0) return;
    //     auto bs_before = 0, bs_after = 0; // bits set before and after
    //     auto j = 1; 
    //     auto i = 0;
    //     while(bit_vector[i].none()) ++i;
    //     lut_select[0] = i*N + nthset(bit_vector[i].to_ullong(), 0);
    //     bs_before = bit_vector[i++].count()-1;
    //     bs_after = bs_before;
    //     for (; i < std::size(bit_vector); ++i){
    //         bs_after += bit_vector[i].count();
    //         if (bs_before/N  != bs_after/N){
    //             lut_select[bs_after/N] = i*N + nthset(bit_vector[i].to_ullong(), (bs_after & ~(N-1)) - bs_before - 1);
    //         };
    //         bs_before = bs_after;
    //     }
        
    // }

    size_t next(size_t pos){
        auto offset = (pos & (N-1)) + 1;
        auto word = bit_vector[pos / N] >> offset << offset;
        if (word.any()) return (pos&~(N-1)) + nthset(word.to_ullong(), 0);
        for (auto i = pos/N + 1 ; i < std::size(bit_vector); ++i){
            word = bit_vector[i];
            if (word.any()) return i*N + nthset(word.to_ullong(), 0);
        }
        return n_bits;
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