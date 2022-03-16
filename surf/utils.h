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

template <size_t B>
class Block{
    public:
    enum { n_chars = B / __CHAR_BIT__, n_bytes = B / sizeof(char), n_bits_char = __CHAR_BIT__};
    char values[n_chars] = {};
    bool operator[](size_t pos){
        return (values[pos/n_bits_char] >> pos%n_bits_char) & 0x1;
    }
    void set(size_t pos){
        values[pos/n_bits_char] |= (1 << pos%n_bits_char);
    }
    void clear(size_t pos){
        values[pos/n_bits_char] &= ~(1 << pos%n_bits_char);
    }
};

template<class B>
class BitVector{
    public:
    enum { n_bits_block = sizeof(B)*__CHAR_BIT__};
    B *v;
    size_t n;
    explicit BitVector(size_t n): v{new B[n]}, n{n} {}
    ~BitVector(){
        delete v;
    }
    bool operator [](size_t pos){
        return v[pos/sizeof(B)][pos%sizeof(B)];
    }
    void set(size_t pos){
        v[pos/sizeof(B)].set(pos%sizeof(B));
    }
    void clear(size_t pos){
        v[pos/sizeof(B)].clear(pos%sizeof(B));
    }
};

void set_pos(BitVector<Block<512>> bit_vector, size_t pos){
    bit_vector.set(pos);
}