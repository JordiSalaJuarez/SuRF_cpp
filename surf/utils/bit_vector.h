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

template<size_t N>
class BitVector{
    public:
    static_assert(std::has_single_bit(N), "N has to be a power of two" );
    typedef std::vector<std::vector<bool>> LevelBitVec; 
    std::vector<std::bitset<N>> bit_vector;
    std::vector<size_t> lut_rank;
    std::vector<size_t> lut_select;
    size_t n_bits;
    BitVector(size_t n): bit_vector(n/N + (n&(N-1)? 1:0)), lut_rank(n/N + (n&(N-1)? 1:0)), lut_select(n/N + (n&(N-1)? 1:0)), n_bits(n) {}
    BitVector(const NestedIterBool auto &other, auto n): bit_vector(n/N + (n&(N-1)? 1:0)), lut_rank(n/N + (n&(N-1)? 1:0)), lut_select(n/N + (n&(N-1)? 1:0)), n_bits(n){
        auto i = 0;
        for(const auto &xs: other)
            for(auto x: xs){
                bit_vector[i/N].set(i&(N-1), x);
                ++i;
            }        
        compute_rank();
        compute_select();
    }
    BitVector(const IterBool auto &other, auto n): bit_vector(n/N + (n&(N-1)? 1:0)), lut_rank(n/N + (n&(N-1)? 1:0)), lut_select(n/N + (n&(N-1)? 1:0)), n_bits(n){
        auto i = 0;
        for(auto x: other){
            bit_vector[i/N].set(i&(N-1), x);
            ++i;
        }        
        compute_rank();
        compute_select();
    }
    void compute_rank(){
        assert(size(lut_rank) > 0);
        lut_rank[0] = 0;
        for (auto i = 1 ; i < size(bit_vector); ++i){
            lut_rank[i] = lut_rank[i-1] + bit_vector[i-1].count();
        }
    }

    void compute_select(){
        assert(size(lut_rank) > 0);
        auto bs_before = 0, bs_after = 0; // bits set before and after
        auto j = 1; 
        auto i = 0;
        while(bit_vector[i].none()) ++i;
        lut_select[0] = i*N + nthset(bit_vector[i].to_ullong(), 0);
        bs_before = bit_vector[i++].count()-1;
        bs_after = bs_before;
        for (; i < size(bit_vector); ++i){
            bs_after += bit_vector[i].count();
            if (bs_before/N  != bs_after/N){
                lut_select[bs_after/N] = i*N + nthset(bit_vector[i].to_ullong(), (bs_after & ~(N-1)) - bs_before - 1);
            };
            bs_before = bs_after;
        }
    }

    size_t rank(size_t pos){
        assert(pos < n_bits);
        return lut_rank[pos / N] + (bit_vector[pos / N] << ((N-1) - (pos & (N-1)))).count();;
    }

    size_t select(size_t pos){
        assert(pos <= rank(n_bits-1));
        --pos;
        auto i = pos / N;
        auto rem = pos & (N-1);
        auto bit_pos = lut_select[i];
        auto bitset = bit_vector[bit_pos / N] >> (bit_pos & (N-1));
        auto count = bitset.count() - 1;
        if (count >= rem){
            return bit_pos + nthset(bitset.to_ullong(), rem);
        }else{
            rem -= count;
        }
        for (auto i = bit_pos / N + 1; i < size(bit_vector); ++i){
            auto count = bit_vector[i].count();
            if (count >= rem){
                if (rem == 0) return bit_pos;
                else return i*N + nthset(bit_vector[i].to_ullong(), rem-1);
            }else{
                rem -= count;
            }
        }
        assert(false); // Failed to find position
    }

    inline bool operator[] (size_t i) const {
        return bit_vector[i/N][i&(N-1)];
    }

};