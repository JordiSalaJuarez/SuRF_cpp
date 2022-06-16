#include <gtest/gtest.h>
#include <iostream>
#include <initializer_list>
#include "bloom/bloom.h"
#include "builder.h"
#include "suffix.h"
#include "surf.h"
#include "utils.h"
#include <span>
#include "input.h"

using namespace std;
using namespace yas;
// Demonstrate some basic assertions.
TEST(LoudsBuilderTest, PrimitiveTrie) {
  // using namespace std::string_literals;
  vector<std::string> keys {
    "aaa",
    "aba",
    "abb",
    "aca"
  };

  auto n_levels = 3;
  vector<vector<char>> labels =     {{'a'}, {'a', 'b', 'c'}, {'a', 'b'}};
  vector<vector<bool>> has_child =  {{ 1 }, { 0,   1,   0 }, { 0,   0 }};
  vector<vector<bool>> louds =      {{ 1 }, { 1,   0,   0 }, { 1,   0 }};
  vector<vector<string>> suffix =   {{}, {"a", "a"}, {"", ""}};
  auto builder = LoudsBuilder(keys);
  ASSERT_EQ(builder.n_levels, n_levels) << "Number of levels is wrong, expected " << n_levels;
  ASSERT_TRUE(eq(size(builder.labels),size(builder.has_child), size(builder.louds), size(labels))) << "Not all sizes are the same";
  for(auto i : {0,1,2}){
    ASSERT_EQ(size(builder.labels[i]), size(labels[i])) << "nested labels size is different at level " << i;
    for (auto j=0; j < size(builder.labels[i]); j++){
      EXPECT_EQ(builder.labels[i][j], labels[i][j]) << "different labels element found at level " << i << " position " << j;
    }
    ASSERT_EQ(size(builder.has_child[i]), size(has_child[i]))  << "nested has_child size is different at level " << i;
    for (auto j=0; j < size(builder.has_child[i]); j++)
      EXPECT_EQ(builder.has_child[i][j], has_child[i][j]) << "different labels element found at level " << i << " position " << j;
    ASSERT_EQ(size(builder.louds[i]), size(louds[i])) << "nested louds size is different at level " << i;
    for (auto j=0; j < size(builder.louds[i]); j++)
      EXPECT_EQ(builder.louds[i][j], louds[i][j]) << "different louds element found at level " << i << " position " << j; 
    ASSERT_EQ(size(builder.suffixes[i]), size(suffix[i])) << "nested suffix size is different at level " << i;
    for (auto j=0; j < size(builder.suffixes[i]); j++)
      EXPECT_EQ(builder.suffixes[i][j], suffix[i][j]) << "different suffixes found at level " << i << "position " << j << " where " << builder.suffixes[i][j] << " != " << suffix[i][j];
  }
 
}



TEST(LoudsBuilderTest, ComplexTrie) {
  // using namespace std::string_literals;
  std::vector<std::string> keys{
    "f",
    "farther",
    "fas",
    "fasten",
    "fat",
    "splice",
    "topper",
    "toy",
    "tries",
    "tripper",
    "trying"
  };

  auto n_levels = 4;
  std::vector<std::vector<char>> labels =     {{'f', 's', 't'}, {'\0', 'a', 'o', 'r'}, {'r', 's', 't', 'p', 'y', 'i', 'y'}, {'\0', 't', 'e', 'p'}};
  std::vector<std::vector<bool>> has_child =  {{ 1 ,  0 ,  1 }, { 0 ,  1 ,  1 ,  1 }, { 0 ,  1 ,  0 ,  0 ,  0 ,  1 ,  0 }, { 0 ,  0 ,  0 ,  0 }};
  std::vector<std::vector<bool>> louds =      {{ 1 ,  0 ,  0 }, { 1 ,  0 ,  1 ,  0 }, { 1 ,  0 ,  0 ,  1 ,  0 ,  1 ,  0 }, { 1 ,  0 ,  1 ,  0 }};
  vector<vector<string>> suffix = {{"plice"}, {""}, {"ther", "", "per", "", "ing"}, {"", "en", "s", "per"}};
  auto builder = LoudsBuilder(keys);
  ASSERT_EQ(builder.n_levels, n_levels) << "Number of levels is wrong, expected " << n_levels;
  ASSERT_TRUE(eq(size(builder.labels),size(builder.has_child), size(builder.louds), size(labels))) << "Not all sizes are the same";
  for(auto i : {0,1,2,3}){
    ASSERT_EQ(size(builder.labels[i]), size(labels[i])) << "nested labels size is different at level" << i;
    for (auto j=0; j < size(builder.labels[i]); j++){
      EXPECT_EQ(builder.labels[i][j], labels[i][j]) << "different labels element found at level " << i << " position " << j;
    }
    ASSERT_EQ(size(builder.has_child[i]), size(has_child[i]))  << "nested has_child size is different at level " << i;
    for (auto j=0; j < size(builder.has_child[i]); j++)
      EXPECT_EQ(builder.has_child[i][j], has_child[i][j]) << "different labels element found at level " << i << " position " << j;
    ASSERT_EQ(size(builder.louds[i]), size(louds[i])) << "nested louds size is different at level " << i;
    for (auto j=0; j < size(builder.louds[i]); j++)
      EXPECT_EQ(builder.louds[i][j], louds[i][j]) << "different louds element found at level " << i << " position " << j; 
    ASSERT_EQ(size(builder.suffixes[i]), size(suffix[i])) << "nested suffix size is different at level " << i;
    for (auto j=0; j < size(builder.suffixes[i]); j++)
      EXPECT_EQ(builder.suffixes[i][j], suffix[i][j]) << "different suffixes found at level " << i << "position " << j << " where " << builder.suffixes[i][j] << " != " << suffix[i][j];

  }
}


TEST(LoudsSparseTest, PrimitiveTrie) {
  // using namespace std::string_literals;
  vector<std::string> keys {
  "aaa",
  "aba",
  "abb",
  "aca"
  };

  auto n_levels = 3;
  std::vector<char> labels =     {'a', 'a', 'b', 'c', 'a', 'b'};
  std::vector<bool> has_child =  { 1 ,  0,   1,   0 ,  0,   0 };
  std::vector<bool> louds =      { 1 ,  1,   0,   0 ,  1,   0 };
  vector<string> suffix =        { "a", "a", "", ""};
  auto sparse = LoudsSparse<suffix::SuffixArray<kind::Suffix>>(keys);
  ASSERT_TRUE(eq(size(sparse.labels),size(sparse.has_child), size(sparse.louds), size(labels))) << "Not all sizes are the same";
  ASSERT_EQ(size(sparse.labels), size(labels));
  for (auto j=0; j < size(sparse.labels); j++){
    EXPECT_EQ(sparse.labels[j], labels[j]) << "different labels element found at position " << j;
  }
  ASSERT_EQ(size(sparse.has_child), size(has_child));
  for (auto j=0; j < size(sparse.has_child); j++)
    EXPECT_EQ(sparse.has_child[j], has_child[j]) << "different labels element found at level position " << j;
  ASSERT_EQ(size(sparse.louds), size(louds));
  for (auto j=0; j < size(sparse.louds); j++)
    EXPECT_EQ(sparse.louds[j], louds[j]) << "different louds element found at level position " << j; 

  // ASSERT_EQ(size(sparse.values), size(suffix));
  // for (auto j=0; j < size(sparse.values); j++)
  //   EXPECT_EQ(sparse.values[j], suffix[j]) << "different values found at level position " << j; 
  
  EXPECT_TRUE(sparse.look_up("aaa"));
  EXPECT_TRUE(sparse.look_up("aba"));
  EXPECT_TRUE(sparse.look_up("abb"));
  EXPECT_TRUE(sparse.look_up("aca"));
  EXPECT_FALSE(sparse.look_up("acb"));
  EXPECT_FALSE(sparse.look_up("bca"));
  EXPECT_FALSE(sparse.look_up("bcad"));
  EXPECT_FALSE(sparse.look_up("dcae"));
  
}

TEST(LoudsSparseTest, ComplexTrie) {
  // using namespace std::string_literals;
  std::vector<std::string> keys{
    "f",
    "farther",
    "fas",
    "fasten",
    "fat",
    "splice",
    "topper",
    "toy",
    "tries",
    "tripper",
    "trying"
  };

  auto n_levels = 4;
  vector<char> labels =     {'f', 's', 't', '\0', 'a', 'o', 'r', 'r', 's', 't', 'p', 'y', 'i', 'y', '\0', 't', 'e', 'p'};
  vector<bool> has_child =  { 1 ,  0 ,  1 ,  0 ,  1 ,  1 ,  1 ,  0 ,  1 ,  0 ,  0 ,  0 ,  1 ,  0 ,  0 ,  0 ,  0 ,  0 };
  vector<bool> louds =      { 1 ,  0 ,  0 ,  1 ,  0 ,  1 ,  0 ,  1 ,  0 ,  0 ,  1 ,  0 ,  1 ,  0 ,  1 ,  0 ,  1 ,  0 };
  vector<string> suffix =   {"plice", "", "ther", "", "per", "", "ing", "", "en", "s", "per"};


  auto sparse = LoudsSparse<suffix::SuffixArray<kind::Suffix>>(keys);
  ASSERT_TRUE(eq(size(sparse.labels),size(sparse.has_child), size(sparse.louds), size(labels))) << "Not all sizes are the same";
  ASSERT_EQ(size(sparse.labels), size(labels));
  for (auto j=0; j < size(sparse.labels); j++){
    EXPECT_EQ(sparse.labels[j], labels[j]) << "different labels element found at position " << j;
  }
  ASSERT_EQ(size(sparse.has_child), size(has_child));
  for (auto j=0; j < size(sparse.has_child); j++)
    EXPECT_EQ(sparse.has_child[j], has_child[j]) << "different labels element found at level position " << j;
  ASSERT_EQ(size(sparse.louds), size(louds));
  for (auto j=0; j < size(sparse.louds); j++)
    EXPECT_EQ(sparse.louds[j], louds[j]) << "different louds element found at level position " << j; 

  // ASSERT_EQ(size(sparse.values), size(suffix));
  // for (auto j=0; j < size(sparse.values); j++)
  //   EXPECT_EQ(sparse.values[j], suffix[j]) << "different values found at level position " << j; 
  

  EXPECT_TRUE(sparse.look_up("f"));
  EXPECT_TRUE(sparse.look_up("farther"));
  EXPECT_TRUE(sparse.look_up("fas"));
  EXPECT_TRUE(sparse.look_up("fasten"));
  EXPECT_TRUE(sparse.look_up("fat"));
  EXPECT_TRUE(sparse.look_up("splice"));
  EXPECT_TRUE(sparse.look_up("topper"));
  EXPECT_TRUE(sparse.look_up("toy"));
  EXPECT_TRUE(sparse.look_up("tries"));
  EXPECT_TRUE(sparse.look_up("tripper"));
  EXPECT_TRUE(sparse.look_up("trying"));
  EXPECT_FALSE(sparse.look_up("try"));
  EXPECT_FALSE(sparse.look_up("top"));
  EXPECT_FALSE(sparse.look_up("tor"));
  EXPECT_FALSE(sparse.look_up("spline"));
  EXPECT_FALSE(sparse.look_up("toys"));

  for (auto i = 0; i < std::size(keys); ++i){
      EXPECT_EQ(sparse.lb(keys[i]), keys[i]);
  }

  for (auto i = 0; i < std::size(keys); ++i){
      EXPECT_EQ(sparse.ub(keys[i]), keys[i]);
  }

  for (auto i = 0; i < std::size(keys)-2; ++i){
      EXPECT_TRUE(sparse.look_up_range(keys[i], keys[i+2])) << "from: " << keys[i] << " to: " << keys[i+2];
  }


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
  vector<std::string> keys {
    "aaa",
    "aba",
    "abb",
    "aca"
  };

  auto n_levels = 3;
  vector<std::bitset<256>> labels =     {"a"_bs, "abc"_bs, "ab"_bs };
  vector<std::bitset<256>> has_child =  {"a"_bs,  "b"_bs,  ""_bs };
  vector<string> suffix =               { "a", "a", "", ""};

  auto dense = LoudsDense<suffix::SuffixArray<kind::Suffix>>(keys);
  ASSERT_TRUE(eq(size(dense.labels),size(dense.has_child), size(labels))) << "Not all sizes are the same";
  ASSERT_EQ(size(dense.labels), size(labels));
  for (auto j=0; j < size(dense.labels); j++){
    EXPECT_EQ(dense.labels[j], labels[j]) << "different labels element found at position " << j;
  }
  ASSERT_EQ(size(dense.has_child), size(has_child));
  for (auto j=0; j < size(dense.has_child); j++)
    EXPECT_EQ(dense.has_child[j], has_child[j]) << "different labels element found at level position " << j;
  
  // ASSERT_EQ(size(dense.values), size(suffix));
  // for (auto j=0; j < size(dense.values); j++)
  //   EXPECT_TRUE(dense.values[j]->check_disk(disk, j), suffix[j]) << "different values found at level position " << j; 

  EXPECT_TRUE(dense.look_up("aaa"));
  EXPECT_TRUE(dense.look_up("aba"));
  EXPECT_TRUE(dense.look_up("abb"));
  EXPECT_TRUE(dense.look_up("aca"));
  EXPECT_FALSE(dense.look_up("acb"));
  EXPECT_FALSE(dense.look_up("bca"));
  EXPECT_FALSE(dense.look_up("bcad"));
  EXPECT_FALSE(dense.look_up("dcae"));
}

TEST(LoudsDenseTest, ComplexTrie) {
  std::vector<std::string> keys{
    "f",
    "farther",
    "fas",
    "fasten",
    "fat",
    "splice",
    "topper",
    "toy",
    "tries",
    "tripper",
    "trying"
  };

  auto n_levels = 4;
  vector<std::bitset<256>> labels =     {"fst"_bs, "\0a"_bs, "or"_bs, "rst"_bs, "py"_bs, "iy"_bs, "\0t"_bs, "ep"_bs};
  vector<std::bitset<256>> has_child =  { "ft"_bs,  "a"_bs, "or"_bs ,  "s"_bs ,  ""_bs , "i"_bs ,  ""_bs ,  ""_bs};
  vector<string> suffix =               {"plice", "", "ther", "", "per", "", "ing", "", "en", "s", "per"};

  
  auto dense = LoudsDense<suffix::SuffixArray<kind::Suffix>>(keys);
  ASSERT_TRUE(eq(size(dense.labels),size(dense.has_child), size(labels))) << "Not all sizes are the same";
  ASSERT_EQ(size(dense.labels), size(labels));
  for (auto j=0; j < size(dense.labels); j++){
    EXPECT_EQ(dense.labels[j], labels[j]) << "different labels element found at position " << j;
  }
  ASSERT_EQ(size(dense.has_child), size(has_child));
  for (auto j=0; j < size(dense.has_child); j++)
    EXPECT_EQ(dense.has_child[j], has_child[j]) << "different labels element found at level position " << j;
  
  // ASSERT_EQ(size(dense.values), size(suffix));
  // for (auto j=0; j < size(dense.values); j++)
  //   EXPECT_TRUE(dense.values[j].matches(suffix[j])) << "different values found at level position " << j; 

  EXPECT_TRUE(dense.look_up("f"));
  EXPECT_TRUE(dense.look_up("farther"));
  EXPECT_TRUE(dense.look_up("fas"));
  EXPECT_TRUE(dense.look_up("fasten"));
  EXPECT_TRUE(dense.look_up("fat"));
  EXPECT_TRUE(dense.look_up("splice"));
  EXPECT_TRUE(dense.look_up("topper"));
  EXPECT_TRUE(dense.look_up("toy"));
  EXPECT_TRUE(dense.look_up("tries"));
  EXPECT_TRUE(dense.look_up("tripper"));
  EXPECT_TRUE(dense.look_up("trying"));
  EXPECT_FALSE(dense.look_up("try"));
  EXPECT_FALSE(dense.look_up("top"));
  EXPECT_FALSE(dense.look_up("tor"));
  EXPECT_FALSE(dense.look_up("spline"));
}



TEST(SurfTest, PrimitiveTrie) {
  // using namespace std::string_literals;
  vector<std::string> keys {
    "aaa",
    "aba",
    "abb",
    "aca"
  };

  auto surf = yas::Surf<suffix::SuffixArray<kind::Suffix>>(keys);

  EXPECT_TRUE(surf.look_up("aaa"));
  EXPECT_TRUE(surf.look_up("aba"));
  EXPECT_TRUE(surf.look_up("abb"));
  EXPECT_TRUE(surf.look_up("aca"));
  EXPECT_FALSE(surf.look_up("acb"));
  EXPECT_FALSE(surf.look_up("bca"));
  EXPECT_FALSE(surf.look_up("bcad"));
  EXPECT_FALSE(surf.look_up("dcae"));


  auto trim = [](std::string&& key){
    if (key.back() == '\0') key.pop_back();
    return key;
  };
  auto all_keys = [&](){
    std::string begin_prefix{};
    std::string end_prefix{};
    std::vector<size_t> begin_idxs{};
    std::vector<size_t> end_idxs{};
    auto begin = surf.begin(begin_prefix, begin_idxs);
    auto end = surf.end(end_prefix, end_idxs);
    vector<string> ans{};
    for (auto it = begin; it != end; ++it){
      ans.push_back(string(*it));
    }
    return ans;
  };

  auto all_keys_reversed = [&](){
    std::string begin_prefix{};
    std::string end_prefix{};
    std::vector<size_t> begin_idxs{};
    std::vector<size_t> end_idxs{};
    auto begin = surf.begin(begin_prefix, begin_idxs);
    auto end = surf.end(end_prefix, end_idxs);
    vector<string> ans{};
    for (auto it = end; it != begin; --it){
      ans.push_back(string(*it));
    }
    return ans;
  };
  auto expected_keys = vector<string>{"aaa", "aba", "abb"};
  auto expected_keys_reversed = vector<string>{"aca", "abb", "aba"};
  EXPECT_TRUE(all_keys() == expected_keys);
  EXPECT_TRUE(all_keys_reversed() == expected_keys_reversed);
}

TEST(SurfTestSuffix, ComplexTrie) {
  // using namespace std::string_literals;
  std::vector<std::string> keys{
    "f",
    "farther",
    "fas",
    "fasten",
    "fat",
    "splice",
    "topper",
    "toy",
    "tries",
    "tripper",
    "trying"
  };


  auto surf = yas::Surf<suffix::SuffixArray<kind::Suffix>>(keys);
  auto trim = [](std::string&& key){
    if (key.back() == '\0') key.pop_back();
    return key;
  };
  auto all_keys = [&](){
    std::string begin_prefix{};
    std::string end_prefix{};
    std::vector<size_t> begin_idxs{};
    std::vector<size_t> end_idxs{};
    auto begin = surf.begin(begin_prefix, begin_idxs);
    auto end = surf.end(end_prefix, end_idxs);
    vector<string> ans{};
    for (auto it = begin; it != end; ++it){
      ans.push_back(string(*it));
    }
    return ans;
  };

  auto all_keys_reversed = [&](){
    std::string begin_prefix{};
    std::string end_prefix{};
    std::vector<size_t> begin_idxs{};
    std::vector<size_t> end_idxs{};
    auto begin = surf.begin(begin_prefix, begin_idxs);
    auto end = surf.end(end_prefix, end_idxs);
    vector<string> ans{};
    for (auto it = end; it != begin; --it){
      ans.push_back(string(*it));
    }
    return ans;
  };

  auto all_keys_lb = [&](std::string_view key){
    std::string begin_prefix{};
    std::string end_prefix{};
    std::vector<size_t> begin_idxs{};
    std::vector<size_t> end_idxs{};
    auto begin = surf.lb_iter(key, begin_prefix, begin_idxs);
    auto end = surf.end(end_prefix, end_idxs);
    vector<string> ans{};
    for (auto it = begin; it != end; ++it){
      ans.push_back(string(*it));
    }
    return ans;
  };
  auto all_keys_ub = [&](auto key){
    std::string begin_prefix{};
    std::string end_prefix{};
    std::vector<size_t> begin_idxs{};
    std::vector<size_t> end_idxs{};
    auto begin = surf.begin(begin_prefix, begin_idxs);
    auto end = surf.ub_iter(key, end_prefix, end_idxs);
    vector<string> ans{};
    for (auto it = begin; it != end; ++it){
      ans.push_back(string(*it));
    }
    return ans;
  };
  auto expected_keys = vector<string>{"f", "farther", "fas", "fasten", "fat", "splice", "topper", "toy", "tries", "tripper"};
  auto expected_keys_reversed = vector<string>{"trying", "tripper", "tries", "toy", "topper", "splice", "fat", "fasten", "fas", "farther"};
  auto expected_keys_lb = vector<string>{"fasten", "fat", "splice", "topper", "toy", "tries", "tripper"};
  auto expected_keys_ub = vector<string>{"f", "farther", "fas", "fasten", "fat", "splice", "topper"};
  auto EXPECT_VECTOR_EQ = [&](auto left, auto right){
    for (auto i = 0UL; i < left.size(); ++i){
      EXPECT_EQ(left[i], right[i]) << "Failed vector equality between " << left[i] << " and " << right[i] << " on index " << i;
    }
  };

  EXPECT_VECTOR_EQ(all_keys(), expected_keys);
  EXPECT_VECTOR_EQ(all_keys_reversed(), expected_keys_reversed);
  EXPECT_VECTOR_EQ(all_keys_lb("fasten"), expected_keys_lb);
  EXPECT_VECTOR_EQ(all_keys_ub("toy"), expected_keys_ub);
  
}


TEST(SurfTestHash, ComplexTrie) {
  // using namespace std::string_literals;
  std::vector<std::string> keys{
    "f",
    "farther",
    "fas",
    "fasten",
    "fat",
    "splice",
    "topper",
    "toy",
    "tries",
    "tripper",
    "trying"
  };


auto surf = yas::Surf<suffix::SuffixBloom<8>>(keys);
  
  EXPECT_TRUE(surf.look_up("f"));
  EXPECT_TRUE(surf.look_up("farther"));
  EXPECT_TRUE(surf.look_up("fas"));
  EXPECT_TRUE(surf.look_up("fasten"));
  EXPECT_TRUE(surf.look_up("fat"));
  EXPECT_TRUE(surf.look_up("splice"));
  EXPECT_TRUE(surf.look_up("topper"));
  EXPECT_TRUE(surf.look_up("toy"));
  EXPECT_TRUE(surf.look_up("tries"));
  EXPECT_TRUE(surf.look_up("tripper"));
  EXPECT_TRUE(surf.look_up("trying"));
  EXPECT_FALSE(surf.look_up("fave"));
  EXPECT_FALSE(surf.look_up("fass"));

  // surf.range_query("f", "trying");
  
}

TEST(LoudsSparseTest, PrimitiveTrieRange) {
  // using namespace std::string_literals;
  vector<std::string> keys {
    "aaa",
    "aba",
    "abb",
    "aca"
  };

  
  auto sparse = LoudsSparse<suffix::SuffixArray<kind::Suffix>>(keys);

  auto trim = [](std::string&& key){
    if (key.back() == '\0') key.pop_back();
    return key;
  };
  auto all_keys = [&](){
    std::string begin_prefix{};
    std::string end_prefix{};
    std::vector<size_t> begin_idxs{};
    std::vector<size_t> end_idxs{};
    auto begin = sparse.begin(begin_prefix, begin_idxs);
    auto end = sparse.end(end_prefix, end_idxs);
    vector<string> ans{};
    for (auto it = begin; it != end; ++it){
      ans.push_back(string(*it));
    }
    return ans;
  };

  auto all_keys_reversed = [&](){
    std::string begin_prefix{};
    std::string end_prefix{};
    std::vector<size_t> begin_idxs{};
    std::vector<size_t> end_idxs{};
    auto begin = sparse.begin(begin_prefix, begin_idxs);
    auto end = sparse.end(end_prefix, end_idxs);
    vector<string> ans{};
    for (auto it = end; it != begin; --it){
      ans.push_back(string(*it));
    }
    return ans;
  };
  auto expected_keys = vector<string>{"aaa", "aba", "abb"};
  auto expected_keys_reversed = vector<string>{"aca", "abb", "aba"};
  EXPECT_TRUE(all_keys() == expected_keys);
  EXPECT_TRUE(all_keys_reversed() == expected_keys_reversed);
}

TEST(LoudsSparseTest, ComplexTrieRange) {
  // using namespace std::string_literals;
  std::vector<std::string> keys{
    "f",
    "farther",
    "fas",
    "fasten",
    "fat",
    "splice",
    "topper",
    "toy",
    "tries",
    "tripper",
    "trying"
  };

  
  auto sparse = LoudsSparse<suffix::SuffixArray<kind::Suffix>>(keys);
  auto trim = [](std::string&& key){
    if (key.back() == '\0') key.pop_back();
    return key;
  };
  auto all_keys = [&](){
    std::string begin_prefix{};
    std::string end_prefix{};
    std::vector<size_t> begin_idxs{};
    std::vector<size_t> end_idxs{};
    auto begin = sparse.begin(begin_prefix, begin_idxs);
    auto end = sparse.end(end_prefix, end_idxs);
    vector<string> ans{};
    for (auto it = begin; it != end; ++it){
      ans.push_back(string(*it));
    }
    return ans;
  };

  auto all_keys_reversed = [&](){
    std::string begin_prefix{};
    std::string end_prefix{};
    std::vector<size_t> begin_idxs{};
    std::vector<size_t> end_idxs{};
    auto begin = sparse.begin(begin_prefix, begin_idxs);
    auto end = sparse.end(end_prefix, end_idxs);
    vector<string> ans{};
    for (auto it = end; it != begin; --it){
      ans.push_back(string(*it));
    }
    return ans;
  };

  auto all_keys_lb = [&](std::string_view key){
    std::string begin_prefix{};
    std::string end_prefix{};
    std::vector<size_t> begin_idxs{};
    std::vector<size_t> end_idxs{};
    auto begin = sparse.lb_iter(key, begin_prefix, begin_idxs);
    auto end = sparse.end(end_prefix, end_idxs);
    vector<string> ans{};
    for (auto it = begin; it != end; ++it){
      ans.push_back(string(*it));
    }
    return ans;
  };
  auto all_keys_ub = [&](auto key){
    std::string begin_prefix{};
    std::string end_prefix{};
    std::vector<size_t> begin_idxs{};
    std::vector<size_t> end_idxs{};
    auto begin = sparse.begin(begin_prefix, begin_idxs);
    auto end = sparse.ub_iter(key, end_prefix, end_idxs);
    vector<string> ans{};
    for (auto it = begin; it != end; ++it){
      ans.push_back(string(*it));
    }
    return ans;
  };
  auto expected_keys = vector<string>{"f", "farther", "fas", "fasten", "fat", "splice", "topper", "toy", "tries", "tripper"};
  auto expected_keys_reversed = vector<string>{"trying", "tripper", "tries", "toy", "topper", "splice", "fat", "fasten", "fas", "farther"};
  auto expected_keys_lb = vector<string>{"fasten", "fat", "splice", "topper", "toy", "tries", "tripper"};
  auto expected_keys_ub = vector<string>{"f", "farther", "fas", "fasten", "fat", "splice", "topper"};
  auto EXPECT_VECTOR_EQ = [&](auto left, auto right){
    for (auto i = 0UL; i < left.size(); ++i){
      EXPECT_EQ(left[i], right[i]) << "Failed vector equality between " << left[i] << " and " << right[i] << " on index " << i;
    }
  };

  EXPECT_VECTOR_EQ(all_keys(), expected_keys);
  EXPECT_VECTOR_EQ(all_keys_reversed(), expected_keys_reversed);
  EXPECT_VECTOR_EQ(all_keys_lb("fasten"), expected_keys_lb);
  EXPECT_VECTOR_EQ(all_keys_ub("toy"), expected_keys_ub);
}


TEST(LoudsDenseTest, PrimitiveTrieRange) {
    // using namespace std::string_literals;
  vector<std::string> keys {
    "aaa",
    "aba",
    "abb",
    "aca"
  };

  
  auto dense = LoudsDense<suffix::SuffixArray<kind::Suffix>>(keys);

  auto trim = [](std::string&& key){
    if (key.back() == '\0') key.pop_back();
    return key;
  };
  auto all_keys = [&](){
    std::string begin_prefix{};
    std::string end_prefix{};
    std::vector<size_t> begin_idxs{};
    std::vector<size_t> end_idxs{};
    auto begin = dense.begin(begin_prefix, begin_idxs);
    auto end = dense.end(end_prefix, end_idxs);
    vector<string> ans{};
    for (auto it = begin; it != end; ++it){
      ans.push_back(string(*it));
    }
    return ans;
  };

  auto all_keys_reversed = [&](){
    std::string begin_prefix{};
    std::string end_prefix{};
    std::vector<size_t> begin_idxs{};
    std::vector<size_t> end_idxs{};
    auto begin = dense.begin(begin_prefix, begin_idxs);
    auto end = dense.end(end_prefix, end_idxs);
    vector<string> ans{};
    for (auto it = end; it != begin; --it){
      ans.push_back(string(*it));
    }
    return ans;
  };
  auto expected_keys = vector<string>{"aaa", "aba", "abb"};
  auto expected_keys_reversed = vector<string>{"aca", "abb", "aba"};
  EXPECT_TRUE(all_keys() == expected_keys);
  EXPECT_TRUE(all_keys_reversed() == expected_keys_reversed);
}

TEST(LoudsDenseTest, ComplexTrieRange) {
    // using namespace std::string_literals;
  std::vector<std::string> keys{
    "f",
    "farther",
    "fas",
    "fasten",
    "fat",
    "splice",
    "topper",
    "toy",
    "tries",
    "tripper",
    "trying"
  };

  
  auto dense = LoudsDense<suffix::SuffixArray<kind::Suffix>>(keys);
  auto trim = [](std::string&& key){
    if (key.back() == '\0') key.pop_back();
    return key;
  };
  auto all_keys = [&](){
    std::string begin_prefix{};
    std::string end_prefix{};
    std::vector<size_t> begin_idxs{};
    std::vector<size_t> end_idxs{};
    auto begin = dense.begin(begin_prefix, begin_idxs);
    auto end = dense.end(end_prefix, end_idxs);
    vector<string> ans{};
    for (auto it = begin; it != end; ++it){
      ans.push_back(string(*it));
    }
    return ans;
  };

  auto all_keys_reversed = [&](){
    std::string begin_prefix{};
    std::string end_prefix{};
    std::vector<size_t> begin_idxs{};
    std::vector<size_t> end_idxs{};
    auto begin = dense.begin(begin_prefix, begin_idxs);
    auto end = dense.end(end_prefix, end_idxs);
    vector<string> ans{};
    for (auto it = end; it != begin; --it){
      ans.push_back(string(*it));
    }
    return ans;
  };

  auto all_keys_lb = [&](std::string_view key){
    std::string begin_prefix{};
    std::string end_prefix{};
    std::vector<size_t> begin_idxs{};
    std::vector<size_t> end_idxs{};
    auto begin = dense.lb_iter(key, begin_prefix, begin_idxs);
    auto end = dense.end(end_prefix, end_idxs);
    vector<string> ans{};
    for (auto it = begin; it != end; ++it){
      ans.push_back(string(*it));
    }
    return ans;
  };
  auto all_keys_ub = [&](auto key){
    std::string begin_prefix{};
    std::string end_prefix{};
    std::vector<size_t> begin_idxs{};
    std::vector<size_t> end_idxs{};
    auto begin = dense.begin(begin_prefix, begin_idxs);
    auto end = dense.ub_iter(key, end_prefix, end_idxs);
    vector<string> ans{};
    for (auto it = begin; it != end; ++it){
      ans.push_back(string(*it));
    }
    return ans;
  };
  auto expected_keys = vector<string>{"f", "farther", "fas", "fasten", "fat", "splice", "topper", "toy", "tries", "tripper"};
  auto expected_keys_reversed = vector<string>{"trying", "tripper", "tries", "toy", "topper", "splice", "fat", "fasten", "fas", "farther"};
  auto expected_keys_lb = vector<string>{"fasten", "fat", "splice", "topper", "toy", "tries", "tripper"};
  auto expected_keys_ub = vector<string>{"f", "farther", "fas", "fasten", "fat", "splice", "topper"};
  auto EXPECT_VECTOR_EQ = [&](auto left, auto right){
    for (auto i = 0UL; i < left.size(); ++i){
      EXPECT_EQ(left[i], right[i]) << "Failed vector equality between " << left[i] << " and " << right[i] << " on index " << i;
    }
  };

  EXPECT_VECTOR_EQ(all_keys(), expected_keys);
  EXPECT_VECTOR_EQ(all_keys_reversed(), expected_keys_reversed);
  EXPECT_VECTOR_EQ(all_keys_lb("fasten"), expected_keys_lb);
  EXPECT_VECTOR_EQ(all_keys_ub("toy"), expected_keys_ub);
}


TEST(BloomFilter, SampleKeys) {
    // using namespace std::string_literals;
  std::vector<std::string> keys{
    "f",
    "farther",
    "fas",
    "fasten",
    "fat",
    "splice",
    "topper",
    "toy",
    "tries",
    "tripper",
    "trying"
  };
  suffix::SuffixBloom<8> filter{keys};
  for(auto const & key: keys){
    EXPECT_TRUE(filter.contains(0, key));
  }
  // Could fail and still be right
  for(auto const & key: keys){
    EXPECT_FALSE(filter.contains(0, key + "x"));
  }
}


// TEST(BuilderPrune, ComplexTrie) {
//   // using namespace std::string_literals;
//   std::vector<std::string> keys{
//     "f",
//     "farther",
//     "fas",
//     "fasten",
//     "fat",
//     "splice",
//     "topper",
//     "toy",
//     "tries",
//     "tripper",
//     "trying"
//   };

//   auto n_levels = 4;
//   std::vector<std::vector<char>> labels =     {{'f', 's', 't'}, {'\0', 'a', 'o', 'r'}, {'r', 's', 't', 'p', 'y', 'i', 'y'}, {'\0', 't', 'e', 'p'}};
//   std::vector<std::vector<bool>> has_child =  {{ 1 ,  0 ,  1 }, { 0 ,  1 ,  1 ,  1 }, { 0 ,  1 ,  0 ,  0 ,  0 ,  1 ,  0 }, { 0 ,  0 ,  0 ,  0 }};
//   std::vector<std::vector<bool>> louds =      {{ 1 ,  0 ,  0 }, { 1 ,  0 ,  1 ,  0 }, { 1 ,  0 ,  0 ,  1 ,  0 ,  1 ,  0 }, { 1 ,  0 ,  1 ,  0 }};
//   vector<vector<string>> suffix = {{"plice"}, {""}, {"ther", "", "per", "", "ing"}, {"", "en", "s", "per"}};
//   auto builder = LoudsBuilder(keys);
//   ASSERT_EQ(builder.n_levels, n_levels) << "Number of levels is wrong, expected " << n_levels;
//   ASSERT_TRUE(eq(size(builder.labels),size(builder.has_child), size(builder.louds), size(labels))) << "Not all sizes are the same";
//   for(auto i : {0,1,2,3}){
//     ASSERT_EQ(size(builder.labels[i]), size(labels[i])) << "nested labels size is different at level" << i;
//     for (auto j=0; j < size(builder.labels[i]); j++){
//       EXPECT_EQ(builder.labels[i][j], labels[i][j]) << "different labels element found at level " << i << " position " << j;
//     }
//     ASSERT_EQ(size(builder.has_child[i]), size(has_child[i]))  << "nested has_child size is different at level " << i;
//     for (auto j=0; j < size(builder.has_child[i]); j++)
//       EXPECT_EQ(builder.has_child[i][j], has_child[i][j]) << "different labels element found at level " << i << " position " << j;
//     ASSERT_EQ(size(builder.louds[i]), size(louds[i])) << "nested louds size is different at level " << i;
//     for (auto j=0; j < size(builder.louds[i]); j++)
//       EXPECT_EQ(builder.louds[i][j], louds[i][j]) << "different louds element found at level " << i << " position " << j; 
//     ASSERT_EQ(size(builder.suffixes[i]), size(suffix[i])) << "nested suffix size is different at level " << i;
//     for (auto j=0; j < size(builder.suffixes[i]); j++)
//       EXPECT_EQ(builder.suffixes[i][j], suffix[i][j]) << "different suffixes found at level " << i << "position " << j << " where " << builder.suffixes[i][j] << " != " << suffix[i][j];


//   auto n_levels = 4;
//   vector<char> labels =     { 'f', 's', 't', '\0', 'a', 'o', 'r', 'r', 's', 't', 'i', 'y' };
//   vector<bool> has_child =  {  1 ,  0 ,  1 ,   0 ,  1 ,  0 ,  1 ,  0 ,  0 ,  0 ,  0 ,  0  };
//   vector<bool> louds =      {  1 ,  0 ,  0 ,   1 ,  0 ,  1 ,  0 ,  1 ,  0 ,  0 ,  1 ,  0  };
//   vector<string> suffix =   {"plice", "", "ther", "", "per", "", "ing", "", "en", "s", "per"};


//   auto builder = LoudsBuilderPrunning(keys, 3);
//   ASSERT_TRUE(eq(size(builder.labels),size(builder.has_child), size(builder.louds), size(labels))) << "Not all sizes are the same";
//   ASSERT_EQ(size(builder.labels), size(labels));
//   for (auto j=0; j < size(builder.labels); j++){
//     EXPECT_EQ(builder.labels[j], labels[j]) << "different labels element found at position " << j;
//   }
//   ASSERT_EQ(size(builder.has_child), size(has_child));
//   for (auto j=0; j < size(builder.has_child); j++)
//     EXPECT_EQ(builder.has_child[j], has_child[j]) << "different labels element found at level position " << j;
//   ASSERT_EQ(size(builder.louds), size(louds));
//   for (auto j=0; j < size(builder.louds); j++)
//     EXPECT_EQ(builder.louds[j], louds[j]) << "different louds element found at level position " << j; 

//   // ASSERT_EQ(size(sparse.values), size(suffix));
//   // for (auto j=0; j < size(sparse.values); j++)
//   //   EXPECT_EQ(sparse.values[j], suffix[j]) << "different values found at level position " << j; 
  

//   EXPECT_TRUE(builder.look_up("f"));
//   EXPECT_TRUE(builder.look_up("farther"));
//   EXPECT_TRUE(builder.look_up("fas"));
//   EXPECT_TRUE(builder.look_up("fasten"));
//   EXPECT_TRUE(builder.look_up("fat"));
//   EXPECT_TRUE(builder.look_up("splice"));
//   EXPECT_TRUE(builder.look_up("topper"));
//   EXPECT_TRUE(builder.look_up("toy"));
//   EXPECT_TRUE(builder.look_up("tries"));
//   EXPECT_TRUE(builder.look_up("tripper"));
//   EXPECT_TRUE(builder.look_up("trying"));
//   EXPECT_FALSE(builder.look_up("try"));
//   EXPECT_FALSE(builder.look_up("top"));
//   EXPECT_FALSE(builder.look_up("tor"));
//   EXPECT_FALSE(builder.look_up("spline"));
//   EXPECT_FALSE(builder.look_up("toys"));

//   for (auto i = 0; i < std::size(keys); ++i){
//       EXPECT_EQ(builder.lb(keys[i]), keys[i]);
//   }

//   for (auto i = 0; i < std::size(keys); ++i){
//       EXPECT_EQ(builder.ub(keys[i]), keys[i]);
//   }

//   for (auto i = 0; i < std::size(keys)-2; ++i){
//       EXPECT_TRUE(builder.look_up_range(keys[i], keys[i+2])) << "from: " << keys[i] << " to: " << keys[i+2];
//   }


// }