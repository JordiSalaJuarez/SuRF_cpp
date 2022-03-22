#include "../utils/bit_vector.h"
#include <gtest/gtest.h>
#include <algorithm>


template<size_t N>
void bit_vector_primitive_test(){
    std::string bits("10101011110");
    std::vector<bool> vector;
    vector.reserve(bits.size());
    for(const auto& a : bits)
        vector.push_back(a == '1');
    BitVector<N> bit_vector(vector, size(bits));
    
    // Test operator[]
    for(auto i = 0; i < size(bits); ++i){
        EXPECT_EQ(bit_vector[i], bits[i] == '1');
    }

    // Test rank
    for(auto i = 0; i < size(bits); ++i){
        auto res = std::count(begin(vector), begin(vector)+i+1, true);
        EXPECT_EQ(bit_vector.rank(i), res);
    }
    auto n = bit_vector.rank(size(vector)-1);
    std::vector<size_t> select(n + 1, 0);

    for (auto i = 0, j = 1; i < size(vector); ++i){
        if (vector[i]){
            select[j++] = i;
        }
    }
    // Test select
    for(auto i = 1 ; i < n + 1; ++i){
        EXPECT_EQ(bit_vector.select(i), select[i]);
    }
}

template<size_t N>
void bit_vector_complex_test(){
    // yes, this is a long string
    std::string bits("0111000101010011001101100101110000101000011010101011000001001110100000011110100111110000000010001101101101011010111001000101110001111001010010010000110101110000111100101011110011101000110101011100110111000111111001011111001101111100110111000110001111101010");
    std::vector<bool> vector;
    vector.reserve(bits.size());
    for(const auto& a : bits)
        vector.push_back(a == '1');
    BitVector<N> bit_vector(vector, size(bits));
    
    // Test operator[]
    for(auto i = 0; i < size(bits); ++i){
        EXPECT_EQ(bit_vector[i], bits[i] == '1');
    }

    // Test rank
    for(auto i = 0; i < size(bits); ++i){
        auto res = std::count(begin(vector), begin(vector)+i+1, true);
        EXPECT_EQ(bit_vector.rank(i), res);
    }
    auto n = bit_vector.rank(size(vector)-1);
    std::vector<size_t> select(n + 1, 0);

    for (auto i = 0, j = 1; i < size(vector); ++i){
        if (vector[i]){
            select[j++] = i;
        }
    }
    // Test select
    for(auto i = 1 ; i < n + 1; ++i){
        EXPECT_EQ(bit_vector.select(i), select[i]);
    }
}

// Primitive tests (for debugging)
TEST(BitVectorTest, Primitive_2) { bit_vector_primitive_test<2>(); }
TEST(BitVectorTest, Primitive_4) { bit_vector_primitive_test<4>(); }
// Complex tests
TEST(BitVectorTest, Complex_2) {   bit_vector_complex_test<2>();  }
TEST(BitVectorTest, Complex_4) {   bit_vector_complex_test<4>();  }
TEST(BitVectorTest, Complex_8) {   bit_vector_complex_test<8>();  }
TEST(BitVectorTest, Complex_16) {  bit_vector_complex_test<16>(); }
TEST(BitVectorTest, Complex_32) {  bit_vector_complex_test<32>(); }
TEST(BitVectorTest, Complex_64) {  bit_vector_complex_test<64>(); }