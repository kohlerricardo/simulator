#include "../../simulator.hpp"
emc_t::emc_t(){
    
};
emc_t::~emc_t(){
    if(this->data_cache !=NULL) delete this->data_cache;
};
// ============================================================================
// @allocate objects to EMC
void emc_t::allocate(){
    // uopBuffer
    // reservation station
    // data cache
    this->data_cache->allocate(EMC_DATA_CACHE);
    //alocate uop buffer
};
// ============================================================================
void emc_t::statistics(){
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
void emc_t::core_to_emc(){};
// ============================================================================
void emc_t::dispatch(){};
// ============================================================================
void emc_t::execute(){};
// ============================================================================
void emc_t::emc_to_core(){};
// ============================================================================