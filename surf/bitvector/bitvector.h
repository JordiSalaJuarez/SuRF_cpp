#include <cstdint>
#include <immintrin.h>
#include <span>
#include <bit>

constexpr size_t K = 4;

using Word64 = int64_t;
using Word512 = union{ __m512i m; int64_t i[8];};

[[gnu::target("popcnt")]] 
inline auto popcount(Word64 word) -> size_t {
    return __builtin_popcount(word);
}

[[gnu::target("default")]] 
inline auto popcount(Word64 word) -> size_t {
    word = word - ((word >> 1) & 0x5555555555555555);
    word = (word & 0x3333333333333333) + ((word >> 2) & 0x3333333333333333);
    return ((word + (word >> 4) & 0x0F0F0F0F0F0F0F0F) * 0x0101010101010101) >> 56;
}






// auto rank(std::span<Word512> w, std::span<size_t> r, size_t i) -> size_t{
//     constexpr size_t bits_per_block = Word512::w * K;
//     constexpr size_t words_per_block = Word512::w / K;
//     constexpr size_t words_diff = (Word512::w / Word64::w);
//     size_t idx_block = i / bits_per_block;
//     size_t last_block = i/Word512::w;
//     size_t ans = r[idx_block];
//     for (size_t u = idx_block * K; i < last_block; ++u){
//         ans += popcount(w[u]);
//     }
//     Word512 last_word = w[last_block]; 
//     for (size_t v = 0; v < i / words_diff; ++v){
//         ans += popcount(last_word.words[v]);
//     }
//     return ans + popcount(last_word.words[i/words_diff].data << (i & (words_diff - 1)));
// }

// auto compute_rank(std::span<Word512> w, std::span<size_t> r){
//     size_t curr_rank = 0;
//     for (size_t i = 0; i < w.size(); ++i){
//         if(i & )
//         curr_rank += popcount(w[i]);
//     }
//     r = 
// }