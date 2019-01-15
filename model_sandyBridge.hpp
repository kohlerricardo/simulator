
/**
Arquivo definindo os parametros do modelo de processador
*/

#ifndef _SANDYBRIDGE_HPP_
#define _SANDYBRIDGE_HPP_
// =========== PROCESSOR =============
#define FETCH_WIDTH 6
#define DECODE_WIDTH 5
#define RENAME_WIDTH 5
#define DISPATCH_WIDTH 6
#define EXECUTE_WIDTH 8
#define COMMIT_WIDTH 4
// =============================
// PROCESSOR LATENCIES STAGES
#define FETCH_LATENCY 3
#define DECODE_LATENCY 3
#define RENAME_LATENCY 3
#define DISPATCH_LATENCY 2
#define EXECUTE_LATENCY 0
#define COMMIT_LATENCY 3
// ========= FUNCTIONAL UNITS RELATED=========

// INTEGER ALU
#define LATENCY_INTEGER_ALU 1
#define WAIT_NEXT_INT_ALU 1
#define INTEGER_ALU 3
// INTEGER MUL
#define LATENCY_INTEGER_MUL 3
#define WAIT_NEXT_INT_MUL 1
#define INTEGER_MUL 1
// INTEGER DIV
#define LATENCY_INTEGER_DIV 32
#define WAIT_NEXT_INT_DIV 32
#define INTEGER_DIV 1

#define QTDE_INTEGER_FU (INTEGER_ALU+INTEGER_MUL+INTEGER_DIV)

//FP ULAS LATENCY 
// FLOATING POINT DIV
#define LATENCY_FP_DIV 10
#define WAIT_NEXT_FP_DIV 10
#define FP_DIV 1
// FLOATING POINT MUL
#define LATENCY_FP_MUL 5
#define WAIT_NEXT_FP_MUL 1
#define FP_MUL 1
// FLOATING POINT ALU
#define LATENCY_FP_ALU 3
#define WAIT_NEXT_FP_ALU 1
#define FP_ALU 1

#define QTDE_FP_FU (FP_ALU+FP_MUL+FP_DIV)
// =====================

// =====================
// MEMORY FU
// =====================
// Load Units
#define LOAD_UNIT 2
#define WAIT_NEXT_MEM_LOAD 1
#define LATENCY_MEM_LOAD 1
// Store Units
#define STORE_UNIT 1
#define WAIT_NEXT_MEM_STORE 1
#define LATENCY_MEM_STORE 1

#define QTDE_MEMORY_FU (LOAD_UNIT+STORE_UNIT)
// Parallel Loads
#define PARALLEL_LOADS 2
#define PARALLEL_STORES 1

// ======================
///UNIFIED FUS

// PROCESSOR BUFFERS SIZE
#define FETCH_BUFFER 18
#define DECODE_BUFFER 28
#define RAT_SIZE 260
#define ROB_SIZE 168
#define UNIFIED_RS 54
//MOB
#define MOB_READ 64
#define MOB_WRITE 36


// ===========BRANCH PREDICTOR=============
#define BTB_ENTRIES 4096
#define BTB_WAYS 4
//COUNTERS
#define TWO_BIT 0
#define PIECEWISE 1 


#define BTB_MISS_PENALITY 5
#define MISSPREDICTION_PENALITY 10
#define N 128
#define M 128
#define H 43
// #define N 64
// #define M 32
// #define H 15
#define  THETA ((2.14*(H)) + 20.58)
// ===========END BRANCH PREDICTOR=============
// *************** DEFINES CACHE *****************

#define KILO 1024
#define MEGA KILO*KILO


// =====================CACHES=======================
// ATTR COMMON
#define LINE_SIZE 64
#define CACHE_LEVELS 2
#define INSTRUCTION_ENABLED 1
#define OFFSET_SIZE 6
// ==================== LEVEL 1 =====================
//https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-optimization-manual.pdf
//Valores retirados do manual de otimização
// D$
#define L1_DATA_SIZE 32*KILO
#define L1_DATA_ASSOCIATIVITY 8
#define L1_DATA_LATENCY 3
#define L1_DATA_SETS (L1_DATA_SIZE/LINE_SIZE)/L1_DATA_ASSOCIATIVITY
// I$
#define L1_INST_SIZE 32*KILO
#define L1_INST_ASSOCIATIVITY 8
#define L1_INST_LATENCY 3
#define L1_INST_SETS (L1_INST_SIZE/LINE_SIZE)/L1_INST_ASSOCIATIVITY
// ==================== LEVEL 1 =====================
// ==================== LEVEL 2 =====================
#define L2_SIZE 256*KILO
#define L2_ASSOCIATIVITY 4
#define L2_LATENCY 12
#define L2_SETS (L2_SIZE/LINE_SIZE)/L2_ASSOCIATIVITY
// ==================== LEVEL 2 =====================
// ==================== LLC     =====================
#define LLC_SIZE 128*KILO
#define LLC_ASSOCIATIVITY 8
#define LLC_LATENCY 12
#define LLC_SETS (LLC_SIZE/LINE_SIZE)/LLC_ASSOCIATIVITY
// ==================== LLC     =====================
// =====================CACHES=======================

// =====================RAM=======================
#define RAM_LATENCY 400
#define RAM_SIZE 4 * MEGA * KILO
#define PARALLEL_LIM_ACTIVE 1
#define MAX_PARALLEL_REQUESTS 4
// =====================RAM=======================
// =====================PREFETCHER=======================
#define PREFETCHER_ACTIVE 1
#define STRIDE_TABLE_SIZE 16   
#define DEGREE 1
#define DISTANCE 4
// active prefetchers
#define STRIDE 1
// =====================PREFETCHER=======================

// =====================MEMORY DESAMBIGUATION=======================
#define DESAMBIGUATION_ENABLED 1
// PERFECT
#define PERFECT 0
// HASHED
#define HASHED 1    
#define LOAD_HASH_SIZE 512
#define STORE_HASH_SIZE 512
#define DESAMBIGUATION_BLOCK_SIZE 4
// SOLVING ADDRESS TO ADDRESS
#define ADDRESS_TO_ADDRESS 1
// REGISTER FORWARD ON EXECUTION
#define REGISTER_FORWARD 1
// =====================MEMORY DESAMBIGUATION=======================

// =====================MEMORY CONFIGURATION=======================
#define ARRAY 0
#define CIRCULAR_BUFFER 1

// =====================MEMORY CONFIGURATION=======================



// ===================== EMC =======================================
#define EMC_ACTIVE 1
#define EMC_ROB_HEAD 0
// ===================== EMC =======================================
//WIDHTs
#define EMC_DISPATCH_WIDTH 2
#define EMC_EXECUTE_WIDTH 2
#define EMC_COMMIT_WIDTH 2
// Latencies
#define EMC_DISPATCH_LATENCY 1
#define EMC_INTEGER_LATENCY 1
#define EMC_COMMIT_LATENCY 1
//pipelines
#define EMC_WAIT_NEXT_INTEGER 1
//Sizes
#define EMC_INTEGER_ALU 2
#define EMC_UNIFIED_RS 8
#define EMC_UOP_BUFFER 16
#define EMC_REGISTERS 16
#define EMC_LSQ_SIZE 16
// EMC CACHE
#define EMC_CACHE_SIZE 4*KILO
#define EMC_CACHE_ASSOCIATIVITY 4
#define EMC_CACHE_LATENCY 1
#define EMC_CACHE_SETS (EMC_CACHE_SIZE/LINE_SIZE)/EMC_CACHE_ASSOCIATIVITY
// Access Predictor
#define MACT_SIZE 256
#define MACT_THRESHOLD 3
#define MACT_SHIFT 4
// ===================== EMC =======================================
// =====================CHECKS=======================
#define SANITY_CHECK 0
#define HEARTBEAT 1
#define HEARTBEAT_CLOCKS 10000000
// ==========DEBUGS
#define DEBUG 0
#define EMC_ACTIVE_DEBUG 0

#if DEBUG
#define FETCH_DEBUG 0
#define DECODE_DEBUG 0
#define RENAME_DEBUG 0
#define DISPATCH_DEBUG 0
#define EXECUTE_DEBUG 0
#define MOB_DEBUG 0
#define COMMIT_DEBUG 0
#define CACHE_MANAGER_DEBUG 0
#define MEM_CONTROLLER_DEBUG 0
// EMC Debugs
#define EMC_DEBUG 0
#define EMC_DISPATCH_DEBUG 0
#define EMC_EXECUTE_DEBUG 1
#define EMC_LSQ_DEBUG 1
#define EMC_COMMIT_DEBUG 1

#endif

#define PERIODIC_CHECK 0
#define CLOCKS_TO_CHECK 500
#define WAIT_CYCLE 158673400
// **************** END DEFINES ******************
#endif //_SANDYBRIDGE_HPP