# Chronospatial Computer

A Metal-based GPU implementation of a custom virtual machine that explores the intersection of program execution and self-referential computation. This project demonstrates how GPU parallelization can be used to solve complex computational problem.

## Overview

The Chronospatial Computer implements a simple virtual machine with the following characteristics:

- 4 registers: `ra`, `rb`, `rc`, and `ip` (instruction pointer)
- Fixed-size program memory (up to 16 instructions)
- Output buffer for storing results (up to 128 characters)
- Custom instruction set with 8 operations:
  - `adv`: Arithmetic division
  - `bxl`: Bitwise XOR with immediate
  - `bst`: Set register value
  - `jnz`: Conditional jump
  - `bxc`: Register XOR
  - `out`: Output value
  - `bdv`: Division operation
  - `cdv`: Division operation

## Program Execution

The virtual machine executes programs that:
1. Take an initial value in the `ra` register
2. Apply a series of arithmetic and bitwise operations
3. Generate output as a string of comma-separated values
4. Verify if the output matches a predefined target string

## Technical Implementation

The project is implemented using Apple's Metal framework for GPU computation. Key technical aspects include:

- Metal shader implementation for program execution
- Parallel processing of multiple initial register states
- Efficient string handling and comparison on the GPU
- Atomic operations for result collection

### GPU Role

The GPU is utilized to parallelize the search for valid initial states of the `ra` register. Instead of sequentially testing different values, the implementation:

1. Divides the search space across GPU threads
2. Each thread executes the program with a different initial `ra` value
3. Results are collected using atomic operations
4. The GPU's parallel processing capabilities significantly speed up the search process
