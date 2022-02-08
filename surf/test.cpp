#include <gtest/gtest.h>
#include <iostream>
#include <initializer_list>
#include "surf.h"
#include "utils.h"
#include <span>


// Demonstrate some basic assertions.
TEST(LoudsBuilderTest, PrimitiveTrie) {
  // using namespace std::string_literals;
  std::stringstream iss;
  iss << "aaa" << std::endl;
  iss << "aba" << std::endl;
  iss << "abb" << std::endl;
  iss << "aca" << std::endl;

  auto n_levels = 3;
  std::vector<std::vector<char>> labels =     {{'a'}, {'a', 'b', 'c'}, {'a', 'a', 'b', 'a'}};
  std::vector<std::vector<bool>> has_child =  {{ 1 }, { 1,   1,   1 }, { 0,   0,   0,   0 }};
  std::vector<std::vector<bool>> louds =      {{ 1 }, { 1,   0,   0 }, { 1,   1,   0,   1 }};
  auto builder = LoudsBuilder::from_stream(iss);
  ASSERT_EQ(builder.n_levels, n_levels) << "Number of levels is wrong, expected " << n_levels;
  ASSERT_TRUE(eq(size(builder.labels),size(builder.has_child), size(builder.louds), size(labels))) << "Not all sizes are the same";
  for(auto i : {0,1,2}){
    ASSERT_EQ(size(builder.labels[i]), size(labels[i])) << "nested labels size is different at level" << i;
    for (auto j=0; j < size(builder.labels[i]); j++){
      ASSERT_EQ(builder.labels[i][j], labels[i][j]) << "different labels element found at level " << i << " position " << j;
    }
    ASSERT_EQ(size(builder.has_child[i]), size(has_child[i]))  << "nested has_child size is different at level " << i;
    for (auto j=0; j < size(builder.has_child[i]); j++)
      ASSERT_EQ(builder.has_child[i][j], has_child[i][j]) << "different labels element found at level " << i << " position " << j;
    ASSERT_EQ(size(builder.louds[i]), size(louds[i])) << "nested louds size is different at level " << i;
    for (auto j=0; j < size(builder.louds[i]); j++)
      ASSERT_EQ(builder.louds[i][j], louds[i][j]) << "different louds element found at level " << i << " position " << j; 
  }
}


TEST(LoudsSparseTest, PrimitiveTrie) {
  // using namespace std::string_literals;
  std::stringstream iss;
  iss << "aaa" << std::endl;
  iss << "aba" << std::endl;
  iss << "abb" << std::endl;
  iss << "aca" << std::endl;

  auto n_levels = 3;
  std::vector<char> labels =     {'a', 'a', 'b', 'c', 'a', 'a', 'b', 'a'};
  std::vector<bool> has_child =  { 1,   1,   1,   1,   0,   0,   0,   0 };
  std::vector<bool> louds =      { 1 ,  1,   0,   0,   1,   1,   0,   1 };
  auto builder = LoudsBuilder::from_stream(iss);
  auto sparse = LoudsSparse<char>::from_builder(builder);
  ASSERT_EQ(builder.n_levels, n_levels) << "Number of levels is wrong, expected " << n_levels;
  ASSERT_TRUE(eq(size(sparse.labels),size(sparse.has_child), size(sparse.louds), size(labels))) << "Not all sizes are the same";
  ASSERT_EQ(size(sparse.labels), size(labels));
  for (auto j=0; j < size(sparse.labels); j++){
    ASSERT_EQ(sparse.labels[j], labels[j]) << "different labels element found at position " << j;
  }
  ASSERT_EQ(size(sparse.has_child), size(has_child));
  for (auto j=0; j < size(sparse.has_child); j++)
    ASSERT_EQ(sparse.has_child[j], has_child[j]) << "different labels element found at level position " << j;
  ASSERT_EQ(size(sparse.louds), size(louds));
  for (auto j=0; j < size(sparse.louds); j++)
    ASSERT_EQ(sparse.louds[j], louds[j]) << "different louds element found at level position " << j; 
  
  ASSERT_TRUE(sparse.look_up("aaa"));
  ASSERT_TRUE(sparse.look_up("aba"));
  ASSERT_TRUE(sparse.look_up("abb"));
  ASSERT_TRUE(sparse.look_up("aca"));
  ASSERT_FALSE(sparse.look_up("acb"));
  ASSERT_FALSE(sparse.look_up("bca"));
  ASSERT_FALSE(sparse.look_up("bcad"));
  ASSERT_FALSE(sparse.look_up("dcae"));
  
}

// lets you create a bitstring containing chars 'a' 'b' as following "ab"bs
constexpr std::bitset<256> operator""_bs(const char *str, std::size_t len){
  const auto string = std::span(str, len);
  std::bitset<256> result{};
  for(const auto& c: string)
    result.set(c, true);
  return result;
}


TEST(LoudsDenseTest, PrimitiveTrie) {
  // using namespace std::string_literals;
  std::stringstream iss;
  iss << "aaa" << std::endl;
  iss << "aba" << std::endl;
  iss << "abb" << std::endl;
  iss << "aca" << std::endl;

  auto n_levels = 3;
  vector<std::bitset<256>> labels =     {"a"_bs, "abc"_bs, "a"_bs, "ab"_bs, "a"_bs};
  vector<std::bitset<256>> has_child =  {"a"_bs, "abc"_bs,  ""_bs,   ""_bs,  ""_bs};
  auto builder = LoudsBuilder::from_stream(iss);
  auto dense = LoudsDense<char>::from_builder(builder);
  ASSERT_EQ(builder.n_levels, n_levels) << "Number of levels is wrong, expected " << n_levels;
  ASSERT_TRUE(eq(size(dense.labels),size(dense.has_child), size(labels))) << "Not all sizes are the same";
  ASSERT_EQ(size(dense.labels), size(labels));
  for (auto j=0; j < size(dense.labels); j++){
    ASSERT_EQ(dense.labels[j], labels[j]) << "different labels element found at position " << j;
  }
  ASSERT_EQ(size(dense.has_child), size(has_child));
  for (auto j=0; j < size(dense.has_child); j++)
    ASSERT_EQ(dense.has_child[j], has_child[j]) << "different labels element found at level position " << j;
  
  ASSERT_TRUE(dense.look_up("aaa"));
  ASSERT_TRUE(dense.look_up("aba"));
  ASSERT_TRUE(dense.look_up("abb"));
  ASSERT_TRUE(dense.look_up("aca"));
  ASSERT_FALSE(dense.look_up("acb"));
  ASSERT_FALSE(dense.look_up("bca"));
  ASSERT_FALSE(dense.look_up("bcad"));
  ASSERT_FALSE(dense.look_up("dcae"));
}