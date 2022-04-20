#pragma once
#include "builder.h"
#include <span>
#include <string_view>
#include <hash_fun.h>

namespace suffix {
    template<size_t Size = sizeof(std::size_t) * 8> // assume a byte is 8 bits in the machine
    struct Hash {
        enum traits : std::size_t { max_n_bits = sizeof(std::size_t) * 8 };
        std::size_t value;
        Hash(std::string const & suffix): value(std::hash<std::string>{}(suffix) << (traits::max_n_bits - Size)){}
        Hash(std::string && suffix): value(std::hash<std::string>{}(suffix) << (traits::max_n_bits - Size)){}
        Hash(std::string_view const & suffix): value(std::hash<std::string_view>{}(suffix) << (traits::max_n_bits - Size)){}
        Hash(std::string_view && suffix): value(std::hash<std::string_view>{}(suffix) << (traits::max_n_bits - Size)){}
        auto operator<=>(const Hash&) const = default;
    };
    struct Suffix : public std::string {
        Suffix(auto && suffix): std::string(std::forward<decltype(suffix)>(suffix)){}
    };
}
