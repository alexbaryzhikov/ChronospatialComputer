#ifndef Print_hpp
#define Print_hpp

#include <iostream>

template <typename... Ts>
void print(const Ts&... ts) { std::cout << std::boolalpha; (std::cout << ... << ts); }

template <typename... Ts>
void println(const Ts&... ts) { std::cout << std::boolalpha; (std::cout << ... << ts) << std::endl; }

#endif /* Print_hpp */
