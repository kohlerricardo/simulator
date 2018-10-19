/// This is the main header file, it contains all the prototypes and
/// describes the relations between classes.
#ifndef _ORCS_ORCS_HPP_
#define _ORCS_ORCS_HPP_

/// C Includes
#include <unistd.h>     /* for getopt */
#include <getopt.h>     /* for getopt_long; POSIX standard getopt is in unistd.h */
#include <inttypes.h>   /* for uint32_t */
#include <zlib.h>
#include <assert.h> //asserts
 #include <sys/time.h> //get time of day
/// C++ Includes
//facilities
#include <iostream>
#include <iomanip>
#include <cmath>
#include <chrono>
#include <thread>
//original
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>
#include <vector>
#include <list>
#include <queue>
#include <algorithm>


// ============================================================================
/// Classes
// ============================================================================

class orcs_engine_t;
class trace_reader_t;
class opcode_package_t;

//  =========================================//
// Branch Predictor Classes
//  =========================================//

class btb_line_t;
class btb_t;
class twoBit_t;
class piecewise_t;
class branch_predictor_t;

//  =========================================//
// Out of order Execution classes
//  =========================================//

class uop_package_t;
class reorder_buffer_line_t;
class memory_order_buffer_line_t;
class processor_t;

//  =========================================//
// Cache Classes
//  =========================================//

class linha_t;
class cacheSet_t;
class cache_t;
class cache_manager_t;

//  =========================================//
// Usefull Classes
//  =========================================//
template<class CB_TYPE> class circular_buffer_t;
class utils_t;
class sanity_test_t;
class priority_memory_access_t;
//  =========================================//
// Prefetcher Classes
//  =========================================//
class prefetcher_t;
class stride_table_t;
class stride_prefetcher_t;
//  =========================================//
// Memory Controller Classes
//  =========================================//
class memory_controller_t;
//  =========================================//
// EMC classes
//  =========================================//
class emc_t;
class emc_opcode_package_t;
class register_remapping_table_t;
// ====================================
// DATA Types
// ====================================
typedef std::vector <reorder_buffer_line_t*> container_ptr_reorder_buffer_line_t;
typedef std::vector <emc_opcode_package_t*> container_ptr_emc_opcode_package_t;
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
// ========================
// Defines Simulators Caracteristics
// ========================
#define MAX_UOP_DECODED 5
#define MAX_REGISTERS 6         /// opcode_package_t uop_package_t  (Max number of register (read or write) for one opcode/uop)
#define MAX_ASSEMBLY_SIZE 32    /// In general 20 is enough
// ========================
// Model simulator
// ========================
#define SKYLAKE 0
// ========================
// ==============================================================================
/// Enumerations
// ==============================================================================
#include "utils/enumerations.hpp"
// ============================================================================
// ==============================================================================
/// Macros
// ==============================================================================
#include "utils/macros.hpp"
// ============================================================================
// ============================================================================
/// Base Includes
// ============================================================================
#if SKYLAKE
#include "./model_skylake.hpp"
#else
#include "./model_sandyBridge.hpp"
#endif
#include "./simulator.hpp"
#include "./orcs_engine.hpp"
#include "./trace_reader.hpp"
#include "./package/opcode_package.hpp"



//  =========================================//
// Usefull Classes
//  =========================================//
#include "./utils/circular_buffer.hpp"
#include "./utils/utils.hpp"
#include "./utils/sanityTest.hpp"
//  =========================================//
// Core Includes
//  =========================================//
#include "./package/uop_package.hpp"
#include "./processor/reorder_buffer_line.hpp"
#include "./processor/memory_order_buffer_line.hpp"
#include "./processor/processor.hpp"
//  =========================================//
// Branch Predictor includes
//  =========================================//
#include "./branch_predictor/btb_line.hpp"
#include "./branch_predictor/btb.hpp"
#include "./branch_predictor/twoBit.hpp"
#include "./branch_predictor/piecewise.hpp"
#include "./branch_predictor/branch_predictor.hpp"
//  =========================================//
// Compare classes to priority queue
//  =========================================//
#include "./utils/class_order.hpp"
//  =========================================//
// Cache Classes
//  =========================================//
// // CACHE INCLUDES
#include "./cache/linha.hpp"
#include "./cache/cacheSet.hpp"
#include "./cache/cache.hpp"
#include "./cache/cache_manager.hpp"
//  =========================================//
// // Prefetcher INCLUDES
#include "./prefetcher/prefetcher.hpp"
#include "./prefetcher/stride_table.hpp"
#include "./prefetcher/stride_prefetcher.hpp"
//  =========================================//
// // MemoryController INCLUDES
#include "./main_memory/memory_controller.hpp"

// // EMC Includes
#include "./main_memory/emc/emc_opcode_package.hpp"
#include "./main_memory/emc/emc.hpp"
#include "./processor/register_remapping_table.hpp"

//  =========================================//
#endif  // _ORCS_ORCS_HPP_
