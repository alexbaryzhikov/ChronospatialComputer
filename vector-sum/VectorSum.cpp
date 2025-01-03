#include <cassert>
#include <simd/simd.h>

#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include "Metal/Metal.hpp"
#include <VectorIO.hpp>
#include <Print.hpp>

struct ComputationHandler {
    MTL::Device* device;
    MTL::CommandQueue* commandQueue;
    MTL::ComputePipelineState* computePipelineState;
    MTL::Buffer* bufferA;
    MTL::Buffer* bufferB;
    MTL::Buffer* bufferResult;

    void init() {
        device = MTL::CreateSystemDefaultDevice();
        commandQueue = device->newCommandQueue();
        buildComputePipelineState();
    }
    
    void release() {
        computePipelineState->release();
        commandQueue->release();
        device->release();
    }

    void buildComputePipelineState() {
        NS::Error* error = nullptr;
        MTL::Library* vectorSumLib = device->newDefaultLibrary();
        if (!vectorSumLib) {
            println(error->localizedDescription()->utf8String());
            assert(false);
        }
        MTL::Function* addVectorsFn = vectorSumLib->newFunction(NS::String::string("addVectors", NS::UTF8StringEncoding));
        computePipelineState = device->newComputePipelineState(addVectorsFn, &error);
        if (!computePipelineState) {
            println(error->localizedDescription()->utf8String());
            assert(false);
        }
        addVectorsFn->release();
        vectorSumLib->release();
    }
    
    std::vector<float> vectorSum(const std::vector<float>& a, const std::vector<float>& b) {
        size_t vectorSize = a.size();
        size_t bufferSize = vectorSize * sizeof(float);

        bufferA = device->newBuffer(a.data(), bufferSize, MTL::ResourceStorageModeShared);
        bufferB = device->newBuffer(b.data(), bufferSize, MTL::ResourceStorageModeShared);
        bufferResult = device->newBuffer(bufferSize, MTL::ResourceStorageModeShared);

        MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
        assert(commandBuffer);
        MTL::ComputeCommandEncoder* commandEncoder = commandBuffer->computeCommandEncoder();
        commandEncoder->setComputePipelineState(computePipelineState);
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

        bufferA->release();
        bufferB->release();
        bufferResult->release();

        return result;
    }
};

int main(int argc, const char* argv[]) {
    NS::AutoreleasePool* pAutoreleasePool = NS::AutoreleasePool::alloc()->init();
    ComputationHandler handler;
    handler.init();
    auto result = handler.vectorSum({0, 1, 2, 3, 4}, {5, 6, 7, 8, 9});
    println("Result: ", result);
    handler.release();
    pAutoreleasePool->release();
    return 0;
}
