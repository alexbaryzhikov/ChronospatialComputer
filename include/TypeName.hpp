#ifndef TypeName_hpp
#define TypeName_hpp

#include <cxxabi.h>

template<typename T>
std::unique_ptr<char, void(*)(void*)> typeName() {
    int status = 0;
    const char* mangled_name = typeid(T).name();
    char* demangled_name = abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status);
    return std::unique_ptr<char, void(*)(void*)>(demangled_name, std::free);
}

#endif /* TypeName_hpp */
