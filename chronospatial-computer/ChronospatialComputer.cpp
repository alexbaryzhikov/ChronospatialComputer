#include <cassert>
#include <simd/simd.h>

#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include "Metal/Metal.hpp"
#include <Print.hpp>

int main(int argc, const char* argv[]) {
    NS::AutoreleasePool* pAutoreleasePool = NS::AutoreleasePool::alloc()->init();
    pAutoreleasePool->release();
    return 0;
}
