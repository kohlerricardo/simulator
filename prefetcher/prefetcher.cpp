#include "../simulator.hpp"

prefetcher_t::prefetcher_t(){
    this->prefetcher = NULL;
    //ctor
};

prefetcher_t::~prefetcher_t()
{
    if(this->prefetcher!=NULL) delete &this->prefetcher;
    //dtor
};
void prefetcher_t::allocate(){
    this->set_latePrefetches(0);
    this->set_usefulPrefetches(0);
    this->set_latePrefetches(0);
    this->set_totalCycleLate(0);
    #if STRIDE
        this->prefetcher = new stride_prefetcher_t;
        this->prefetcher->allocate();
    #endif  
};
// ================================================================
// @mobLine - references to index the prefetch
// @*cache - cache to be instaled line prefetched
// ================================================================
void prefetcher_t::prefecht(memory_order_buffer_line_t *mob_line,cache_t *cache){
    int64_t newAddress = this->prefetcher->verify(mob_line->opcode_address,mob_line->memory_address);
    uint32_t sacrifice;
    if(newAddress != POSITION_FAIL){
        uint32_t status = cache->read(newAddress,sacrifice);
        if(status == MISS){
            this->add_totalPrefetched();
            linha_t *linha = cache->installLine(newAddress,RAM_LATENCY);
            #if EMC_ACTIVE
                linha_t *linha_emc = orcs_engine.memory_controller->emc->data_cache->installLine(newAddress,RAM_LATENCY);
                linha_emc->linha_ptr_inf = linha;
                linha->linha_ptr_emc=linha_emc; 
            #endif
            linha->prefetched =1; 
        }
    }
};
void prefetcher_t::statistics(){
    
    if(orcs_engine.output_file_name == NULL){
    ORCS_PRINTF("##############  PREFETCHER ##################\n")
    ORCS_PRINTF("Total Prefetches: %u\n", this->get_totalPrefetched())
    ORCS_PRINTF("Useful Prefetches: %u -> %.4f \n", this->get_usefulPrefetches(),(this->get_usefulPrefetches()*100.0)/this->get_totalPrefetched())
    ORCS_PRINTF("Late Prefetches: %u -> %.4f \n",this->get_latePrefetches(),((this->get_latePrefetches()*100.0)/this->get_totalPrefetched()))
    ORCS_PRINTF("MediaAtraso: %.3f\n",(float)this->get_totalCycleLate()/(float)this->get_latePrefetches())
    }
    else{
        FILE *output = fopen(orcs_engine.output_file_name,"a+");
		if(output != NULL){
            utils_t::largeSeparator(output);
            fprintf(output,"##############  PREFETCHER ##################\n");
            fprintf(output,"Total Prefetches: %u\n", this->get_totalPrefetched());
            fprintf(output,"Useful Prefetches: %u\n", this->get_usefulPrefetches());
            fprintf(output,"Late Prefetches: %u\n",this->get_latePrefetches());
            fprintf(output,"MediaAtraso: %.4f\n",(float)this->get_totalCycleLate()/(float)this->get_latePrefetches());
            utils_t::largeSeparator(output);
        }
        fclose(output);
    }
}; 