#include <metal_stdlib>
using namespace metal;

kernel void addVectors(device float *inputA [[ buffer(0) ]],
                       device float *inputB [[ buffer(1) ]],
                       device float *result [[ buffer(2) ]],
                       uint id [[ thread_position_in_grid ]]) {
    result[id] = inputA[id] + inputB[id];
}
