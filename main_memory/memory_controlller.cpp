#include "./../simulator.hpp"
#include <string>
// ============================================================================
memory_controller_t::memory_controller_t(){};
// ============================================================================
memory_controller_t::~memory_controller_t(){
    if(this->data_cache !=NULL) delete this->data_cache;
};
// ============================================================================

// ============================================================================
// @allocate objects to EMC
void memory_controller_t::allocate(){
    // uopBuffer
    // reservation station
    // data cache

};
// ============================================================================
void memory_controller_t::statistics(){
    if(orcs_engine.output_file_name == NULL){
        utils_t::largestSeparator();
        ORCS_PRINTF("EMC - Enhaced Memory Controller")
        utils_t::largestSeparator();
    }else{
        FILE *output = fopen(orcs_engine.output_file_name,"a+");
		if(output != NULL){
        utils_t::largestSeparator(output);
        fprintf(output,"EMC - Enhaced Memory Controller");
        utils_t::largestSeparator(output);
        }
    }
};
// ============================================================================
void memory_controller_t::core_to_emc(){};
// ============================================================================
void memory_controller_t::dispatch(){};
// ============================================================================
void memory_controller_t::execute(){};
// ============================================================================
void memory_controller_t::emc_to_core(){};
// ============================================================================
void memory_controller_t::clock(){};
// ============================================================================