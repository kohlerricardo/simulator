#include "./../simulator.hpp"
#include <string>
// ============================================================================
memory_controller_t::memory_controller_t(){

    this->emc = NULL;
	// Data Cache
	this->data_cache = NULL;
};
// ============================================================================
memory_controller_t::~memory_controller_t() = default;
// ============================================================================

// ============================================================================
// @allocate objects to EMC
void memory_controller_t::allocate(){
    // ======================= data cache =======================
	this->data_cache = new cache_t;
	this->data_cache->allocate(EMC_DATA_CACHE);

    // ======================= EMC =======================
    #if EMC_ACTIVE
    this->emc = new emc_t[NUMBER_OF_PROCESSORS];
    for (uint32_t i = 0; i < NUMBER_OF_PROCESSORS; i++)
    {
        this->emc[i].allocate();
    }
    #endif
};
// ============================================================================
void memory_controller_t::statistics(){
    FILE *output = stdout;
    bool close = false;
	if(orcs_engine.output_file_name != NULL){
        close=true;
		output = fopen(orcs_engine.output_file_name,"a+");
    }
	if (output != NULL){
        utils_t::largestSeparator(output);
        fprintf(output,"#Memory Controller\n");
        utils_t::largestSeparator(output);
        fprintf(output,"Requests_Made: %lu\n",this->get_requests_made());
        fprintf(output,"Requests_from_LLC: %lu\n",this->get_requests_llc());
        fprintf(output,"Requests_from_EMC: %lu\n",this->get_requests_emc());
        utils_t::largestSeparator(output);
        if(close) fclose(output);
        #if EMC_ACTIVE
            for (uint32_t i = 0; i < NUMBER_OF_PROCESSORS; i++)
            {
                this->emc[i].statistics();
            }
            utils_t::largestSeparator(output);
            fprintf(output, "##############  EMC_Data_Cache ##################\n");
            this->data_cache->statistics();
        #endif
        }
};
// ============================================================================
void memory_controller_t::clock(){
    #if EMC_ACTIVE
        for (uint32_t i = 0; i < NUMBER_OF_PROCESSORS; i++)
        {
            this->emc[i].clock();        
        }
    #endif

};
// ============================================================================

// ============================================================================
uint64_t memory_controller_t::requestDRAM(){
    this->add_requests_made();
    return RAM_LATENCY;
};
// ============================================================================
void memory_controller_t::reset_statistics(){
    this->set_requests_made(0);
    this->set_operations_executed(0);
    this->set_requests_emc(0);
    this->set_requests_llc(0);
}
// ============================================================================
