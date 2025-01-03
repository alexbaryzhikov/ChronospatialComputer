#ifndef VectorIO_hpp
#define VectorIO_hpp

#include <iostream>

template <typename T>
std::istream& operator>>(std::istream& is, std::vector<T>& v) {
    for (T t; is >> t;)
        v.push_back(std::move(t));
    return is;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) os << ' ';
        os << v[i];
    }
    return os;
}

#endif /* VectorIO_hpp */
