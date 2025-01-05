#include <cassert>
#include <simd/simd.h>

#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include "Metal/Metal.hpp"
#include <VectorIO.hpp>
#include <Print.hpp>
#include <TypeName.hpp>

template<typename T>
std::shared_ptr<T> autoRelease(T* resource, NS::Error** error = nullptr) {
    if (!resource) {
        print("Couldn't create ", typeName<T>().get());
        if (error && *error) {
            println(": ", (*error)->localizedDescription()->utf8String());
        } else {
            println();
        }
        assert(false);
    }
    println("Created ", typeName<T>().get(), " at ", resource);
    return std::shared_ptr<T>(resource, [](T* resource) {
        resource->release();
        println("Released ", typeName<T>().get(), " at ", resource);
    });
}

struct GPUHandler {
    std::shared_ptr<MTL::Device> device;
    std::shared_ptr<MTL::CommandQueue> commandQueue;
    std::shared_ptr<MTL::ComputePipelineState> computePipelineState;

    GPUHandler()
    : device(autoRelease(MTL::CreateSystemDefaultDevice()))
    , commandQueue(autoRelease(device->newCommandQueue())) {
        NS::Error* error = nullptr;
        auto vectorSumLib = autoRelease(device->newDefaultLibrary());
        auto addVectorsFn = autoRelease(vectorSumLib->newFunction(NS::String::string("addVectors", NS::UTF8StringEncoding)));
        computePipelineState = autoRelease(device->newComputePipelineState(addVectorsFn.get(), &error), &error);
    }
    
    std::vector<float> vectorSum(const std::vector<float>& a, const std::vector<float>& b) {
        auto pool = autoRelease(NS::AutoreleasePool::alloc()->init());

        size_t vectorSize = a.size();
        size_t bufferSize = vectorSize * sizeof(float);
        auto bufferA = device->newBuffer(a.data(), bufferSize, MTL::ResourceStorageModeShared);
        auto bufferB = device->newBuffer(b.data(), bufferSize, MTL::ResourceStorageModeShared);
        auto bufferResult = device->newBuffer(bufferSize, MTL::ResourceStorageModeShared);

        auto commandBuffer = commandQueue->commandBuffer();
        auto commandEncoder = commandBuffer->computeCommandEncoder();
        commandEncoder->setComputePipelineState(computePipelineState.get());
        commandEncoder->setBuffer(bufferA, 0, 0);
        commandEncoder->setBuffer(bufferB, 0, 1);
        commandEncoder->setBuffer(bufferResult, 0, 2);

        MTL::Size threadsPerGrid(vectorSize, 1, 1);
        MTL::Size threadsPerThreadgroup(computePipelineState->maxTotalThreadsPerThreadgroup(), 1, 1);
        commandEncoder->dispatchThreads(threadsPerGrid, threadsPerThreadgroup);
        commandEncoder->endEncoding();
        commandBuffer->commit();
        commandBuffer->waitUntilCompleted();

        std::vector<float> result(vectorSize);
        std::memcpy(result.data(), bufferResult->contents(), bufferResult->length());
        return result;
    }
};

void compute() {
    GPUHandler gpuHandler;
    std::vector<float> a = {rand() / float(RAND_MAX), rand() / float(RAND_MAX), rand() / float(RAND_MAX)};
    std::vector<float> b = {rand() / float(RAND_MAX), rand() / float(RAND_MAX), rand() / float(RAND_MAX)};
    std::vector<float> result = gpuHandler.vectorSum(a, b);
    println("Result: ", result);
}

int main(int argc, const char* argv[]) {
    auto pool = autoRelease(NS::AutoreleasePool::alloc()->init());
    compute();
}
