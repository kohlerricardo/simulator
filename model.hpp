
/**
Arquivo definindo os parametros do modelo de processador
*/

#ifndef _SKYLAKE_HPP_
#define _SKYLAKE_HPP_
// =========== PROCESSOR =============
// PROCESSOR STAGES WIDHT
#define FETCH_WIDTH 4
#define DECODE_WIDTH 5
#define RENAME_WIDTH 6
#define DISPATCH_WIDTH 8
#define COMMIT_WIDTH 8
// PROCESSOR LATENCIES STAGES
#define FETCH_LATENCY 5
#define DECODE_LATENCY 5
#define RENAME_LATENCY 3
#define DISPATCH_LATENCY 2
//ULAS INTEGER LATENCY
#define LATENCY_INTEGER_ALU 1
#define LATENCY_INTEGER_MUL 3
#define LATENCY_INTEGER_DIV 8
//FP ULAS LATENCY
#define LATENCY_FP_DIV 8
#define LATENCY_FP_MUL 4
#define LATENCY_FP_ALU 4

// QTDE UFS
#define INTEGER_ALU 4
#define INTEGER_MUL 1
#define INTEGER_DIV 1
#define FP_ALU 1
#define FP_MUL 1
#define FP_DIV 1
#define LOAD_UNIT 2
#define STORE_UNIT 1

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


#define BTB_MISS_PENALITY 8
#define MISSPREDICTION_PENALITY 15
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
#define L2_LATENCY 12
#define L2_SETS (L2_SIZE/LINE_SIZE)/L2_ASSOCIATIVITY
// ==================== LEVEL 2 =====================
// ==================== LLC     =====================
#define LLC_SIZE 1*MEGA
#define LLC_ASSOCIATIVITY 16
#define LLC_LATENCY 44
#define LLC_SETS (LLC_SIZE/LINE_SIZE)/LLC_ASSOCIATIVITY
// ==================== LLC     =====================
// =====================CACHES=======================

// =====================RAM=======================
#define RAM_LATENCY 170
#define RAM_SIZE 4 * MEGA * KILO
// =====================RAM=======================

// =====================CHECKS=======================
#define SANITY_CHECK 1



// **************** END DEFINES ******************
#endif //_SKYLAKE_HPP_