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
    return std::shared_ptr<T>(resource, [](T* resource){ resource->release(); });
}

static constexpr size_t PROGRAM_OUTPUT_MAX = 128;

struct GPUHandler {
    std::shared_ptr<MTL::Device> device;
    std::shared_ptr<MTL::CommandQueue> commandQueue;
    std::shared_ptr<MTL::ComputePipelineState> computePipelineState;

    GPUHandler()
    : device(autoRelease(MTL::CreateSystemDefaultDevice()))
    , commandQueue(autoRelease(device->newCommandQueue())) {}

    void buildPipelineState(const char* functionName) {
        NS::Error* error = nullptr;
        auto vectorSumLib = autoRelease(device->newDefaultLibrary());
        auto addVectorsFn = autoRelease(vectorSumLib->newFunction(NS::String::string(functionName, NS::UTF8StringEncoding)));
        computePipelineState = autoRelease(device->newComputePipelineState(addVectorsFn.get(), &error), &error);
    }
    
    std::string part1() {
        auto pool = autoRelease(NS::AutoreleasePool::alloc()->init());

        buildPipelineState("part1");
        
        auto outputBuffer = device->newBuffer(PROGRAM_OUTPUT_MAX, MTL::ResourceStorageModeShared);

        auto commandBuffer = commandQueue->commandBuffer();
        auto commandEncoder = commandBuffer->computeCommandEncoder();
        commandEncoder->setComputePipelineState(computePipelineState.get());
        commandEncoder->setBuffer(outputBuffer, 0, 0);

        MTL::Size threadsPerGrid(1, 1, 1);
        MTL::Size threadsPerThreadgroup(1, 1, 1);
        commandEncoder->dispatchThreads(threadsPerGrid, threadsPerThreadgroup);
        commandEncoder->endEncoding();
        commandBuffer->commit();
        commandBuffer->waitUntilCompleted();

        return std::string(static_cast<char*>(outputBuffer->contents()));
    }

    std::vector<int64_t> part2() {
        auto pool = autoRelease(NS::AutoreleasePool::alloc()->init());
        
        buildPipelineState("part2");
        
        auto outputBuffer = device->newBuffer(sizeof(int64_t) * 32, MTL::ResourceStorageModeShared);
        auto outputSizeBuffer = device->newBuffer(sizeof(uint), MTL::ResourceStorageModeShared);
        
        auto commandBuffer = commandQueue->commandBuffer();
        auto commandEncoder = commandBuffer->computeCommandEncoder();
        commandEncoder->setComputePipelineState(computePipelineState.get());
        commandEncoder->setBuffer(outputBuffer, 0, 0);
        commandEncoder->setBuffer(outputSizeBuffer, 0, 1);
        
        MTL::Size threadsPerGrid(10000, 10000, 1);
        MTL::Size threadsPerThreadgroup(32, 32, 1);
        commandEncoder->dispatchThreads(threadsPerGrid, threadsPerThreadgroup);
        commandEncoder->endEncoding();
        commandBuffer->commit();
        commandBuffer->waitUntilCompleted();
        
        std::vector<int64_t> result;
        auto out = static_cast<int64_t*>(outputBuffer->contents());
        uint outSize = static_cast<uint*>(outputSizeBuffer->contents())[0];
        for (size_t i = 0; i < outSize; ++i) {
            result.push_back(out[i]);
        }
        return result;
    }
};

int main(int argc, const char* argv[]) {
    auto pool = autoRelease(NS::AutoreleasePool::alloc()->init());
    GPUHandler handler = GPUHandler();
    println("Part1: ", handler.part1());
    println("Part2: ", handler.part2());
}
