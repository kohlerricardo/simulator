#include "../simulator.hpp"

cache_manager_t::cache_manager_t(){
this->data_cache = NULL;
this->inst_cache = NULL;
};
cache_manager_t::~cache_manager_t(){
    if(this->data_cache) delete[] this->data_cache;
    if(this->inst_cache) delete this->inst_cache;
    
};
void cache_manager_t::allocate(){
    //Allocate I$
    this->inst_cache = new cache_t;
    this->inst_cache->allocate(INST_CACHE);
    //Allocate D$
    this->data_cache = new cache_t[CACHE_LEVELS];
    this->data_cache[0].allocate(L1); //L1
    this->data_cache[1].allocate(LLC); //LLC 
};
uint32_t cache_manager_t::searchInstruction(uint64_t instructionAddress){
    uint32_t ttc = 0;
    uint32_t latency_request = 0;
    int32_t sacrifice = POSITION_FAIL;
    uint32_t hit = this->inst_cache->read(instructionAddress,ttc);
    // this->inst_cache->add_cacheAccess();
    //if hit, add Searched instructions. Must be equal inst cache hit 
    latency_request+=ttc;
    if(hit==HIT){
        #if CACHE_MANAGER_DEBUG
            ORCS_PRINTF("Latency L1 %u\n",latency_request)
        #endif
        this->add_instructionSearched();    
    }else
    {   
        hit = this->data_cache[1].read(instructionAddress,ttc);
        // ==========
        // update inst cache miss, update instruction llc search.
        // Inst cache miss must be equal llc search inst
        // ORCS_PRINTF("Latency L1 MISS %u\n",latency_request)
        this->add_instructionLLCSearched();
        latency_request+=ttc;        
        // ==========
        
        if(hit == HIT){
            this->data_cache[1].returnLine(instructionAddress,this->inst_cache,sacrifice);
            #if CACHE_MANAGER_DEBUG
                ORCS_PRINTF("Latency LLC HIT %u\n",latency_request)
            #endif
        }else{
            //llc inst miss
            latency_request+=RAM_LATENCY;
            #if CACHE_MANAGER_DEBUG
                ORCS_PRINTF("Latency LLC MISS %u\n",latency_request)
            #endif
            // ====================
            // add mem controller, to install lines
            // ====================
            this->inst_cache->installLine(instructionAddress);
            this->data_cache[1].installLine(instructionAddress);
        }
    }
    return latency_request;
};
uint32_t cache_manager_t::searchData(uint64_t dataAddress){
    uint32_t ttc = 0;
    uint32_t latency_request = 0;
    int32_t sacrifice = POSITION_FAIL;
    uint32_t hit = this->data_cache[0].read(dataAddress,ttc);
    latency_request+=ttc;
    //if hit, add Searched instructions. Must be equal inst cache hit 
    if(hit==HIT){
        #if CACHE_MANAGER_DEBUG
            ORCS_PRINTF("L1 Hit TTC %u\n",ttc)   
            ORCS_PRINTF("L1 Hit LR %u\n",latency_request)   
        #endif
    }else
    {   
        hit = this->data_cache[1].read(dataAddress,ttc);
        // ==========
        // update inst cache miss, update instruction llc search.
        // Inst cache miss must be equal llc search inst
        // ==========
        latency_request+=ttc;
        #if CACHE_MANAGER_DEBUG
            ORCS_PRINTF("L1 MISS TTC %u\n",ttc)
            ORCS_PRINTF("L1 MISS LR %u\n",latency_request)
        #endif
        if(hit == HIT){
            this->data_cache[1].returnLine(dataAddress,&this->data_cache[0],sacrifice);
            #if CACHE_MANAGER_DEBUG
                ORCS_PRINTF("LLC Hit TTC %u\n",ttc)
                ORCS_PRINTF("LLC Hit LR %u\n",latency_request)
            #endif
        }else{
            //llc inst miss
            latency_request+=RAM_LATENCY;
        #if CACHE_MANAGER_DEBUG
            ORCS_PRINTF("LLC MISS LR %u\n",latency_request)
        #endif
            // ====================
            // add mem controller, to install lines
            // ====================
            this->data_cache[0].installLine(dataAddress);
            this->data_cache[1].installLine(dataAddress);
        }
    }
    return latency_request;
};
uint32_t cache_manager_t::writeData(uint64_t dataAddress){

    uint32_t ttc = 0;
    int32_t line=POSITION_FAIL;
    uint32_t latency_request = 0;
    uint32_t hit = this->data_cache[0].read(dataAddress,ttc);
    latency_request+=ttc;
    //if hit, add Searched instructions. Must be equal inst cache hit 
    if(hit==HIT){
        #if CACHE_MANAGER_DEBUG
            ORCS_PRINTF("L1 Hit TTC %u\n",ttc)   
            ORCS_PRINTF("L1 Hit LR %u\n",latency_request)
        #endif
        this->data_cache[0].write(dataAddress,line);
    }else{   
        hit = this->data_cache[1].read(dataAddress,ttc);
        // ==========
        // update inst cache miss, update instruction llc search.
        // Inst cache miss must be equal llc search inst
        // ==========

        if(hit == HIT){
            latency_request+=ttc;
            this->data_cache[1].returnLine(dataAddress,&this->data_cache[0],line);
            this->data_cache[0].write(dataAddress,line);
        }else{
            //llc inst miss
            latency_request+=RAM_LATENCY;
            // ====================
            // add mem controller, to install lines
            // algo no estilo 
            // ttc = orcs_engine.memory_controller->request(dataAddress);
            // ====================
            this->data_cache[0].installLine(dataAddress);
            this->data_cache[1].installLine(dataAddress);
            this->data_cache[0].write(dataAddress,line);
        }
    }
    return latency_request;
};
void cache_manager_t::statistics(){
    ORCS_PRINTF("##############  Cache Manager ##################\n")
    ORCS_PRINTF("Instruction Searched : %lu\n",this->get_instructionSearched())
    ORCS_PRINTF("Instruction LLC Searched : %lu\n",this->get_instructionLLCSearched())
    ORCS_PRINTF("Data Searched : %lu\n",this->get_dataSearched())
    
    ORCS_PRINTF("############## Instruction Cache ##################\n")
    this->inst_cache->statistics();
    ORCS_PRINTF("##############  Data Cache L1 ##################\n")
    this->data_cache[0].statistics();
    ORCS_PRINTF("##############  LLC Cache ##################\n")
    this->data_cache[1].statistics();

};