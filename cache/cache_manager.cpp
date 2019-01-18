#include "./../simulator.hpp"

cache_manager_t::cache_manager_t(){
this->data_cache = NULL;
this->inst_cache = NULL;
};
cache_manager_t::~cache_manager_t()=default;
void cache_manager_t::allocate(){
    //Allocate I$
    this->inst_cache = new cache_t;
    this->inst_cache->allocate(INST_CACHE);
    //Allocate D$
    this->data_cache = new cache_t[CACHE_LEVELS];
    this->data_cache[L1].allocate(L1); //L1
    this->data_cache[L2].allocate(L2); //L2
    this->data_cache[LLC].allocate(LLC); //Llc
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
    }else{   
        // READ L2 CACHE TO FIND INSTRUCTION;
        // =========================
        this->inst_cache->add_cacheAccess();
        this->inst_cache->add_cacheMiss();
        this->add_instructionLLCSearched();
        // =========================
        hit = this->data_cache[L2].read(instructionAddress,ttc);
        latency_request+=ttc;        
        // ==========
        if(hit == HIT){
            // L2 HIT
            //========================================= 
            this->data_cache[L2].add_cacheAccess();
            this->data_cache[L2].add_cacheHit();
            //========================================= 
            this->data_cache[L2].returnLine(instructionAddress,this->inst_cache);
        }else{
            // L2 MISS
            //========================================= 
            this->data_cache[L2].add_cacheAccess();
            this->data_cache[L2].add_cacheMiss();
            //========================================= 
            // =========================
            // READ LLC CACHE TO FIND INSTRUCTION;
            hit = this->data_cache[LLC].read(instructionAddress,ttc);
            latency_request+=ttc;    
            if(hit == HIT){
            // LLC HIT
            //========================================= 
            this->data_cache[LLC].add_cacheAccess();
            this->data_cache[LLC].add_cacheHit();
            //========================================= 
            this->data_cache[LLC].returnLine(instructionAddress,this->inst_cache);
            }else{
                //request to Memory Controller
                ttc = orcs_engine.memory_controller->requestDRAM();
                orcs_engine.memory_controller->add_requests_llc(); // requests made by LLC
                latency_request+=ttc;
                #if CACHE_MANAGER_DEBUG
                    // ORCS_PRINTF("Latency LLC MISS %u\n",latency_request)
                #endif
                // ====================
                // Install cache lines
                // ====================
                linha_t *linha_inst = NULL;
                linha_t *linha_l2 = NULL;
                linha_t *linha_llc = NULL;
                linha_inst = this->inst_cache->installLine(instructionAddress,latency_request);
                linha_l2 = this->data_cache[L2].installLine(instructionAddress,latency_request);
                linha_llc = this->data_cache[LLC].installLine(instructionAddress,latency_request);
                // SETTING POINTERS
                //setting linha_inst
                linha_inst->linha_ptr_l2=linha_l2;
                linha_inst->linha_ptr_llc=linha_llc;
                // setting level l2
                linha_l2->linha_ptr_l1=linha_inst;
                linha_l2->linha_ptr_llc=linha_llc;
                // settign LLC
                linha_llc->linha_ptr_l1=linha_inst;
                linha_llc->linha_ptr_l2=linha_l2;
                //NULLING POINTERS <LEAK MEMORY>
                linha_inst = NULL;
                linha_l2 = NULL;
                linha_llc = NULL;
            }
        }
    }
    return latency_request;
};
uint32_t cache_manager_t::searchData(memory_order_buffer_line_t *mob_line){
    #if CACHE_MANAGER_DEBUG
    if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
            ORCS_PRINTF("===================================\n")
            ORCS_PRINTF("Global_Cycle %lu\n",orcs_engine.get_global_cycle())
		}

    #endif
    uint32_t ttc = 0;
    uint32_t latency_request = 0;
    uint32_t hit = this->data_cache[L1].read(mob_line->memory_address,ttc);
    this->data_cache[L1].add_cacheRead();
    latency_request+=ttc;
    //L1 Hit
    if(hit==HIT){
        //========================================= 
        this->data_cache[L1].add_cacheAccess();
        this->data_cache[L1].add_cacheHit();
        //========================================= 
    }
    else{
        // L1 MISS
        //========================================= 
        this->data_cache[L1].add_cacheAccess();
        this->data_cache[L1].add_cacheMiss();
        //========================================= 
        // ACCESS L2 CACHE - MAKE READ
        hit = this->data_cache[L2].read(mob_line->memory_address,ttc);
        this->data_cache[L2].add_cacheRead();
        latency_request+=ttc;
        if(hit == HIT){
            // L2 Hit
            //========================================= 
            this->data_cache[L2].add_cacheAccess();
            this->data_cache[L2].add_cacheHit();
            //========================================= 
            this->data_cache[L2].returnLine(mob_line->memory_address,&this->data_cache[L1]);
        }else{
            // L2 MISS
            this->add_readMiss();
            //========================================= 
            this->data_cache[L2].add_cacheAccess();
            this->data_cache[L2].add_cacheMiss();
            hit = this->data_cache[LLC].read(mob_line->memory_address,ttc);
            latency_request+=ttc;
            if(hit == HIT){
                this->data_cache[LLC].add_cacheAccess();
                this->data_cache[LLC].add_cacheHit();
                this->data_cache[LLC].returnLine(mob_line->memory_address,&this->data_cache[L1]);
                //========================================= 
                #if PREFETCHER_ACTIVE
                    this->prefetcher->prefecht(mob_line,&this->data_cache[LLC]);
                #endif
                //========================================= 
            }else{
                this->data_cache[LLC].add_cacheAccess();
                this->data_cache[LLC].add_cacheMiss();
                orcs_engine.processor->has_llc_miss=true; // setting llc miss
                mob_line->is_llc_miss=true;
                //========================================= 
                //request to Memory Controller
                ttc = orcs_engine.memory_controller->requestDRAM();
                orcs_engine.memory_controller->add_requests_llc();  // requests made by LLC
                mob_line->waiting_DRAM=true;                        //Settind wait DRAM
                latency_request +=ttc;                              // Add latency from RAM
                // ====================
                orcs_engine.processor->request_DRAM++;
                // ====================
                //========================================= 
                #if PREFETCHER_ACTIVE
                    this->prefetcher->prefecht(mob_line,&this->data_cache[LLC]);
                #endif
                //========================================= 
                // ====================
                // Install cache lines
                // ====================
                linha_t *linha_l1 = NULL;
                linha_t *linha_l2 = NULL;
                linha_t *linha_llc = NULL;
                linha_l1 = this->data_cache[L1].installLine(mob_line->memory_address,latency_request);
                linha_l2 = this->data_cache[L2].installLine(mob_line->memory_address,latency_request);
                linha_llc = this->data_cache[LLC].installLine(mob_line->memory_address,latency_request);
                // SETTING POINTERS
                //setting linha_l1
                linha_l1->linha_ptr_l2=linha_l2;
                linha_l1->linha_ptr_llc=linha_llc;
                // setting level l2
                linha_l2->linha_ptr_l1=linha_l1;
                linha_l2->linha_ptr_llc=linha_llc;
                // settign LLC
                linha_llc->linha_ptr_l1=linha_l1;
                linha_llc->linha_ptr_l2=linha_l2;
                //NULLING POINTERS <LEAK MEMORY>
                linha_l1 = NULL;
                linha_l2 = NULL;
                linha_llc = NULL;
                // =====================================
                //EMC
                // =====================================
                #if EMC_ACTIVE
                linha_t *linha_emc = NULL;
                linha_emc = orcs_engine.memory_controller->emc->data_cache->installLine(mob_line->memory_address,RAM_LATENCY);
                linha_llc->linha_ptr_emc=linha_emc;
                linha_emc->linha_ptr_llc=linha_llc;
                #endif
            }
        }
    }
    return latency_request;
};
uint32_t cache_manager_t::writeData(memory_order_buffer_line_t *mob_line){

    uint32_t ttc = 0;
    uint32_t latency_request = 0;
    uint32_t hit = this->data_cache[L1].read(mob_line->memory_address,ttc);
    latency_request+=ttc;
    //if hit
    if(hit==HIT){
        //========================================= 
        this->data_cache[L1].add_cacheAccess();
        this->data_cache[L1].add_cacheHit();
        //========================================= 
        #if CACHE_MANAGER_DEBUG
            // ORCS_PRINTF("L1 Hit TTC %u\n",ttc)   
            // ORCS_PRINTF("L1 Hit LR %u\n",latency_request)
        #endif
        this->data_cache[L1].write(mob_line->memory_address); 
    }else{   
        // L1 MISS
        //========================================= 
        this->data_cache[L1].add_cacheAccess();
        this->data_cache[L1].add_cacheMiss();
        //========================================= 
        hit = this->data_cache[L2].read(mob_line->memory_address,ttc);
        if(hit == HIT){
            //========================================= 
            this->data_cache[L2].add_cacheAccess();
            this->data_cache[L2].add_cacheHit();
            //========================================= 
            latency_request+=ttc;
            this->data_cache[L2].returnLine(mob_line->memory_address,&this->data_cache[L1]);
            this->data_cache[L1].write(mob_line->memory_address);
        }else{
            // L2 MISS
            //========================================= 
            this->data_cache[L2].add_cacheAccess();
            this->data_cache[L2].add_cacheMiss();
            //========================================= 
            // search LLC
            hit = this->data_cache[LLC].read(mob_line->memory_address,ttc);
            if(hit == HIT){
                //========================================= 
                this->data_cache[LLC].add_cacheAccess();
                this->data_cache[LLC].add_cacheHit();
                latency_request+=ttc;
                //========================================= 
                this->data_cache[LLC].returnLine(mob_line->memory_address,&this->data_cache[L1]);
                this->data_cache[L1].write(mob_line->memory_address);
            }else{
                //llc miss
                ttc = orcs_engine.memory_controller->requestDRAM();
                orcs_engine.memory_controller->add_requests_llc(); // requests made by LLC
                latency_request +=ttc;
                // ====================
                // Install cache lines
                // ====================
                linha_t *linha_l1 = NULL;
                linha_t *linha_l2 = NULL;
                linha_t *linha_llc = NULL;
                linha_l1 = this->data_cache[L1].installLine(mob_line->memory_address,latency_request);
                linha_l2 = this->data_cache[L2].installLine(mob_line->memory_address,latency_request);
                linha_llc = this->data_cache[LLC].installLine(mob_line->memory_address,latency_request);
                // SETTING POINTERS
                //setting linha_l1
                linha_l1->linha_ptr_l2=linha_l2;
                linha_l1->linha_ptr_llc=linha_llc;
                // setting level l2
                linha_l2->linha_ptr_l1=linha_l1;
                linha_l2->linha_ptr_llc=linha_llc;
                // settign LLC
                linha_llc->linha_ptr_l1=linha_l1;
                linha_llc->linha_ptr_l2=linha_l2;
                //NULLING POINTERS <LEAK MEMORY>
                linha_l1 = NULL;
                linha_l2 = NULL;
                linha_llc = NULL;
                this->data_cache[L1].write(mob_line->memory_address);
            }
        }
    }
    return latency_request;
};


uint32_t cache_manager_t::search_EMC_Data(memory_order_buffer_line_t *mob_line){
    uint32_t ttc = 0;
    uint32_t latency_request = 0;
    uint32_t hit = orcs_engine.memory_controller->emc->data_cache->read(mob_line->memory_address,ttc);
    orcs_engine.memory_controller->emc->data_cache->add_cacheRead();
    latency_request+=ttc;
    //EMC data cache Hit
    if(hit==HIT){
        //========================================= 
        orcs_engine.memory_controller->emc->data_cache->add_cacheAccess();
        orcs_engine.memory_controller->emc->data_cache->add_cacheHit();
        //========================================= 
    }else{
        // EMC CACHE MISS
        //========================================= 
        orcs_engine.memory_controller->emc->data_cache->add_cacheAccess();
        orcs_engine.memory_controller->emc->data_cache->add_cacheMiss();
        //========================================= 
        hit = this->data_cache[LLC].read(mob_line->memory_address,ttc); 

        if(hit == HIT){ 
            latency_request+=ttc;
            // marcando access llc emc
            orcs_engine.memory_controller->emc->add_access_LLC();
            orcs_engine.memory_controller->emc->add_access_LLC_Hit();
            mob_line->is_llc_miss=false;
        }else{
            orcs_engine.memory_controller->emc->add_access_LLC();
            orcs_engine.memory_controller->emc->add_access_LLC_Miss();
            
            latency_request += RAM_LATENCY;

            linha_t *linha_llc = this->data_cache[1].installLine(mob_line->memory_address,latency_request);
            linha_t *linha_emc = orcs_engine.memory_controller->emc->data_cache->installLine(mob_line->memory_address,latency_request);
            // linking emc and llc
            linha_llc->linha_ptr_emc = linha_emc;
            linha_emc->linha_llc = linha_llc;
            orcs_engine.memory_controller->add_requests_emc();//number of requests made by emc
            orcs_engine.memory_controller->add_requests_made();//add requests made by emc to total
        }
    }
    return latency_request;
};

void cache_manager_t::statistics(){
    bool close = false;
    FILE *output = stdout;
	if(orcs_engine.output_file_name != NULL){
		output = fopen(orcs_engine.output_file_name,"a+");
        close=true;
    }
	if (output != NULL){
        utils_t::largestSeparator(output);  
        fprintf(output,"##############  Cache Memories ##################\n");
        utils_t::largestSeparator(output);  
        }
	if(close) fclose(output);
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
};