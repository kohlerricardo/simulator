#include "simulator.hpp"

// =====================================================================
orcs_engine_t::orcs_engine_t() {
	this->trace_reader = NULL;
	this->processor = NULL;
	this->branchPredictor = NULL;
	this->cacheManager = NULL;
	this->memory_controller = NULL;
};

// =====================================================================
void orcs_engine_t::allocate() {
	// Statistics Time
	gettimeofday(&this->stat_timer_start, NULL);
	gettimeofday(&this->stat_timer_end, NULL);
	// 
	this->trace_reader = new trace_reader_t;
	this->processor = new processor_t;
	this->branchPredictor = new branch_predictor_t;
	this->cacheManager = new cache_manager_t;
	this->memory_controller = new memory_controller_t;
};

