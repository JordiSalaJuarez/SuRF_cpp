#include <gtest/gtest.h>
#include <iostream>
#include <initializer_list>
#include "surf.h"
#include "utils.h"
#include <span>

using namespace std;
using namespace yas;
// Demonstrate some basic assertions.
TEST(LoudsBuilderTest, PrimitiveTrie) {
  // using namespace std::string_literals;
  std::stringstream iss;
  iss << "aaa" << std::endl;
  iss << "aba" << std::endl;
  iss << "abb" << std::endl;
  iss << "aca" << std::endl;

  auto n_levels = 3;
  vector<vector<char>> labels =     {{'a'}, {'a', 'b', 'c'}, {'a', 'b'}};
  vector<vector<bool>> has_child =  {{ 1 }, { 0,   1,   0 }, { 0,   0 }};
  vector<vector<bool>> louds =      {{ 1 }, { 1,   0,   0 }, { 1,   0 }};
  vector<vector<string>> suffix =   {{}, {"a", "a"}, {"", ""}};
  auto builder = LoudsBuilder::from_stream(iss);
  ASSERT_EQ(builder.n_levels, n_levels) << "Number of levels is wrong, expected " << n_levels;
  ASSERT_TRUE(eq(size(builder.labels),size(builder.has_child), size(builder.louds), size(labels))) << "Not all sizes are the same";
  for(auto i : {0,1,2}){
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
    ASSERT_EQ(size(builder.suffix[i]), size(suffix[i])) << "nested suffix size is different at level " << i;
    for (auto j=0; j < size(builder.suffix[i]); j++)
      EXPECT_EQ(builder.suffix[i][j], suffix[i][j]) << "different suffixes found at level " << i << "position " << j << " where " << builder.suffix[i][j] << " != " << suffix[i][j];
  }
 
}



TEST(LoudsBuilderTest, ComplexTrie) {
  // using namespace std::string_literals;
  std::stringstream iss;
  iss << "f" << std::endl;
  iss << "farther" << std::endl;
  iss << "fas" << std::endl;
  iss << "fasten" << std::endl;
  iss << "fat" << std::endl;
  iss << "splice" << std::endl;
  iss << "topper" << std::endl;
  iss << "toy" << std::endl;
  iss << "tries" << std::endl;
  iss << "tripper" << std::endl;
  iss << "trying" << std::endl;

  auto n_levels = 4;
  std::vector<std::vector<char>> labels =     {{'f', 's', 't'}, {'\0', 'a', 'o', 'r'}, {'r', 's', 't', 'p', 'y', 'i', 'y'}, {'\0', 't', 'e', 'p'}};
  std::vector<std::vector<bool>> has_child =  {{ 1 ,  0 ,  1 }, { 0 ,  1 ,  1 ,  1 }, { 0 ,  1 ,  0 ,  0 ,  0 ,  1 ,  0 }, { 0 ,  0 ,  0 ,  0 }};
  std::vector<std::vector<bool>> louds =      {{ 1 ,  0 ,  0 }, { 1 ,  0 ,  1 ,  0 }, { 1 ,  0 ,  0 ,  1 ,  0 ,  1 ,  0 }, { 1 ,  0 ,  1 ,  0 }};
  vector<vector<string>> suffix = {{"plice"}, {""}, {"ther", "", "per", "", "ing"}, {"", "en", "s", "per"}};
  auto builder = LoudsBuilder::from_stream(iss);
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
    ASSERT_EQ(size(builder.suffix[i]), size(suffix[i])) << "nested suffix size is different at level " << i;
    for (auto j=0; j < size(builder.suffix[i]); j++)
      EXPECT_EQ(builder.suffix[i][j], suffix[i][j]) << "different suffixes found at level " << i << "position " << j << " where " << builder.suffix[i][j] << " != " << suffix[i][j];

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
  std::vector<char> labels =     {'a', 'a', 'b', 'c', 'a', 'b'};
  std::vector<bool> has_child =  { 1 ,  0,   1,   0 ,  0,   0 };
  std::vector<bool> louds =      { 1 ,  1,   0,   0 ,  1,   0 };
  vector<string> suffix =        { "a", "a", "", ""};

  auto builder = LoudsBuilder::from_stream(iss);
  auto sparse = LoudsSparse::from_builder(builder);
  ASSERT_EQ(builder.n_levels, n_levels) << "Number of levels is wrong, expected " << n_levels;
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

  ASSERT_EQ(size(sparse.values), size(suffix));
  for (auto j=0; j < size(sparse.values); j++)
    EXPECT_EQ(sparse.values[j], suffix[j]) << "different values found at level position " << j; 
  
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
  std::stringstream iss;
  iss << "f" << std::endl;
  iss << "farther" << std::endl;
  iss << "fas" << std::endl;
  iss << "fasten" << std::endl;
  iss << "fat" << std::endl;
  iss << "splice" << std::endl;
  iss << "topper" << std::endl;
  iss << "toy" << std::endl;
  iss << "tries" << std::endl;
  iss << "tripper" << std::endl;
  iss << "trying" << std::endl;

  auto n_levels = 4;
  vector<char> labels =     {'f', 's', 't', '\0', 'a', 'o', 'r', 'r', 's', 't', 'p', 'y', 'i', 'y', '\0', 't', 'e', 'p'};
  vector<bool> has_child =  { 1 ,  0 ,  1 ,  0 ,  1 ,  1 ,  1 ,  0 ,  1 ,  0 ,  0 ,  0 ,  1 ,  0 ,  0 ,  0 ,  0 ,  0 };
  vector<bool> louds =      { 1 ,  0 ,  0 ,  1 ,  0 ,  1 ,  0 ,  1 ,  0 ,  0 ,  1 ,  0 ,  1 ,  0 ,  1 ,  0 ,  1 ,  0 };
  vector<string> suffix =   {"plice", "", "ther", "", "per", "", "ing", "", "en", "s", "per"};

  auto builder = LoudsBuilder::from_stream(iss);
  auto sparse = LoudsSparse::from_builder(builder);
  ASSERT_EQ(builder.n_levels, n_levels) << "Number of levels is wrong, expected " << n_levels;
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

  ASSERT_EQ(size(sparse.values), size(suffix));
  for (auto j=0; j < size(sparse.values); j++)
    EXPECT_EQ(sparse.values[j], suffix[j]) << "different values found at level position " << j; 
  

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
  vector<std::bitset<256>> labels =     {"a"_bs, "abc"_bs, "ab"_bs };
  vector<std::bitset<256>> has_child =  {"a"_bs,  "b"_bs,  ""_bs };
  vector<string> suffix =               { "a", "a", "", ""};
  auto builder = LoudsBuilder::from_stream(iss);
  auto dense = LoudsDense::from_builder(builder);
  ASSERT_EQ(builder.n_levels, n_levels) << "Number of levels is wrong, expected " << n_levels;
  ASSERT_TRUE(eq(size(dense.labels),size(dense.has_child), size(labels))) << "Not all sizes are the same";
  ASSERT_EQ(size(dense.labels), size(labels));
  for (auto j=0; j < size(dense.labels); j++){
    EXPECT_EQ(dense.labels[j], labels[j]) << "different labels element found at position " << j;
  }
  ASSERT_EQ(size(dense.has_child), size(has_child));
  for (auto j=0; j < size(dense.has_child); j++)
    EXPECT_EQ(dense.has_child[j], has_child[j]) << "different labels element found at level position " << j;
  
  ASSERT_EQ(size(dense.values), size(suffix));
  for (auto j=0; j < size(dense.values); j++)
    EXPECT_EQ(dense.values[j], suffix[j]) << "different values found at level position " << j; 

  EXPECT_TRUE(dense.look_up("aaa").found);
  EXPECT_TRUE(dense.look_up("aba").found);
  EXPECT_TRUE(dense.look_up("abb").found);
  EXPECT_TRUE(dense.look_up("aca").found);
  EXPECT_FALSE(dense.look_up("acb").found);
  EXPECT_FALSE(dense.look_up("bca").found);
  EXPECT_FALSE(dense.look_up("bcad").found);
  EXPECT_FALSE(dense.look_up("dcae").found);
}

TEST(LoudsDenseTest, ComplexTrie) {
  std::stringstream iss;
  iss << "f" << std::endl;
  iss << "farther" << std::endl;
  iss << "fas" << std::endl;
  iss << "fasten" << std::endl;
  iss << "fat" << std::endl;
  iss << "splice" << std::endl;
  iss << "topper" << std::endl;
  iss << "toy" << std::endl;
  iss << "tries" << std::endl;
  iss << "tripper" << std::endl;
  iss << "trying" << std::endl;

  auto n_levels = 4;
  vector<std::bitset<256>> labels =     {"fst"_bs, "\0a"_bs, "or"_bs, "rst"_bs, "py"_bs, "iy"_bs, "\0t"_bs, "ep"_bs};
  vector<std::bitset<256>> has_child =  { "ft"_bs,  "a"_bs, "or"_bs ,  "s"_bs ,  ""_bs , "i"_bs ,  ""_bs ,  ""_bs};
  vector<string> suffix =               {"plice", "", "ther", "", "per", "", "ing", "", "en", "s", "per"};

  auto builder = LoudsBuilder::from_stream(iss);
  auto dense = LoudsDense::from_builder(builder);
  ASSERT_EQ(builder.n_levels, n_levels) << "Number of levels is wrong, expected " << n_levels;
  ASSERT_TRUE(eq(size(dense.labels),size(dense.has_child), size(labels))) << "Not all sizes are the same";
  ASSERT_EQ(size(dense.labels), size(labels));
  for (auto j=0; j < size(dense.labels); j++){
    EXPECT_EQ(dense.labels[j], labels[j]) << "different labels element found at position " << j;
  }
  ASSERT_EQ(size(dense.has_child), size(has_child));
  for (auto j=0; j < size(dense.has_child); j++)
    EXPECT_EQ(dense.has_child[j], has_child[j]) << "different labels element found at level position " << j;
  
  ASSERT_EQ(size(dense.values), size(suffix));
  for (auto j=0; j < size(dense.values); j++)
    EXPECT_EQ(dense.values[j], suffix[j]) << "different values found at level position " << j; 

  EXPECT_TRUE(dense.look_up("f").found);
  EXPECT_TRUE(dense.look_up("farther").found);
  EXPECT_TRUE(dense.look_up("fas").found);
  EXPECT_TRUE(dense.look_up("fasten").found);
  EXPECT_TRUE(dense.look_up("fat").found);
  EXPECT_TRUE(dense.look_up("splice").found);
  EXPECT_TRUE(dense.look_up("topper").found);
  EXPECT_TRUE(dense.look_up("toy").found);
  EXPECT_TRUE(dense.look_up("tries").found);
  EXPECT_TRUE(dense.look_up("tripper").found);
  EXPECT_TRUE(dense.look_up("trying").found);
  EXPECT_FALSE(dense.look_up("try").found);
  EXPECT_FALSE(dense.look_up("top").found);
  EXPECT_FALSE(dense.look_up("tor").found);
  EXPECT_FALSE(dense.look_up("spline").found);
}



TEST(SurfTest, PrimitiveTrie) {
  // using namespace std::string_literals;
  std::stringstream iss;
  iss << "aaa" << std::endl;
  iss << "aba" << std::endl;
  iss << "abb" << std::endl;
  iss << "aca" << std::endl;


  auto builder = LoudsBuilder::from_stream(iss);
  auto surf = Surf::from_builder(builder, 2);
  
  EXPECT_TRUE(surf.look_up("aaa"));
  EXPECT_TRUE(surf.look_up("aba"));
  EXPECT_TRUE(surf.look_up("abb"));
  EXPECT_TRUE(surf.look_up("aca"));
  EXPECT_FALSE(surf.look_up("acb"));
  EXPECT_FALSE(surf.look_up("bca"));
  EXPECT_FALSE(surf.look_up("bcad"));
  EXPECT_FALSE(surf.look_up("dcae"));
  
  // surf.range_query("aaa", "abb");
}

TEST(SurfTest, ComplexTrie) {
  // using namespace std::string_literals;
  std::stringstream iss;
  iss << "f" << std::endl;
  iss << "farther" << std::endl;
  iss << "fas" << std::endl;
  iss << "fasten" << std::endl;
  iss << "fat" << std::endl;
  iss << "splice" << std::endl;
  iss << "topper" << std::endl;
  iss << "toy" << std::endl;
  iss << "tries" << std::endl;
  iss << "tripper" << std::endl;
  iss << "trying" << std::endl;


  auto builder = LoudsBuilder::from_stream(iss);
  auto surf = Surf::from_builder(builder, 2);
  
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
  EXPECT_FALSE(surf.look_up("try"));
  EXPECT_FALSE(surf.look_up("top"));
  EXPECT_FALSE(surf.look_up("tor"));
  EXPECT_FALSE(surf.look_up("spline"));

  surf.range_query("f", "trying");
  
}



// TEST(LoudsDenseTest, PrimitiveTrieRange) {
//   // using namespace std::string_literals;
//   std::stringstream iss;
//   iss << "aaa" << std::endl;
//   iss << "aba" << std::endl;
//   iss << "abb" << std::endl;
//   iss << "aca" << std::endl;

  
//   auto builder = LoudsBuilder::from_stream(iss);
//   auto dense = LoudsDense::from_builder(builder);
//   auto get_stats = [&](auto from, auto to){
//     struct stats {size_t count{0}; vector<string> words{};};
//     stats ans{};
//     auto count = 0;
//     for (auto it = dense.begin(from, to); it != dense.end(); ++it){
//       ans.count += 1;
//       ans.words.push_back(*it);
//     }
//     return ans;
//   };
//   auto stats1 = get_stats("aaa", "aba");
//   EXPECT_EQ(stats1.count, 2);
//   for(const auto word : {"aaa", "aba"}){
//     const auto& v = stats1.words;
//     EXPECT_TRUE(find(begin(v), end(v), word) != end(v)) << "Word " << word << " was not found" ;
//   }

//   auto stats2 = get_stats("ab", "aca");
//   EXPECT_EQ(stats2.count, 3);
//   for(const auto word : {"aba", "abb", "aca"}){
//     const auto& v = stats2.words;
//     EXPECT_TRUE(find(begin(v), end(v), word) != end(v)) << "Word " << word << " was not found" ;
//   }
// }

TEST(LoudsSparseTest, PrimitiveTrieRange) {
  // using namespace std::string_literals;
  std::stringstream iss;
  iss << "aaa" << std::endl;
  iss << "aba" << std::endl;
  iss << "abb" << std::endl;
  iss << "aca" << std::endl;

  
  auto builder = LoudsBuilder::from_stream(iss);
  auto sparse = LoudsSparse::from_builder(builder);
  auto _begin = begin(sparse);
  auto _end = end(sparse);

  auto all_keys = [&sparse](){
    auto begin = sparse.begin();
    auto end = sparse.end();
    vector<string> ans{};
    for (auto it = begin; it != end; ++it){
      auto [prefix, suffix] = *it;
      ans.push_back(prefix+suffix);
    }
    return ans;
  };

  auto all_keys_reversed = [&sparse](){
    auto begin = sparse.begin();
    auto end = sparse.end();
    vector<string> ans{};
    for (auto it = end; it != begin; --it){
      auto [prefix, suffix] = *it;
      ans.push_back(prefix+suffix);
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
  std::stringstream iss;
  iss << "f" << std::endl;
  iss << "farther" << std::endl;
  iss << "fas" << std::endl;
  iss << "fasten" << std::endl;
  iss << "fat" << std::endl;
  iss << "splice" << std::endl;
  iss << "topper" << std::endl;
  iss << "toy" << std::endl;
  iss << "tries" << std::endl;
  iss << "tripper" << std::endl;
  iss << "trying" << std::endl;

  
  auto builder = LoudsBuilder::from_stream(iss);
  auto sparse = LoudsSparse::from_builder(builder);

  auto all_keys = [&sparse](){
    auto begin = sparse.begin();
    auto end = sparse.end();
    vector<string> ans{};
    for (auto it = begin; it != end; ++it){
      auto [prefix, suffix] = *it;
      ans.push_back(prefix+suffix);
    }
    return ans;
  };

  auto all_keys_reversed = [&sparse](){
    auto begin = sparse.begin();
    auto end = sparse.end();
    vector<string> ans{};
    for (auto it = end; it != begin; --it){
      auto [prefix, suffix] = *it;
      ans.push_back(prefix+suffix);
    }
    return ans;
  };

  auto all_keys_lb = [&sparse](auto key){
    auto begin = sparse.lb(key);
    auto end = sparse.end();
    vector<string> ans{};
    for (auto it = begin; it != end; ++it){
      auto [prefix, suffix] = *it;
      ans.push_back(prefix+suffix);
    }
    return ans;
  };
  auto all_keys_ub = [&sparse](auto key){
    auto begin = sparse.begin();
    auto end = sparse.ub(key);
    vector<string> ans{};
    for (auto it = begin; it != end; ++it){
      auto [prefix, suffix] = *it;
      ans.push_back(prefix+suffix);
    }
    return ans;
  };
  auto expected_keys = vector<string>{"f", "farther", "fas", "fasten", "fat", "splice", "topper", "toy", "tries", "tripper"};
  auto expected_keys_reversed = vector<string>{"trying", "tripper", "tries", "toy", "topper", "splice", "fat", "fasten", "fas", "farther"};
  auto expected_keys_lb = vector<string>{"fasten", "fat", "splice", "topper", "toy", "tries", "tripper"};
  auto expected_keys_ub = vector<string>{"f", "farther", "fas", "fasten", "fat", "splice", "topper"};
  EXPECT_TRUE(all_keys() == expected_keys) << "Not all keys are as expected";
  EXPECT_TRUE(all_keys_reversed() == expected_keys_reversed) << "Not all reversed keys are as expected";
  EXPECT_TRUE(all_keys_lb("fasten") == expected_keys_lb) << "Not all keys lb are as expected";
  EXPECT_TRUE(all_keys_ub("toy") == expected_keys_ub) << "Not all keys ub are as expected";
}


// TEST(LoudsDenseTest, PrimitiveTrieRange) {
//   std::stringstream iss;
//   iss << "aaa" << std::endl;
//   iss << "aba" << std::endl;
//   iss << "abb" << std::endl;
//   iss << "aca" << std::endl;

  
//   auto builder = LoudsBuilder::from_stream(iss);
//   auto dense = LoudsDense::from_builder(builder);
//   auto _begin = begin(dense);
//   auto _end = end(dense);

//   auto all_keys = [&dense](){
//     auto begin = dense.begin();
//     auto end = dense.end();
//     vector<string> ans{};
//     for (auto it = begin; it != end; ++it){
//       auto [prefix, suffix] = *it;
//       ans.push_back(prefix+suffix);
//     }
//     return ans;
//   };

//   auto all_keys_reversed = [&dense](){
//     auto begin = dense.begin();
//     auto end = dense.end();
//     vector<string> ans{};
//     for (auto it = end; it != begin; --it){
//       auto [prefix, suffix] = *it;
//       ans.push_back(prefix+suffix);
//     }
//     return ans;
//   };
//   auto expected_keys = vector<string>{"aaa", "aba", "abb"};
//   auto expected_keys_reversed = vector<string>{"aca", "abb", "aba"};
//   EXPECT_TRUE(all_keys() == expected_keys);
//   EXPECT_TRUE(all_keys_reversed() == expected_keys_reversed);
// }

TEST(LoudsDenseTest, ComplexTrieRange) {
  // using namespace std::string_literals;
  std::stringstream iss;
  iss << "f" << std::endl;
  iss << "farther" << std::endl;
  iss << "fas" << std::endl;
  iss << "fasten" << std::endl;
  iss << "fat" << std::endl;
  iss << "splice" << std::endl;
  iss << "topper" << std::endl;
  iss << "toy" << std::endl;
  iss << "tries" << std::endl;
  iss << "tripper" << std::endl;
  iss << "trying" << std::endl;

  
  auto builder = LoudsBuilder::from_stream(iss);
  auto dense = LoudsDense::from_builder(builder);
  auto all_keys = [&dense](){
    auto begin = dense.begin();
    auto end = dense.end();
    vector<string> ans{};
    for (auto it = begin; it != end; ++it){
      auto [prefix, suffix] = *it;
      ans.push_back(prefix+suffix);
    }
    return ans;
  };

  auto all_keys_reversed = [&dense](){
    auto begin = dense.begin();
    auto end = dense.end();
    vector<string> ans{};
    for (auto it = end; it != begin; --it){
      auto [prefix, suffix] = *it;
      ans.push_back(prefix+suffix);
    }
    return ans;
  };
  auto all_keys_lb = [&dense](auto key){
    auto begin = dense.lb(key);
    auto end = dense.end();
    vector<string> ans{};
    for (auto it = begin; it != end; ++it){
      auto [prefix, suffix] = *it;
      ans.push_back(prefix+suffix);
    }
    return ans;
  };
  auto all_keys_ub = [&dense](auto key){
    auto begin = dense.begin();
    auto end = dense.ub(key);
    vector<string> ans{};
    for (auto it = begin; it != end; ++it){
      auto [prefix, suffix] = *it;
      ans.push_back(prefix+suffix);
    }
    return ans;
  };
  auto expected_keys = vector<string>{"f", "farther", "fas", "fasten", "fat", "splice", "topper", "toy", "tries", "tripper"};
  auto expected_keys_reversed = vector<string>{"trying", "tripper", "tries", "toy", "topper", "splice", "fat", "fasten", "fas", "farther"};
  auto expected_keys_lb = vector<string>{"fasten", "fat", "splice", "topper", "toy", "tries", "tripper"};
  auto expected_keys_ub = vector<string>{"f", "farther", "fas", "fasten", "fat", "splice", "topper"};
  EXPECT_TRUE(all_keys() == expected_keys);
  EXPECT_TRUE(all_keys_reversed() == expected_keys_reversed);
  EXPECT_TRUE(all_keys_lb("fasten") == expected_keys_lb);
  EXPECT_TRUE(all_keys_ub("toy") == expected_keys_ub);
}
