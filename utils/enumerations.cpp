#include "../simulator.hpp"

const char* get_enum_instruction_operation_char(instruction_operation_t type) {
    switch (type) {
        // ====================================================================
        /// INTEGERS
        case INSTRUCTION_OPERATION_INT_ALU:     return "OP_IN_ALU"; break;
        case INSTRUCTION_OPERATION_INT_MUL:     return "OP_IN_MUL"; break;
        case INSTRUCTION_OPERATION_INT_DIV:     return "OP_IN_DIV"; break;
        // ====================================================================
        /// FLOAT POINT
        case INSTRUCTION_OPERATION_FP_ALU:      return "OP_FP_ALU"; break;
        case INSTRUCTION_OPERATION_FP_MUL:      return "OP_FP_MUL"; break;
        case INSTRUCTION_OPERATION_FP_DIV:      return "OP_FP_DIV"; break;
        // ====================================================================
        /// BRANCHES
        case INSTRUCTION_OPERATION_BRANCH:      return "OP_BRANCH"; break;
        // ====================================================================
        /// MEMORY OPERATIONS
        case INSTRUCTION_OPERATION_MEM_LOAD:    return "OP_LOAD  "; break;
        case INSTRUCTION_OPERATION_MEM_STORE:   return "OP_STORE "; break;
        // ====================================================================
        /// NOP or NOT IDENTIFIED
        case INSTRUCTION_OPERATION_NOP:         return "OP_NOP   "; break;
        case INSTRUCTION_OPERATION_OTHER:       return "OP_OTHER "; break;
        // ====================================================================
        /// SYNCHRONIZATION
        case INSTRUCTION_OPERATION_BARRIER:     return "OP_BARRIER"; break;
        // ====================================================================
        /// HMC
        case INSTRUCTION_OPERATION_HMC_ROA:     return "HMC_ROA"; break;
        case INSTRUCTION_OPERATION_HMC_ROWA:     return "HMC_ROWA"; break;
    };
    ERROR_PRINTF("Wrong INSTRUCTION_OPERATION\n");
    return "FAIL";
};