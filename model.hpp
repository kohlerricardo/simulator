
/**
Arquivo definindo os parametros do modelo de processador
*/

#ifndef _SANDY_BRIDGE_HPP_
#define _SANDY_BRIDGE_HPP_
// PROCESSOR STAGES WIDHT
#define FETCH_DECODE_WIDTH 4
#define DECODE_BUFFER 32
#define EXECUTE_WIDTH 6


#define UNIFIED_RS 54
#define LOAD_BUFFER 64
#define STORE_BUFFER 36
// ===========BRANCH PREDICTOR=============
#define BTB_ENTRIES 4096
#define BTB_WAYS 2
//COUNTERS
#define TWO_BIT 0
#define PIECEWISE 1 
enum taken_t{
    NOT_TAKEN = 0,
    TAKEN = 1
};
enum status_t{
    HIT,
    MISS
};
#define BTB_MISS_PENALITY 8

// =====================
// include e defines do branch predictor
// #define N 128
// #define M 128
// #define H 43
#define N 256
#define M 64
#define H 63
#define  THETA ((2.14*(H)) + 20.58)
// ===========END BRANCH PREDICTOR=============
// *************** DEFINES CACHE *****************

#define KILO 1024
#define MEGA KILO*KILO

enum cacheLevel_t{
    L1,
    L2,
    LLC
};
#define CACHE_LEVELS 2
#define BLOCK_SIZE 64
#define BYTES_ON_LINE 24
//Define L1
#define L1_SIZE 32*KILO
#define L1_ASSOCIATIVITY 8
#define L1_LATENCY 1
#define L1_SETS (L1_SIZE/BLOCK_SIZE)/L1_ASSOCIATIVITY
//Define L2
#define L2_SIZE 256*KILO
#define L2_ASSOCIATIVITY 8
#define L2_LATENCY 4
#define L2_SETS (LLC_SIZE/BLOCK_SIZE)/LLC_ASSOCIATIVITY
//Define RAM
#define RAM_LATENCY 170
#define RAM_SIZE 4 * MEGA * KILO

// **************** END DEFINES ******************



#endif //_SANDY_BRIDGE_HPP_