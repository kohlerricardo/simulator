#include "./../simulator.hpp"
#include <string>
// ============================================================================
memory_controller_t::memory_controller_t(){};
// ============================================================================
memory_controller_t::~memory_controller_t(){
    #if EMC_ACTIVE
    if(this->emc !=NULL) delete emc;
    #endif
};
// ============================================================================

// ============================================================================
// @allocate objects to EMC
void memory_controller_t::allocate(){
    #if EMC_ACTIVE
    this->emc = new emc_t;
    this->emc->allocate();
    #endif
};
// ============================================================================
void memory_controller_t::statistics(){
    if(orcs_engine.output_file_name == NULL){
        utils_t::largestSeparator();
        ORCS_PRINTF("Memory Controller")
        utils_t::largestSeparator();
    }else{
        FILE *output = fopen(orcs_engine.output_file_name,"a+");
		if(output != NULL){
        utils_t::largestSeparator(output);
        fprintf(output,"Memory Controller");
        utils_t::largestSeparator(output);
        }
    }
};
// ============================================================================
void memory_controller_t::clock(){};
// ============================================================================