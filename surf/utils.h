#pragma once
#include <algorithm>
#include <iterator>
#include <span>
#include <ranges>
#include <vector>

template<typename... Args>
auto all(Args const & ... args) { return (... && args); }

template<typename... Args>
auto any(Args const & ... args) { return (... || args); }

template<typename A0, typename ... Args>
auto eq(A0 const & a0, Args const & ... args) { return ( (args == a0) && ... && true ); } 

template<typename A0, typename ... Args>
auto neq(A0 const & a0, Args const & ... args) { return ( (args != a0) || ... || false ); } 

template<typename... Args>
auto sum(Args const & ... args) { return (... + args); }

template<typename... Args>
auto prod(Args const & ... args) { return (... * args); }

auto slice(std::ranges::forward_range auto xs, size_t from, size_t to){
  return std::span(std::begin(xs) + from, std::begin(xs) + to);
}

template<typename T>
  concept range_value_string = 
    std::ranges::range<T> && 
    std::same_as<std::ranges::range_value_t<T>, std::string>;


template <class T>
std::vector<T> merge(std::span<std::vector<T>> xs, std::size_t capacity)
{
    std::vector<T> ans;
    ans.reserve(capacity);
    for (const auto &x : xs) {
        ans.insert(std::end(ans), std::begin(x), std::end(x));
    }
    return ans;
}

template <class To, class From>
std::vector<To> merge(std::span<std::vector<From>> xs, std::size_t capacity)
{
    std::vector<To> ans;
    ans.reserve(capacity);
    for (const auto &x : xs) {
        ans.insert(std::end(ans), std::begin(x), std::end(x));
    }
    return ans;
}


auto count(std::vector<bool> const & vs, bool x) -> size_t
{
  return std::count(std::begin(vs), std::end(vs), x);
}

