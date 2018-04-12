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
    this->hit=0;
    this->miss=0;
};
uint32_t cache_manager_t::searchInstruction(uint64_t instructionAddress){
    uint32_t ttc = 0;
    uint32_t latency_request = 0;
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
            this->data_cache[1].returnLine(instructionAddress,this->inst_cache);
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
            // Install cache lines
            // ====================
            linha_t *linha_inst = NULL;
            linha_t *linha_llc = NULL;
            linha_llc = this->data_cache[1].installLine(instructionAddress);
            linha_inst = this->inst_cache->installLine(instructionAddress);
            linha_inst->linha_ptr_sup=linha_llc;
            linha_llc->linha_ptr_inf=linha_inst;
        }
    }
    return latency_request;
};
uint32_t cache_manager_t::searchData(uint64_t dataAddress){
    uint32_t ttc = 0;
    uint32_t latency_request = 0;
    uint32_t hit = this->data_cache[0].read(dataAddress,ttc);
    latency_request+=ttc;
    //if hit, add Searched instructions. Must be equal inst cache hit 
    if(hit==HIT){
        #if CACHE_MANAGER_DEBUG
            ORCS_PRINTF("L1 Hit TTC %u\n",ttc)   
            ORCS_PRINTF("L1 Hit LR %u\n",latency_request)   
        #endif
    }else{
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
            this->data_cache[1].returnLine(dataAddress,&this->data_cache[0]);
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
            // Install cache lines
            // ====================
            linha_t *linha_l1 = NULL;
            linha_t *linha_llc = NULL;
            linha_llc = this->data_cache[1].installLine(dataAddress);
            linha_l1 = this->data_cache[0].installLine(dataAddress);
            linha_l1->linha_ptr_sup=linha_llc;
            linha_llc->linha_ptr_inf=linha_l1;
        }
    }
    return latency_request;
};
uint32_t cache_manager_t::writeData(uint64_t dataAddress){

    uint32_t ttc = 0;
    uint32_t latency_request = 0;
    uint32_t hit = this->data_cache[0].read(dataAddress,ttc);
    latency_request+=ttc;
    //if hit, add Searched instructions. Must be equal inst cache hit 
    if(hit==HIT){

        #if CACHE_MANAGER_DEBUG
            ORCS_PRINTF("L1 Hit TTC %u\n",ttc)   
            ORCS_PRINTF("L1 Hit LR %u\n",latency_request)
        #endif
        this->data_cache[0].write(dataAddress);
    }else{   
        ttc = 0;
        hit = this->data_cache[1].read(dataAddress,ttc);
        // ==========
        // update inst cache miss, update instruction llc search.
        // Inst cache miss must be equal llc search inst
        // ==========

        if(hit == HIT){
            latency_request+=ttc;
            // linha_t* linha_l1 = NULL;
            // install line new on d0
            this->data_cache[1].returnLine(dataAddress,&this->data_cache[0]);
            this->data_cache[0].write(dataAddress);
        }else{
            //llc inst miss
            latency_request+=RAM_LATENCY;
            // ====================
            // add mem controller, to install lines
            // algo no estilo 
            // ttc = orcs_engine.memory_controller->request(dataAddress);
            // ====================

            // ====================
            // Install cache lines
            // ====================
            linha_t *linha_l1 = NULL;
            linha_t *linha_llc = NULL;
            linha_llc = this->data_cache[1].installLine(dataAddress);
            linha_l1 = this->data_cache[0].installLine(dataAddress);
            linha_l1->linha_ptr_sup=linha_llc;
            linha_llc->linha_ptr_inf=linha_l1;
            this->data_cache[0].write(dataAddress);
        }
    }
    return latency_request;
};
void cache_manager_t::insertQueueRead(memory_order_buffer_line_t mob_line){
    ERROR_ASSERT_PRINTF(mob_line.memory_operation == MEMORY_OPERATION_READ,"Error, Inserting Not Read Operation")
    this->read_buffer.push(mob_line);
};
void cache_manager_t::insertQueueWrite(memory_order_buffer_line_t mob_line){
    ERROR_ASSERT_PRINTF(mob_line.memory_operation == MEMORY_OPERATION_WRITE,"Error, Inserting Not Write Operation")
    this->read_buffer.push(mob_line);
};
void cache_manager_t::clock(){
    uint32_t read_executed = 0,write_executed=0;
    while(!this->read_buffer.empty()){
        if(read_executed >= PARALLEL_LOADS){
            break;
        }

        if(this->read_buffer.top().readyAt >= orcs_engine.get_global_cycle()){
            break;
        }
        uint32_t latency = 0;
        latency = this->searchData(this->read_buffer.top().memory_address);
        this->read_buffer.top().rob_ptr->uop.updatePackageReady(latency);
        this->read_buffer.top().rob_ptr->mob_ptr->status=PACKAGE_STATE_READY;
        this->read_buffer.top().rob_ptr->mob_ptr->readyAt=this->read_buffer.top().rob_ptr->mob_ptr->readyAt+latency;
        read_executed++;
        this->read_buffer.pop();
    }
     while(!this->write_buffer.empty()){
        if(write_executed >= PARALLEL_STORES){
            break;
        }
        if(this->write_buffer.top().readyAt >= orcs_engine.get_global_cycle()){
            break;
        }
        uint32_t latency = 0;
        latency = this->searchData(this->write_buffer.top().memory_address);
        //se não terminar ou travar, significa que nao ta atualizando o mob,
        // entao tem atualizar via top().rob_ptr->mob_ptr.updateXXXX
        this->write_buffer.top().rob_ptr->uop.updatePackageReady(latency);
        this->write_buffer.top().rob_ptr->mob_ptr->status=PACKAGE_STATE_READY;
        this->write_buffer.top().rob_ptr->mob_ptr->readyAt=this->write_buffer.top().rob_ptr->mob_ptr->readyAt+latency;
        write_executed++;
        this->write_buffer.pop();
    }
}
void cache_manager_t::statistics(){
    ORCS_PRINTF("##############  Cache Manager ##################\n")
    ORCS_PRINTF("Instruction Searched : %lu\n",this->get_instructionSearched())
    ORCS_PRINTF("Instruction LLC Searched : %lu\n",this->get_instructionLLCSearched())
    ORCS_PRINTF("L1 Hits: %lu\n",this->get_hit())
    ORCS_PRINTF("L1 Miss: %lu\n",this->get_miss())
    
    ORCS_PRINTF("############## Instruction Cache ##################\n")
    this->inst_cache->statistics();
    ORCS_PRINTF("##############  Data Cache L1 ##################\n")
    this->data_cache[0].statistics();
    ORCS_PRINTF("##############  LLC Cache ##################\n")
    this->data_cache[1].statistics();

};