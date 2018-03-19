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
#if DEBUG
    // ORCS_PRINTF("Instruction Cache\n")
    // this->inst_cache->printCacheConfiguration();
    // ORCS_PRINTF("L1 DATA Cache\n")
    // this->data_cache[0].printCacheConfiguration();
    // ORCS_PRINTF("LLC Cache\n")
    // this->data_cache[1].printCacheConfiguration();
#endif

};
uint32_t cache_manager_t::searchInstruction(uint64_t instructionAddress){
    uint32_t ttc = 0;
    int32_t sacrifice = POSITION_FAIL;
    uint32_t hit = this->inst_cache->read(instructionAddress,ttc);
    this->inst_cache->add_cacheAccess();
    //if hit, add Searched instructions. Must be equal inst cache hit 
    if(hit==HIT){
        ORCS_PRINTF("TTC %u\n",ttc)
        this->add_instructionSearched();    
    }else
    {   
        hit = this->data_cache[1].read(instructionAddress,ttc);
        // ==========
        // update inst cache miss, update instruction llc search.
        // Inst cache miss must be equal llc search inst
        this->add_instructionLLCSearched();
        // ==========
        ORCS_PRINTF("TTC %u\n",ttc)
        if(hit == HIT){
            this->data_cache[1].returnLine(instructionAddress,this->inst_cache,sacrifice);
        }else{
            //llc inst miss
            ttc+=RAM_LATENCY;
            // ====================
            // add mem controller, to install lines
            // ====================
            this->inst_cache->installLine(instructionAddress);
            this->data_cache[1].installLine(instructionAddress);
        }
    }
    return ttc;
};
uint32_t cache_manager_t::searchData(uint64_t dataAddress){
    uint32_t ttc = 0;
    int32_t sacrifice = POSITION_FAIL;
    uint32_t hit = this->data_cache[0].read(dataAddress,ttc);
    this->data_cache[0].add_cacheAccess();
    //if hit, add Searched instructions. Must be equal inst cache hit 
    if(hit==HIT){
        ORCS_PRINTF("TTC %u\n",ttc)   
    }else
    {   
        hit = this->data_cache[1].read(dataAddress,ttc);
        // ==========
        // update inst cache miss, update instruction llc search.
        // Inst cache miss must be equal llc search inst
        // ==========
        ORCS_PRINTF("TTC %u\n",ttc)
        if(hit == HIT){
            this->data_cache[1].returnLine(dataAddress,&this->data_cache[0],sacrifice);
        }else{
            //llc inst miss
            ttc+=RAM_LATENCY;
            // ====================
            // add mem controller, to install lines
            // ====================
            this->data_cache[0].installLine(dataAddress);
            this->data_cache[1].installLine(dataAddress);
        }
    }
    return ttc;
};
uint32_t cache_manager_t::writeData(uint64_t dataAddress){

    uint32_t ttc = 0;
    int32_t line=POSITION_FAIL;
    uint32_t hit = this->data_cache[0].read(dataAddress,ttc);
    this->data_cache[0].add_cacheAccess();
    //if hit, add Searched instructions. Must be equal inst cache hit 
    if(hit==HIT){
        ORCS_PRINTF("TTC %u\n",ttc)
        this->data_cache[0].write(dataAddress,line);
    }else{   
        hit = this->data_cache[1].read(dataAddress,ttc);
        // ==========
        // update inst cache miss, update instruction llc search.
        // Inst cache miss must be equal llc search inst
        // ==========
        ORCS_PRINTF("TTC %u\n",ttc)

        if(hit == HIT){
            this->data_cache[1].returnLine(dataAddress,&this->data_cache[0],line);
            this->data_cache[0].write(dataAddress,line);
        }else{
            //llc inst miss
            ttc+=RAM_LATENCY;
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
    return ttc;
};