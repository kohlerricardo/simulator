/**
 * Enumerations File to ORCS, SINUCA like
 * 
*/

#ifndef _ORCS_ENUMERATOR_HPP_
#define _ORCS_ENUMERATOR_HPP_
/**
 * Enum Branch Predictor
*/
enum taken_t{
    NOT_TAKEN = 0,
    TAKEN = 1
};
/**
 * Enum  Hit Miss Situations (Cache,BTB, etc)
*/
enum cache_status_t{
    HIT,
    MISS
};

/**
 * Enum cache Level
*/
enum cacheLevel_t{
    L1,
    L2,
    LLC
};

/// Enumerates the INSTRUCTION (Opcode and Uop) operation type
enum instruction_operation_t {
    /// NOP
    INSTRUCTION_OPERATION_NOP,
    /// INTEGERS
    INSTRUCTION_OPERATION_INT_ALU,
    INSTRUCTION_OPERATION_INT_MUL,
    INSTRUCTION_OPERATION_INT_DIV,
    /// FLOAT POINT
    INSTRUCTION_OPERATION_FP_ALU,
    INSTRUCTION_OPERATION_FP_MUL,
    INSTRUCTION_OPERATION_FP_DIV,
    /// BRANCHES
    INSTRUCTION_OPERATION_BRANCH,
    /// MEMORY OPERATIONS
    INSTRUCTION_OPERATION_MEM_LOAD,
    INSTRUCTION_OPERATION_MEM_STORE,
    /// NOT IDENTIFIED
    INSTRUCTION_OPERATION_OTHER,
    /// SYNCHRONIZATION
    INSTRUCTION_OPERATION_BARRIER,
    /// HMC
    INSTRUCTION_OPERATION_HMC_ROA,     //#12 READ+OP +Answer
    INSTRUCTION_OPERATION_HMC_ROWA     //#13 READ+OP+WRITE +Answer
};
//get char operation
const char* get_enum_instruction_operation_char(instruction_operation_t type);
// ============================================================================
/// Enumerates the types of branches
enum branch_t {
    BRANCH_SYSCALL,
    BRANCH_CALL,
    BRANCH_RETURN,
    BRANCH_UNCOND,
    BRANCH_COND
};
//Status for packages and uOps
enum package_state_t {
    PACKAGE_STATE_FREE,
    PACKAGE_STATE_UNTREATED,
    PACKAGE_STATE_READY,
    PACKAGE_STATE_WAIT,
    PACKAGE_STATE_TRANSMIT
};


#endif