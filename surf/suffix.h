#pragma once
#include "bloom/bloom.h"
#include "builder.h"
#include <cstdint>
#include <cstring>
#include <span>
#include <string_view>
#include <hash_fun.h>

namespace kind {
    using namespace std;
    template<size_t Size = sizeof(std::size_t) * 8> // assume a byte is 8 bits in the machine
    struct Hash {
        enum : size_t {size = Size};
        enum traits : std::size_t { max_n_bits = sizeof(std::size_t) * 8 };
        uint64_t value;
        Hash(std::string const & suffix): value(std::hash<std::string>{}(suffix) << (traits::max_n_bits - Size)){}
        Hash(std::string && suffix): value(std::hash<std::string>{}(suffix) << (traits::max_n_bits - Size)){}
        Hash(std::string_view const & suffix): value(std::hash<std::string_view>{}(suffix) << (traits::max_n_bits - Size)){}
        Hash(std::string_view && suffix): value(std::hash<std::string_view>{}(suffix) << (traits::max_n_bits - Size)){}
        auto operator<=>(const Hash&) const = default;
    };
    struct Suffix : public std::string {
        enum : size_t {size = 0};
        Suffix(string_view suffix): string(suffix.data(), suffix.size()) {}
    };
    template<size_t Size = sizeof(std::size_t) * 8>
    struct Real {
        enum : size_t {size = Size};
        enum traits : std::size_t { max_n_bits = sizeof(std::size_t) * 8 };
        uint64_t value;
        static auto construct(std::string_view suffix) -> uint64_t {
            std::array<uint8_t, 8> arr{};
            uint64_t ans;
            std::memcpy(arr.data(), suffix.data(), Size/8 + ((Size%8) != 0U ? 1 : 0));
            arr[Size/8] = arr[Size/8] << (8 - Size%8);
            std::memcpy(&ans, arr.data(), arr.size());
            return ans;
        }
        Real(std::string const & suffix): value(construct(suffix)){}
        Real(std::string && suffix): value(construct(suffix)){}
        Real(std::string_view && suffix): value(construct(suffix)){}
        Real(std::string_view const & suffix): value(construct(suffix)){}
        auto operator<=>(const Real&) const = default;
    };

    template<size_t HashSize = sizeof(std::size_t) * 4, size_t LenSize = sizeof(std::size_t) * 4>
    struct HashLen {
        enum : size_t {size = HashSize + LenSize};
        enum traits : std::size_t { max_n_bits = sizeof(std::size_t) * 8 };
        uint64_t value;
        static auto construct(std::string_view suffix) -> uint64_t {
            std::array<uint8_t, 8> arr{};
            auto hash = std::hash<std::string_view>{}(suffix) << (64-HashSize);
            auto size = ((suffix.size() << (64 - LenSize)) >> HashSize);
            return hash | size;
        }
        HashLen(std::string const & suffix): value(construct(suffix)){}
        HashLen(std::string && suffix): value(construct(suffix)){}
        HashLen(std::string_view && suffix): value(construct(suffix)){}
        HashLen(std::string_view const & suffix): value(construct(suffix)){}
        auto operator<=>(const HashLen&) const = default;
    };
}

namespace suffix {
    using namespace std;
    template<class Kind>
    struct SuffixArray{
        vector<Kind> suffixes{};
        enum : size_t {size = Kind::size};
        auto contains(size_t idx, const string_view suffix) -> bool {
            return  Kind{suffix} == suffixes[idx];
        }
        auto size_bytes() -> size_t {
            return (suffixes.size() * Kind::size) / 8 ;
        }
        explicit SuffixArray(vector<string> const & suffixes) {
            for(const auto & suffix : suffixes){
                this->suffixes.push_back(Kind{suffix});
            }
        }
    };
    template <size_t BitsKey>
    struct SuffixBloom{
        BloomFilter filter;
        string data;
        enum : size_t {size = BitsKey};
        SuffixBloom(vector<string> const & keys): filter{BitsKey} {
            filter.CreateFilter(keys, keys.size(), &data);
        }
        SuffixBloom(span<string> const & keys): filter{BitsKey} {
            filter.CreateFilter(keys, keys.size(), &data);
        }
        auto contains(size_t, const string_view suffix) -> bool {
            return filter.KeyMayMatch(suffix, data);
        }
        auto size_bytes() -> size_t {
            return data.size() + sizeof(BloomFilter);
        }

    };
    template <size_t BitsKey, size_t Super>
    struct SuffixSuperBloom{
        BloomFilter filter;
        vector<string> datas;
        enum : size_t {size = BitsKey};
        SuffixSuperBloom(vector<string> const & keys): filter{BitsKey} {
            size_t blocks = keys.size() / Super;
            size_t n = keys.size();
            for (size_t i = 0; i < blocks; ++i){
                datas.push_back({});
                auto * first = keys.data() + i*Super;
                auto * last = first + Super;
                span<string const> block_keys{first, last};
                filter.CreateFilter(block_keys, block_keys.size(), &datas.back());
            }
            if (n % Super != 0){
                auto * first = keys.data() + ((n/Super) * Super);
                auto * last = keys.data() + keys.size();
                datas.push_back({});
                span<string const> block_keys{first, last};
                filter.CreateFilter(block_keys, block_keys.size(), &datas.back());
            }
        }
        SuffixSuperBloom(span<string const> & keys): filter{BitsKey} {
            size_t blocks = keys.size() / Super;
            size_t n = keys.size();
            for (size_t i = 0; i < blocks; ++i){
                datas.emplace_back();
                auto * first = keys.data() + i*Super;
                auto * last = first + Super;
                span<string const> block_keys{first, last};
                filter.CreateFilter(block_keys, block_keys.size(), &datas.back());
            }
            if (n % Super != 0){
                auto * first = keys.data() + ((n/Super) * Super);
                auto * last = keys.data() + keys.size();
                datas.emplace_back();
                span<string const> block_keys{first, last};
                filter.CreateFilter(block_keys, block_keys.size(), &datas.back());
            }
        }
        auto contains(size_t i, string_view suffix) -> bool {
            size_t n = Super;
            string const & data = datas[i/n];
            return filter.KeyMayMatch(suffix, data);
        }
        auto size_bytes() -> size_t {
            size_t bytes = 0UL;
            for (string & data : datas){
                bytes += data.size();
            }
            return bytes + sizeof(BloomFilter);
        }

    };
}
