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

