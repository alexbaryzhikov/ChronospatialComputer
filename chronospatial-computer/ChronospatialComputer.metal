#include <metal_stdlib>
using namespace metal;

static constexpr constant size_t PROGRAM_SIZE_MAX = 16;
static constexpr constant size_t PROGRAM_OUTPUT_MAX = 128;

enum Opcode {
    adv, // 0
    bxl, // 1
    bst, // 2
    jnz, // 3
    bxc, // 4
    out, // 5
    bdv, // 6
    cdv, // 7
};

struct Instruction {
    Opcode opcode;
    int operand;
};

struct Input {
    int64_t ra;
    Instruction program[PROGRAM_SIZE_MAX];
    size_t programSize = 0;
    char programStr[PROGRAM_OUTPUT_MAX];
    size_t programStrSize = 0;
};

size_t intToStr(int64_t value, thread char* str) {
    size_t i = 0;
    if (value < 0) {
        str[i++] = '-';
        value = -value;
    }
    char temp[32];
    size_t j = 0;
    do {
        temp[j++] = '0' + (value % 10);
    } while (value /= 10);
    while (j) str[i++] = temp[--j];
    str[i] = '\0';
    return i;
}

size_t strCopy(const thread char* from, device char* to) {
    size_t i = 0;
    for (; from[i] != '\0'; ++i) {
        to[i] = from[i];
    }
    to[i] = '\0';
    return i;
}

bool strEqual(const constant char* a, size_t sizeA, const thread char* b, size_t sizeB) {
    if (sizeA != sizeB) return false;
    for (size_t i = 0; i < sizeA; ++i) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

struct Computer {
    int64_t ra = 0;
    int64_t rb = 0;
    int64_t rc = 0;
    int64_t ip = 0;
    char output[PROGRAM_OUTPUT_MAX];
    size_t outputSize = 0;
    
    int64_t resolveOperand(int x) {
        switch (x) {
            case 0: return 0;
            case 1: return 1;
            case 2: return 2;
            case 3: return 3;
            case 4: return ra;
            case 5: return rb;
            case 6: return rc;
            default: return -1;
        }
    }

    void exec_adv(int x) { ra /= 1 << resolveOperand(x); }

    void exec_bxl(int x) { rb ^= x; }

    void exec_bst(int x) { rb = resolveOperand(x) % 8; }

    void exec_jnz(int x) { if (ra) ip = x / 2 - 1; }

    void exec_bxc(int x) { rb ^= rc; }

    void exec_out(int x) {
        if (outputSize) {
            output[outputSize++] = ',';
        }
        int64_t val = resolveOperand(x) % 8;
        outputSize += intToStr(val, output + outputSize);
    }

    void exec_bdv(int x) { rb = ra / (1 << resolveOperand(x)); }

    void exec_cdv(int x) { rc = ra / (1 << resolveOperand(x)); }

    void execInstruction(Opcode opcode, int operand) {
        switch (opcode) {
            case adv: exec_adv(operand); break;
            case bxl: exec_bxl(operand); break;
            case bst: exec_bst(operand); break;
            case jnz: exec_jnz(operand); break;
            case bxc: exec_bxc(operand); break;
            case out: exec_out(operand); break;
            case bdv: exec_bdv(operand); break;
            case cdv: exec_cdv(operand); break;
        }
        ++ip;
    }
    
    void exec(const constant Instruction* program, size_t size) {
        while (ip < static_cast<int64_t>(size)) {
            Instruction instruction = program[ip];
            execInstruction(instruction.opcode, instruction.operand);
        }
    }
};

static const constant Input input1 = {
    .ra = 117440,
    .program = {
        {adv, 3},
        {out, 4},
        {jnz, 0}
    },
    .programSize = 3,
    .programStr = "0,3,5,4,3,0",
    .programStrSize = 11
};

static const constant Input input2 = {
    .ra = 37293246,
    .program = {
        {bst, 4},
        {bxl, 6},
        {cdv, 5},
        {bxc, 4},
        {bxl, 7},
        {adv, 3},
        {out, 5},
        {jnz, 0},
    },
    .programSize = 8,
    .programStr = "2,4,1,6,7,5,4,4,1,7,0,3,5,5,3,0",
    .programStrSize = 31
};

void exec(thread Computer& computer, const constant Input& input) {
    computer.ra = input.ra;
    computer.exec(input.program, input.programSize);
}

kernel void part1(device char* output [[buffer(0)]],
                  uint id [[ thread_position_in_grid ]]) {
    Computer computer;
    exec(computer, input2);
    strCopy(computer.output, output);
}

bool validOutput(thread Computer& computer, const constant Input& input) {
    computer.exec(input.program, input.programSize);
    return strEqual(input.programStr, input.programStrSize, computer.output, computer.outputSize);
}

kernel void part2(device int64_t* output [[buffer(0)]],
                  device atomic_uint *outputSize [[buffer(1)]],
                  uint2 grid [[ threads_per_grid ]],
                  uint2 pos [[ thread_position_in_grid ]]) {
    Computer computer;
    int64_t ra = static_cast<int64_t>(pos[0]) * grid[1] + pos[1];
    computer.ra = ra ;
    if (validOutput(computer, input2)) {
        size_t i = atomic_fetch_add_explicit(outputSize, 1, memory_order_relaxed);
        output[i] = ra;
    }
}
