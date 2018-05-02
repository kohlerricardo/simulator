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
    //Read/Write counters
    this->set_readHit(0);
    this->set_readMiss(0);
    this->set_writeHit(0);
    this->set_writeMiss(0);

    //Allocate Prefetcher
    #if PREFETCHER_ACTIVE
    this->prefetcher = new prefetcher_t;
    this->prefetcher->allocate();
    #endif
    this->inst_load_miss=0;
    this->inst_load_load=0;
    this->inst_load_deps=0;
};
uint32_t cache_manager_t::searchInstruction(uint64_t instructionAddress){
    uint32_t ttc = 0;
    uint32_t latency_request = 0;
    uint32_t hit = this->inst_cache->read(instructionAddress,ttc);
    //if hit, add Searched instructions. Must be equal inst cache hit 
    latency_request+=ttc;
    if(hit==HIT){
        this->inst_cache->add_cacheAccess();
        this->inst_cache->add_cacheHit();
        this->add_instructionSearched();    
    }else
    {   
        // =========================
        this->inst_cache->add_cacheAccess();
        this->inst_cache->add_cacheMiss();
        this->add_instructionLLCSearched();
        // =========================
        hit = this->data_cache[1].read(instructionAddress,ttc);
        // ==========
        // update inst cache miss, update instruction llc search.
        // Inst cache miss must be equal llc search inst
        // ORCS_PRINTF("Latency L1 MISS %u\n",latency_request)

        latency_request+=ttc;        
        // ==========
        if(hit == HIT){
            //========================================= 
            this->data_cache[1].add_cacheAccess();
            this->data_cache[1].add_cacheHit();
            //========================================= 
            this->data_cache[1].returnLine(instructionAddress,this->inst_cache);
            #if CACHE_MANAGER_DEBUG
                // ORCS_PRINTF("Latency LLC HIT %u\n",latency_request)
            #endif
        }else{
            //========================================= 
            this->data_cache[1].add_cacheAccess();
            this->data_cache[1].add_cacheMiss();
            //========================================= 
            //llc inst miss
            latency_request+=RAM_LATENCY;
            #if CACHE_MANAGER_DEBUG
                // ORCS_PRINTF("Latency LLC MISS %u\n",latency_request)
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
uint32_t cache_manager_t::searchData(memory_order_buffer_line_t *mob_line,cache_status_t *has_llc_miss){
    uint32_t ttc = 0;
    uint32_t latency_request = 0;
    uint32_t hit = this->data_cache[0].read(mob_line->memory_address,ttc);
    latency_request+=ttc;
    //if hit, add Searched instructions. Must be equal inst cache hit 
    if(hit==HIT){
        //========================================= 
        this->data_cache[0].add_cacheAccess();
        this->data_cache[0].add_cacheHit();
        //========================================= 
    }else{
        //========================================= 
        this->data_cache[0].add_cacheAccess();
        this->data_cache[0].add_cacheMiss();
        //========================================= 
        hit = this->data_cache[1].read(mob_line->memory_address,ttc);
        // ==========
        // update inst cache miss, update instruction llc search.
        // Inst cache miss must be equal llc search inst
        // ==========
        latency_request+=ttc;
        #if CACHE_MANAGER_DEBUG
            // ORCS_PRINTF("L1 MISS TTC %u\n",ttc)
            // ORCS_PRINTF("L1 MISS LR %u\n",latency_request)
        #endif
        if(hit == HIT){
            //========================================= 
            this->data_cache[1].add_cacheAccess();
            this->data_cache[1].add_cacheHit();
            //========================================= 
            this->data_cache[1].returnLine(mob_line->memory_address,&this->data_cache[0]);
            // =========== Prefetcher ==================
            #if PREFETCHER_ACTIVE
                this->prefetcher->prefecht(mob_line,&this->data_cache[1]);
            #endif
        }else{
            this->add_readMiss();


            //========================================= 
            this->data_cache[1].add_cacheAccess();
            this->data_cache[1].add_cacheMiss();
            //========================================= 

            //========================================= 
            #if PREFETCHER_ACTIVE
            this->prefetcher->prefecht(mob_line,&this->data_cache[1]);
            #endif
            //========================================= 
            //llc inst miss
            latency_request+=RAM_LATENCY;

            // ====================
            // Install cache lines
            // ====================
            linha_t *linha_l1 = NULL;
            linha_t *linha_llc = NULL;
            linha_llc = this->data_cache[1].installLine(mob_line->memory_address);
            linha_l1 = this->data_cache[0].installLine(mob_line->memory_address);
            linha_l1->linha_ptr_sup=linha_llc;
            linha_llc->linha_ptr_inf=linha_l1;
            // =====================================
            //EMC
            // =====================================
            // linha_t *linha_emc = NULL;
            // linha_emc = orcs_engine.emc->data_cache->installLine(mob_line->memory_address);
            // linha_llc->linha_ptr_emc=linha_emc;
            // linha_emc->linha_ptr_llc=linha_llc;
            *has_llc_miss=MISS;
        }
    }
    return latency_request;
};
uint32_t cache_manager_t::writeData(memory_order_buffer_line_t *mob_line){

    uint32_t ttc = 0;
    uint32_t latency_request = 0;
    uint32_t hit = this->data_cache[0].read(mob_line->memory_address,ttc);
    latency_request+=ttc;
    //if hit, add Searched instructions. Must be equal inst cache hit 
    if(hit==HIT){
        //========================================= 
        this->data_cache[0].add_cacheAccess();
        this->data_cache[0].add_cacheHit();
        //========================================= 
        #if CACHE_MANAGER_DEBUG
            // ORCS_PRINTF("L1 Hit TTC %u\n",ttc)   
            // ORCS_PRINTF("L1 Hit LR %u\n",latency_request)
        #endif
        this->data_cache[0].write(mob_line->memory_address); 
    }else{   
        //========================================= 
        this->data_cache[0].add_cacheAccess();
        this->data_cache[0].add_cacheMiss();
        //========================================= 
        ttc = 0;
        hit = this->data_cache[1].read(mob_line->memory_address,ttc);
        // ==========
        // update inst cache miss, update instruction llc search.
        // Inst cache miss must be equal llc search inst
        // ==========

        if(hit == HIT){
            //========================================= 
            this->data_cache[1].add_cacheAccess();
            this->data_cache[1].add_cacheHit();
            //========================================= 
            latency_request+=ttc;
            // linha_t* linha_l1 = NULL;
            // install line new on d0
            this->data_cache[1].returnLine(mob_line->memory_address,&this->data_cache[0]);
            this->data_cache[0].write(mob_line->memory_address);
            #if PREFETCHER_ACTIVE
            this->prefetcher->prefecht(mob_line,&this->data_cache[1]);
            #endif
        }else{
            this->add_writeMiss();
            //========================================= 
            this->data_cache[1].add_cacheAccess();
            this->data_cache[1].add_cacheMiss();
            //========================================= 
            
            //========================================= 
            #if PREFETCHER_ACTIVE
            this->prefetcher->prefecht(mob_line,&this->data_cache[1]);
            #endif
            //========================================= 
            
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
            linha_llc = this->data_cache[1].installLine(mob_line->memory_address);
            linha_l1 = this->data_cache[0].installLine(mob_line->memory_address);
            linha_l1->linha_ptr_sup=linha_llc;
            linha_llc->linha_ptr_inf=linha_l1;
            this->data_cache[0].write(mob_line->memory_address);
        }
    }
    return latency_request;
};
void cache_manager_t::insertQueueRead(memory_order_buffer_line_t* mob_line){
    ERROR_ASSERT_PRINTF(mob_line->memory_operation == MEMORY_OPERATION_READ,"Error, Inserting Not Read Operation")
    this->read_buffer.push(mob_line);
};
void cache_manager_t::insertQueueWrite(memory_order_buffer_line_t* mob_line){
    ERROR_ASSERT_PRINTF(mob_line->memory_operation == MEMORY_OPERATION_WRITE,"Error, Inserting Not Write Operation")
    this->read_buffer.push(mob_line);
};
void cache_manager_t::clock(){
   
}
void cache_manager_t::statistics(){

    if(orcs_engine.output_file_name == NULL){
        ORCS_PRINTF("##############  Cache Memories ##################\n")
    }
    else{
        FILE *output = fopen(orcs_engine.output_file_name,"a+");
            if(output != NULL){
                utils_t::largestSeparator(output);  
                fprintf(output,"##############  Cache Memories ##################\n");
                utils_t::largestSeparator(output);  
            }
            fclose(output);
        }    
    // ORCS_PRINTF("############## Instruction Cache ##################\n")
    this->inst_cache->statistics();
    // ORCS_PRINTF("##############  Data Cache L1 ##################\n")
    this->data_cache[0].statistics();
    // ORCS_PRINTF("##############  LLC Cache ##################\n")
    this->data_cache[1].statistics();
    // Prefetcher
    #if PREFETCHER_ACTIVE
    this->prefetcher->statistics();
    #endif
    ORCS_PRINTF("Media inst entre Loads deps %.3f\n",(float(this->inst_load_load)/float(this->inst_load_deps)))
};