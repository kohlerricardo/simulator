#include "simulator.hpp"

// =====================================================================
orcs_engine_t::orcs_engine_t() {
	this->trace_reader = NULL;
	this->processor = NULL;
	this->branchPredictor = NULL;
	this->cacheManager = NULL;
	this->memory_controller = NULL;
	this->is_warmup =  false;
	this->instruction_warmup_counter =0;
};

// =====================================================================
void orcs_engine_t::allocate() {
	// Statistics Time
	gettimeofday(&this->stat_timer_start, NULL);
	gettimeofday(&this->stat_timer_end, NULL);
	// 
	ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(NUMBER_OF_PROCESSORS)==OK,"Error - # of processors must be power of 2 value")
	this->trace_reader = new trace_reader_t[NUMBER_OF_PROCESSORS];
	this->processor = new processor_t[NUMBER_OF_PROCESSORS];
	this->branchPredictor = new branch_predictor_t[NUMBER_OF_PROCESSORS];
	this->cacheManager = new cache_manager_t;
	this->memory_controller = new memory_controller_t;
};
bool orcs_engine_t::get_simulation_alive(){
	for(uint16_t cpu=0;cpu<NUMBER_OF_PROCESSORS;cpu++){
		if (this->processor[cpu].isBusy()) {
            return OK;
        }
	}
	return FAIL;
};
void orcs_engine_t::global_reset_statistics(){
	for(uint8_t i = 0; i < NUMBER_OF_PROCESSORS; i++)
	{
		this->processor[i].reset_statistics();
		


		this->branchPredictor[i].reset_statistics();
		#if EMC_ACTIVE
			orcs_engine.memory_controller->emc[i].reset_statistics();
		#endif
		
		this->processor[i].set_warmup_last_opcode(orcs_engine.trace_reader[i].get_fetch_instructions());
		this->processor[i].set_warmup_reset_cycle(orcs_engine.get_global_cycle());
	}
	orcs_engine.memory_controller->data_cache->reset_statistics();
	orcs_engine.memory_controller->reset_statistics();
	#if PREFETCHER_ACTIVE
		orcs_engine.prefetcher->reset_statistics();
	#endif
	for(uint8_t i = 0; i < SIZE_OF_L1_CACHES_ARRAY; i++){
		this->cacheManager->inst_cache[i].reset_statistics();
		this->cacheManager->L1_data_cache[i].reset_statistics();
	}
	for(uint8_t i = 0; i < SIZE_OF_L2_CACHES_ARRAY; i++){
		this->cacheManager->L2_data_cache[i].reset_statistics();

	}
	for(uint8_t i = 0; i < SIZE_OF_LLC_CACHES_ARRAY; i++){
		this->cacheManager->LLC_data_cache[i].reset_statistics();

	}
}
