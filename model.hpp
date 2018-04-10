
/**
Arquivo definindo os parametros do modelo de processador
*/

#ifndef _SKYLAKE_HPP_
#define _SKYLAKE_HPP_
// =========== PROCESSOR =============
// PROCESSOR STAGES WIDHT
// #define FETCH_WIDTH 1
// #define DECODE_WIDTH 1
// #define RENAME_WIDTH 1
// #define DISPATCH_WIDTH 1
// #define EXECUTE_WIDTH 1
// #define COMMIT_WIDTH 1
// ==============================
#define FETCH_WIDTH 4
#define DECODE_WIDTH 5
#define RENAME_WIDTH 6
#define DISPATCH_WIDTH 8
#define EXECUTE_WIDTH 8
#define COMMIT_WIDTH 8
// =============================
// PROCESSOR LATENCIES STAGES
#define FETCH_LATENCY 1
#define DECODE_LATENCY 1
#define RENAME_LATENCY 1
#define DISPATCH_LATENCY 1
#define EXECUTE_LATENCY 0
#define COMMIT_LATENCY 1
// ========= FUNCTIONAL UNITS RELATED=========

// INTEGER ALU
#define LATENCY_INTEGER_ALU 1
#define WAIT_NEXT_INT_ALU 1
#define INTEGER_ALU 4
// INTEGER MUL
#define LATENCY_INTEGER_MUL 3
#define WAIT_NEXT_INT_MUL 1
#define INTEGER_MUL 1
// INTEGER DIV
#define LATENCY_INTEGER_DIV 8
#define WAIT_NEXT_INT_DIV 1
#define INTEGER_DIV 1

#define QTDE_INTEGER_FU (INTEGER_ALU+INTEGER_MUL+INTEGER_DIV)

//FP ULAS LATENCY 
// FLOATING POINT DIV
#define LATENCY_FP_DIV 8
#define WAIT_NEXT_FP_DIV 1
#define FP_DIV 1
// FLOATING POINT MUL
#define LATENCY_FP_MUL 4
#define WAIT_NEXT_FP_MUL 1
#define FP_MUL 1
// FLOATING POINT ALU
#define LATENCY_FP_ALU 4
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
#define FETCH_BUFFER 58
#define DECODE_BUFFER 128
#define RAT_SIZE 260
#define ROB_SIZE 224
#define UNIFIED_RS 97
//MOB
#define MOB_READ 72
#define MOB_WRITE 56


// ===========BRANCH PREDICTOR=============
#define BTB_ENTRIES 4096
#define BTB_WAYS 2
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
// D$
#define L1_DATA_SIZE 32*KILO
#define L1_DATA_ASSOCIATIVITY 8
#define L1_DATA_LATENCY 4
#define L1_DATA_SETS (L1_DATA_SIZE/LINE_SIZE)/L1_DATA_ASSOCIATIVITY
// I$
#define L1_INST_SIZE 32*KILO
#define L1_INST_ASSOCIATIVITY 8
#define L1_INST_LATENCY 4
#define L1_INST_SETS (L1_INST_SIZE/LINE_SIZE)/L1_INST_ASSOCIATIVITY
// ==================== LEVEL 1 =====================
// ==================== LEVEL 2 =====================
#define L2_SIZE 256*KILO
#define L2_ASSOCIATIVITY 4
#define L2_LATENCY 8
#define L2_SETS (L2_SIZE/LINE_SIZE)/L2_ASSOCIATIVITY
// ==================== LEVEL 2 =====================
// ==================== LLC     =====================
#define LLC_SIZE 1*MEGA
#define LLC_ASSOCIATIVITY 16
#define LLC_LATENCY 32
#define LLC_SETS (LLC_SIZE/LINE_SIZE)/LLC_ASSOCIATIVITY
// ==================== LLC     =====================
// =====================CACHES=======================

// =====================RAM=======================
#define RAM_LATENCY 170
#define RAM_SIZE 4 * MEGA * KILO
// =====================RAM=======================
#define LATENCY_TOTAL (L1_DATA_LATENCY+LLC_LATENCY+RAM_LATENCY)
// =====================CHECKS=======================
#define SANITY_CHECK 1
// ==========DEBUGS
#define DEBUG 0

#if DEBUG
#define FETCH_DEBUG 0
#define DECODE_DEBUG 0
#define RENAME_DEBUG 1
#define DISPATCH_DEBUG 0
#define EXECUTE_DEBUG 0
#define MOB_DEBUG 0
#define COMMIT_DEBUG 0
#define CACHE_MANAGER_DEBUG 0

#endif

#define PERIODIC_CHECK 0
#define CLOCKS_TO_CHECK 1000
#define WAIT_CYCLE 2400
// **************** END DEFINES ******************
#endif //_SKYLAKE_HPP_