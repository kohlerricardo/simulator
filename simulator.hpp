/// This is the main header file, it contains all the prototypes and
/// describes the relations between classes.
#ifndef _ORCS_ORCS_HPP_
#define _ORCS_ORCS_HPP_

/// C Includes
#include <unistd.h>     /* for getopt */
#include <getopt.h>     /* for getopt_long; POSIX standard getopt is in unistd.h */
#include <inttypes.h>   /* for uint32_t */
#include <zlib.h>

/// C++ Includes
//facilities
#include <iostream>
#include <iomanip>
//original
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>




// ============================================================================
/// Classes
// ============================================================================

class orcs_engine_t;
class trace_reader_t;
class opcode_package_t;


///////////////////////////////////////////
// Branch Predictor Classes
///////////////////////////////////////////
class btb_line_t;
class btb_t;
class twoBit_t;
// class piecewise_t;
///////////////////////////////////////////
// Out of order Execution classes
///////////////////////////////////////////
class uop_package_t;
class processor_t;
///////////////////////////////////////////
// Cache Classes
///////////////////////////////////////////

// ============
// CACHE CLASSES
// class linha_t;
// class cacheSet_t;
// class cache_t;
// class utils_t;

///////////////////////////////////////////
// Usefull Classes
///////////////////////////////////////////
template<class CB_TYPE> class circular_buffer_t;
// class utils_t;




// ============================================================================
/// Global SINUCA_ENGINE instantiation
// ============================================================================
extern orcs_engine_t orcs_engine;


// ============================================================================
/// Definitions for Log, Debug, Warning, Error and Statistics
// ============================================================================
#define POSITION_FAIL -1        /// FAIL when return is int32_t
#define FAIL 0                  /// FAIL when return is int32_t or uint32_t
#define OK 1                    /// OK when return is int32_t or uint32_t

#define TRACE_LINE_SIZE 512

#define MAX_REGISTERS 6         /// opcode_package_t uop_package_t  (Max number of register (read or write) for one opcode/uop)
#define MAX_ASSEMBLY_SIZE 32    /// In general 20 is enough




/// DETAIL DESCRIPTION: Almost all errors and messages use this definition.
/// It will DEACTIVATE all the other messages below
#define ORCS_PRINTF(...) printf(__VA_ARGS__);

// ~ #define ORCS_DEBUG
#ifdef ORCS_DEBUG
    #define DEBUG_PRINTF(...) {\
                                  ORCS_PRINTF("DEBUG: ");\
                                  ORCS_PRINTF(__VA_ARGS__);\
                              }
#else
    #define DEBUG_PRINTF(...)
#endif



#define ERROR_INFORMATION() {\
                                ORCS_PRINTF("ERROR INFORMATION\n");\
                                ORCS_PRINTF("ERROR: File: %s at Line: %u\n", __FILE__, __LINE__);\
                                ORCS_PRINTF("ERROR: Function: %s\n", __PRETTY_FUNCTION__);\
                                ORCS_PRINTF("ERROR: Cycle: %" PRIu64 "\n", orcs_engine.get_global_cycle());\
                            }


#define ERROR_ASSERT_PRINTF(v, ...) if (!(v)) {\
                                        ERROR_INFORMATION();\
                                        ORCS_PRINTF("ERROR_ASSERT: %s\n", #v);\
                                        ORCS_PRINTF("\nERROR: ");\
                                        ORCS_PRINTF(__VA_ARGS__);\
                                        exit(EXIT_FAILURE);\
                                    }

#define ERROR_PRINTF(...) {\
                              ERROR_INFORMATION();\
                              ORCS_PRINTF("\nERROR: ");\
                              ORCS_PRINTF(__VA_ARGS__);\
                              exit(EXIT_FAILURE);\
                          }


// ==============================================================================
/// Enumerations
// ==============================================================================

// ============================================================================
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

// ============================================================================
/// Enumerates the types of branches
enum branch_t {
    BRANCH_SYSCALL,
    BRANCH_CALL,
    BRANCH_RETURN,
    BRANCH_UNCOND,
    BRANCH_COND
};


// ============================================================================
/// Base Includes
// ============================================================================
#include "./model.hpp"
#include "./simulator.hpp"
#include "./orcs_engine.hpp"
#include "./trace_reader.hpp"
#include "./package/opcode_package.hpp"

///////////////////////////////////////////
// Usefull Classes
///////////////////////////////////////////
#include "./utils/circular_buffer.hpp"
// #include "./utils/utils.hpp"

///////////////////////////////////////////
// Core Includes
///////////////////////////////////////////
#include "./package/uop_package.hpp"
#include "./processor/processor.hpp"
///////////////////////////////////////////
// Branch Predictor includes
///////////////////////////////////////////
#include "./branch_predictor/btb_line.hpp"
#include "./branch_predictor/btb.hpp"
#include "./branch_predictor/twoBit.hpp"
// #include "./branch_predictor/piecewise.hpp"
///////////////////////////////////////////
// Cache Classes
///////////////////////////////////////////
// // CACHE INCLUDES
// #include "./cache.hpp"
// #include "./cacheSet.hpp"
// #include "./linha.hpp"
// #include "./utils.hpp"




#endif  // _ORCS_ORCS_HPP_
