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
    this->inst_cache->add_cacheAccess();
    uint32_t time_to_complete = 0;
    int32_t sacrifice = POSITION_FAIL;
    uint32_t hit = this->inst_cache->searchAddress(instructionAddress);
    time_to_complete+=L1_INST_LATENCY;
    //if hit, add Searched instructions. Must be equal inst cache hit 
    if(hit==HIT){
        this->add_instructionSearched();
        this->inst_cache->add_cacheHit();      
    }else
    {   
        // ==========
        // update inst cache miss, update instruction llc search.
        // Inst cache miss must be equal llc search inst
        this->inst_cache->add_cacheMiss();
        this->add_instructionLLCSearched();
        // ==========

        hit = this->data_cache[LLC].searchAddress(instructionAddress);
        this->data_cache[LLC].add_cacheAccess();
        time_to_complete+=LLC_LATENCY;
        if(hit == HIT){
            this->data_cache[LLC].returnLine(instructionAddress,this->inst_cache,sacrifice);
            // llc inst hit
        }else{
            //llc inst miss
            time_to_complete+=RAM_LATENCY;
            this->inst_cache->installLine(instructionAddress);
            this->data_cache[LLC].installLine(instructionAddress);
        }
    }
    return time_to_complete;
};
uint32_t cache_manager_t::searchData(uint64_t dataAddress){
return dataAddress;
};
uint32_t cache_manager_t::writeData(uint64_t dataAddress){
    uint32_t ttc = 0;
    int32_t line=POSITION_FAIL;
    ttc += L1_DATA_LATENCY;
    uint32_t hit = this->data_cache[L1].searchAddress(dataAddress);
    this->add_dataSearched();
    if(hit==HIT){
        this->data_cache[L1].write(dataAddress,line);
    }else{
        ttc+= LLC_LATENCY;
        hit = this->data_cache[LLC].searchAddress(dataAddress);
        if(hit ==  HIT){
            this->data_cache[LLC].returnLine(dataAddress,&this->data_cache[L1],line);
            this->data_cache[L1].write(dataAddress,line);
        }
        else{
            this->data_cache[L1].installLine(dataAddress); 
            this->data_cache[LLC].installLine(dataAddress); 
            this->data_cache[L1].write(dataAddress,line);
        }
    }
    return OK;
};