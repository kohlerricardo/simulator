#include "./../simulator.hpp"

cache_manager_t::cache_manager_t(){
this->L1_data_cache = NULL;
this->L2_data_cache = NULL;
this->LLC_data_cache = NULL;
this->inst_cache = NULL;
}
cache_manager_t::~cache_manager_t()=default;
void cache_manager_t::allocate(){
    //Allocate I$
    ERROR_ASSERT_PRINTF(NUMBER_OF_PROCESSORS == SIZE_OF_L1_CACHES_ARRAY,"Error - # Instruction Caches must be equal # PROCESSORS \n")
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(SIZE_OF_L1_CACHES_ARRAY)==OK,"Error - Cache Size Array must be power of 2 value \n")
    this->inst_cache = new cache_t[SIZE_OF_L1_CACHES_ARRAY];
    for (uint32_t i = 0; i < SIZE_OF_L1_CACHES_ARRAY; i++){
        this->inst_cache[i].allocate(INST_CACHE);
    }
    //Allocate L1 D$
    ERROR_ASSERT_PRINTF(NUMBER_OF_PROCESSORS == SIZE_OF_L1_CACHES_ARRAY,"Error - # Data Caches must be equal # PROCESSORS \n")
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(SIZE_OF_L1_CACHES_ARRAY)==OK,"Error - Cache Size Array must be power of 2 value \n")
    this->L1_data_cache = new cache_t[SIZE_OF_L1_CACHES_ARRAY];
    for (uint32_t i = 0; i < SIZE_OF_L1_CACHES_ARRAY; i++){
        this->L1_data_cache[i].allocate(L1);
    }
     //Allocate L2 D$
    ERROR_ASSERT_PRINTF(SIZE_OF_L2_CACHES_ARRAY <= NUMBER_OF_PROCESSORS,"Error - # Instruction Caches must be equal # PROCESSORS \n")
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(SIZE_OF_L2_CACHES_ARRAY)==OK,"Error - Cache Size Array must be power of 2 value \n")
    this->L2_data_cache = new cache_t[SIZE_OF_L2_CACHES_ARRAY];
    for (uint32_t i = 0; i < SIZE_OF_L2_CACHES_ARRAY; i++){
        this->L2_data_cache[i].allocate(L2);
    }
     //Allocate LLC D$
    ERROR_ASSERT_PRINTF(SIZE_OF_LLC_CACHES_ARRAY <= NUMBER_OF_PROCESSORS,"Error - # Instruction Caches must be equal # PROCESSORS \n")
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(SIZE_OF_LLC_CACHES_ARRAY)==OK,"Error - Cache Size Array must be power of 2 value \n")
    this->LLC_data_cache = new cache_t[SIZE_OF_LLC_CACHES_ARRAY];
    for (uint32_t i = 0; i < SIZE_OF_LLC_CACHES_ARRAY; i++){
        this->LLC_data_cache[i].allocate(LLC);
    }
   
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
}
uint32_t cache_manager_t::searchInstruction(uint32_t processor_id,uint64_t instructionAddress){
    uint32_t ttc = 0;
    uint32_t latency_request = 0;
    int32_t index_inst = this->generate_index_array(processor_id,INST_CACHE);
    int32_t index_l2 = this->generate_index_array(processor_id,L2);
    int32_t index_llc = this->generate_index_array(processor_id,LLC);
    if((index_inst == POSITION_FAIL)||(index_l2==POSITION_FAIL)||(index_llc==POSITION_FAIL)){
        ERROR_PRINTF("Error on generate index to access array")
    }

    #if CACHE_MANAGER_DEBUG
        if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
            ORCS_PRINTF("Indexes [%d] [%d] [%d]\n",index_inst,index_l2,index_llc);
        }
    #endif
    uint32_t hit = this->inst_cache[index_inst].read(instructionAddress,ttc);
    //if hit, add Searched instructions. Must be equal inst cache hit 
    latency_request+=ttc;
    if(hit==HIT){
        #if CACHE_MANAGER_DEBUG
            if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
                ORCS_PRINTF("HIT L1 Inst ttc:[%u] Latency Request: [%u]\n",ttc,latency_request);
            }
        #endif
        this->inst_cache[index_inst].add_cacheAccess();
        this->inst_cache[index_inst].add_cacheHit();
        this->add_instructionSearched();    
    }else{   
        ttc = 0;
        // READ L2 CACHE TO FIND INSTRUCTION;
        // =========================
        this->inst_cache[index_inst].add_cacheAccess();
        this->inst_cache[index_inst].add_cacheMiss();
        this->add_instructionLLCSearched();
        // =========================
        hit = this->L2_data_cache[index_l2].read(instructionAddress,ttc);
        latency_request+=ttc;        
        // ==========
        if(hit == HIT){
            #if CACHE_MANAGER_DEBUG
                if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
                    ORCS_PRINTF("HIT L2 ttc:[%u] Latency Request: [%u]\n",ttc,latency_request);
                }
            #endif
            // L2 HIT
            //========================================= 
            this->L2_data_cache[index_l2].add_cacheAccess();
            this->L2_data_cache[index_l2].add_cacheHit();
            //========================================= 
            this->L2_data_cache[index_l2].returnLine(instructionAddress,this->inst_cache);
        }else{
            ttc = 0;
            // L2 MISS
            //========================================= 
            this->L2_data_cache[index_l2].add_cacheAccess();
            this->L2_data_cache[index_l2].add_cacheMiss();
            //========================================= 
            // =========================
            // READ LLC CACHE TO FIND INSTRUCTION;
            hit = this->LLC_data_cache[index_llc].read(instructionAddress,ttc);
            latency_request+=ttc;    
            if(hit == HIT){
                #if CACHE_MANAGER_DEBUG
                    if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
                        ORCS_PRINTF("HIT LLC ttc:[%u] Latency Request: [%u]\n",ttc,latency_request);
                    }
                #endif
            // LLC HIT
            //========================================= 
            this->LLC_data_cache[index_llc].add_cacheAccess();
            this->LLC_data_cache[index_llc].add_cacheHit();
            //========================================= 
            this->LLC_data_cache[index_llc].returnLine(instructionAddress,&this->L2_data_cache[index_l2]);
            this->L2_data_cache[index_l2].returnLine(instructionAddress,&this->inst_cache[index_inst]);
            }else{
                //========================================= 
                this->LLC_data_cache[index_llc].add_cacheAccess();
                this->LLC_data_cache[index_llc].add_cacheMiss();
                //request to Memory Controller
                ttc = orcs_engine.memory_controller->requestDRAM();
                orcs_engine.memory_controller->add_requests_llc(); // requests made by LLC
                latency_request+=ttc;
                #if CACHE_MANAGER_DEBUG
                    if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
                        ORCS_PRINTF("HIT LLC ttc:[%u] Latency Request: [%u]\n",ttc,latency_request);
                    }
                #endif
                // ====================
                // Install cache lines
                // ====================
                linha_t *linha_inst = NULL;
                linha_t *linha_l2 = NULL;
                linha_t *linha_llc = NULL;
                linha_inst = this->inst_cache[index_inst].installLine(instructionAddress,latency_request);
                linha_l2 = this->L2_data_cache[index_l2].installLine(instructionAddress,latency_request);
                linha_llc = this->LLC_data_cache[index_llc].installLine(instructionAddress,latency_request);
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
}
uint32_t cache_manager_t::searchData(memory_order_buffer_line_t *mob_line){
    #if CACHE_MANAGER_DEBUG
    if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
            ORCS_PRINTF("============== SEARCH DATA =====================\n")
            ORCS_PRINTF("Global_Cycle %lu\n",orcs_engine.get_global_cycle())
		}

    #endif
    uint32_t ttc = 0;
    uint32_t latency_request = 0;
    int32_t index_l1 = this->generate_index_array(mob_line->processor_id,L1);
    int32_t index_l2 = this->generate_index_array(mob_line->processor_id,L2);
    int32_t index_llc = this->generate_index_array(mob_line->processor_id,LLC);
    if((index_l1 == POSITION_FAIL)||(index_l2==POSITION_FAIL)||(index_llc==POSITION_FAIL)){
        ERROR_PRINTF("Error on generate index to access array")
    }

    uint32_t hit = this->L1_data_cache[index_l1].read(mob_line->memory_address,ttc);
    this->L1_data_cache[index_l1].add_cacheRead();
    latency_request+=ttc;
    //L1 Hit
    if(hit==HIT){
        //========================================= 
        this->L1_data_cache[index_l1].add_cacheAccess();
        this->L1_data_cache[index_l1].add_cacheHit();
        //========================================= 
    }
    else{
        ttc = 0;
        // L1 MISS
        //========================================= 
        this->L1_data_cache[index_l1].add_cacheAccess();
        this->L1_data_cache[index_l1].add_cacheMiss();
        //========================================= 
        // ACCESS L2 CACHE - MAKE READ
        hit = this->L2_data_cache[index_l2].read(mob_line->memory_address,ttc);
        this->L2_data_cache[index_l2].add_cacheRead();
        latency_request+=ttc;
        if(hit == HIT){
            // L2 Hit
            //========================================= 
            this->L2_data_cache[index_l2].add_cacheAccess();
            this->L2_data_cache[index_l2].add_cacheHit();
            //========================================= 
            this->L2_data_cache[index_l2].returnLine(mob_line->memory_address,&this->L1_data_cache[index_l1]);
        }else{
            ttc = 0;
            // L2 MISS
            //========================================= 
            this->L2_data_cache[index_l2].add_cacheAccess();
            this->L2_data_cache[index_l2].add_cacheMiss();
            hit = this->LLC_data_cache[index_llc].read(mob_line->memory_address,ttc);
            latency_request+=ttc;
            this->LLC_data_cache[index_llc].add_cacheRead();
            if(hit == HIT){
                this->LLC_data_cache[index_llc].add_cacheAccess();
                this->LLC_data_cache[index_llc].add_cacheHit();

                this->LLC_data_cache[index_llc].returnLine(mob_line->memory_address,&this->L2_data_cache[index_l2]);
                this->L2_data_cache[index_l2].returnLine(mob_line->memory_address,&this->L1_data_cache[index_l1]);
                //========================================= 
                #if PREFETCHER_ACTIVE
                    this->prefetcher->prefecht(mob_line,&this->LLC_data_cache[index_llc]);
                #endif
                //========================================= 
            }else{
                this->LLC_data_cache[index_llc].add_cacheAccess();
                this->LLC_data_cache[index_llc].add_cacheMiss();
                orcs_engine.processor[mob_line->processor_id].has_llc_miss=true; // setting llc miss
                mob_line->core_generate_miss=true;
                mob_line->rob_ptr->original_miss=true;
                //========================================= 
                //request to Memory Controller
                ttc = orcs_engine.memory_controller->requestDRAM();
                orcs_engine.memory_controller->add_requests_llc();  // requests made by LLC
                mob_line->waiting_DRAM=true;                        //Settind wait DRAM
                latency_request +=ttc;                              // Add latency from RAM
                // ====================
                orcs_engine.processor[mob_line->processor_id].request_DRAM++;
                // ====================
                //========================================= 
                #if PREFETCHER_ACTIVE
                    this->prefetcher->prefecht(mob_line,&this->LLC_data_cache[index_llc]);
                #endif
                //========================================= 
                // ====================
                // Install cache lines
                // ====================
                linha_t *linha_l1 = NULL;
                linha_t *linha_l2 = NULL;
                linha_t *linha_llc = NULL;
                linha_l1 = this->L1_data_cache[index_l1].installLine(mob_line->memory_address,latency_request);
                linha_l2 = this->L2_data_cache[index_l2].installLine(mob_line->memory_address,latency_request);
                linha_llc = this->LLC_data_cache[index_llc].installLine(mob_line->memory_address,latency_request);
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
                // =====================================
                //EMC
                // =====================================
                #if EMC_ACTIVE
                linha_t *linha_emc = NULL;
                linha_emc = orcs_engine.memory_controller->data_cache->installLine(mob_line->memory_address,RAM_LATENCY);
                linha_llc->linha_ptr_emc=linha_emc;
                linha_emc->linha_ptr_llc=linha_llc;
                linha_emc=NULL;
                #endif
                //NULLING POINTERS <LEAK MEMORY>
                linha_l1 = NULL;
                linha_l2 = NULL;
                linha_llc = NULL;
            }
        }
    }
    return latency_request;
}
uint32_t cache_manager_t::writeData(memory_order_buffer_line_t *mob_line){

    uint32_t ttc = 0;
    uint32_t latency_request = 0;

    int32_t index_l1 = this->generate_index_array(mob_line->processor_id,L1);
    int32_t index_l2 = this->generate_index_array(mob_line->processor_id,L2);
    int32_t index_llc = this->generate_index_array(mob_line->processor_id,LLC);
    if((index_l1 == POSITION_FAIL)||(index_l2==POSITION_FAIL)||(index_llc==POSITION_FAIL)){
        ERROR_PRINTF("Error on generate index to access array")
    }
    uint32_t hit = this->L1_data_cache[index_l1].read(mob_line->memory_address,ttc);
    latency_request+=ttc;
    //if hit
    if(hit==HIT){
        //========================================= 
        this->L1_data_cache[index_l1].add_cacheAccess();
        this->L1_data_cache[index_l1].add_cacheHit();
        //========================================= 
        #if CACHE_MANAGER_DEBUG
            // ORCS_PRINTF("L1 Hit TTC %u\n",ttc)   
            // ORCS_PRINTF("L1 Hit LR %u\n",latency_request)
        #endif
        this->L1_data_cache[index_l1].write(mob_line->memory_address); 
    }else{   
        ttc = 0;
        // L1 MISS
        //========================================= 
        this->L1_data_cache[index_l1].add_cacheAccess();
        this->L1_data_cache[index_l1].add_cacheMiss();
        //========================================= 
        hit = this->L2_data_cache[index_l2].read(mob_line->memory_address,ttc);
        latency_request+=ttc;
        if(hit == HIT){
            //========================================= 
            this->L2_data_cache[index_l2].add_cacheAccess();
            this->L2_data_cache[index_l2].add_cacheHit();
            //========================================= 
            this->L2_data_cache[index_l2].returnLine(mob_line->memory_address,&this->L1_data_cache[index_l1]);
            this->L1_data_cache[index_l1].write(mob_line->memory_address);
        }else{
            ttc = 0;
            // L2 MISS
            //========================================= 
            this->L2_data_cache[index_l2].add_cacheAccess();
            this->L2_data_cache[index_l2].add_cacheMiss();
            //========================================= 
            // search LLC
            hit = this->LLC_data_cache[index_llc].read(mob_line->memory_address,ttc);
            latency_request+=ttc;
            if(hit == HIT){
                //========================================= 
                this->LLC_data_cache[index_llc].add_cacheAccess();
                this->LLC_data_cache[index_llc].add_cacheHit();
                //========================================= 
                this->LLC_data_cache[index_llc].returnLine(mob_line->memory_address,&this->L2_data_cache[index_l2]);
                this->L2_data_cache[index_l2].returnLine(mob_line->memory_address,&this->L1_data_cache[index_l1]);
                //Writing Data
                this->L1_data_cache[index_l1].write(mob_line->memory_address);
            }else{
                //llc miss
                this->LLC_data_cache[index_llc].add_cacheAccess();
                this->LLC_data_cache[index_llc].add_cacheMiss();
                ttc = orcs_engine.memory_controller->requestDRAM();
                orcs_engine.memory_controller->add_requests_llc(); // requests made by LLC
                latency_request +=ttc;
                // ====================
                // Install cache lines
                // ====================
                linha_t *linha_l1 = NULL;
                linha_t *linha_l2 = NULL;
                linha_t *linha_llc = NULL;
                linha_l1 = this->L1_data_cache[index_l1].installLine(mob_line->memory_address,latency_request);
                linha_l2 = this->L2_data_cache[index_l2].installLine(mob_line->memory_address,latency_request);
                linha_llc = this->LLC_data_cache[index_llc].installLine(mob_line->memory_address,latency_request);
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
                this->L1_data_cache[index_l1].write(mob_line->memory_address);
            }
        }
    }
    return latency_request;
}


uint32_t cache_manager_t::search_EMC_Data(memory_order_buffer_line_t *mob_line){
        #if CACHE_MANAGER_DEBUG
            if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
                ORCS_PRINTF("============== SEARCH_EMC_DATA =====================\n")
                ORCS_PRINTF("Global_Cycle %lu\n",orcs_engine.get_global_cycle())
            }

        #endif
    uint32_t ttc = 0;
    uint32_t latency_request = 0;
    
    int32_t index_llc = this->generate_index_array(mob_line->processor_id,LLC);
    if(index_llc==POSITION_FAIL){
        ERROR_PRINTF("Error on generate index to access array")
    }
    uint32_t hit = orcs_engine.memory_controller->data_cache->read(mob_line->memory_address,ttc);
    orcs_engine.memory_controller->data_cache->add_cacheRead();
    latency_request+=ttc;
    //EMC data cache Hit
    if(hit==HIT){
        #if CACHE_MANAGER_DEBUG
            if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
                ORCS_PRINTF("HIT L1_EMC ttc:[%u] Latency Request: [%u]\n",ttc,latency_request);
            }
        #endif
        //========================================= 
        orcs_engine.memory_controller->data_cache->add_cacheAccess();
        orcs_engine.memory_controller->data_cache->add_cacheHit();
        mob_line->l1_emc_hit=true;
        //========================================= 
    }else{
        // EMC CACHE MISS
        //========================================= 
        orcs_engine.memory_controller->data_cache->add_cacheAccess();
        orcs_engine.memory_controller->data_cache->add_cacheMiss();
        orcs_engine.memory_controller->emc[mob_line->processor_id].add_access_LLC();

        //========================================= 
        // Predict Access Direct RAM
        uint64_t index = utils_t::hash_function(HASH_FUNCTION_INPUT1_ONLY,mob_line->emc_opcode_ptr->uop.opcode_address>>2,0,orcs_engine.memory_controller->emc[mob_line->processor_id].mact_bits_mask);
        if(orcs_engine.memory_controller->emc[mob_line->processor_id].memory_access_counter_table[index]>=MACT_THRESHOLD){
            //add statistics do preditor		
            mob_line->emc_predict_access_ram = true;
        }else{
            mob_line->emc_predict_access_ram = false;
        }
        //========================================= 
        hit = this->LLC_data_cache[index_llc].read(mob_line->memory_address,ttc); 
        if(hit == HIT){ 
        latency_request+=ttc;
        #if CACHE_MANAGER_DEBUG
            if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
                ORCS_PRINTF("HIT LLC_FROM_EMC ttc:[%u] Latency Request: [%u]\n",ttc,latency_request);
            }
        #endif
            // ===================================
            // marcando access llc emc
            orcs_engine.memory_controller->emc[mob_line->processor_id].add_access_LLC_Hit();
            mob_line->emc_generate_miss=false;
            // ===================================
        }else{
            latency_request += orcs_engine.memory_controller->requestDRAM();
            #if CACHE_MANAGER_DEBUG
                if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
                    ORCS_PRINTF("To RAM ttc:[%u] Latency Request: [%u]\n",ttc,latency_request);
                }
            #endif
            // ===================================
            orcs_engine.memory_controller->emc[mob_line->processor_id].add_access_LLC_Miss();
            mob_line->waiting_DRAM = true;
            mob_line->emc_generate_miss=true;
            // ===================================
            linha_t *linha_llc = this->LLC_data_cache[index_llc].installLine(mob_line->memory_address,latency_request);
            linha_t *linha_emc = orcs_engine.memory_controller->data_cache->installLine(mob_line->memory_address,latency_request);
            // linking emc and llc
            linha_llc->linha_ptr_emc = linha_emc;
            linha_emc->linha_ptr_llc = linha_llc;
            orcs_engine.memory_controller->add_requests_emc();//number of requests made by emc
            linha_emc=NULL;
        }
    }
    return latency_request;
}
int32_t cache_manager_t::generate_index_array(uint32_t processor_id,cacheLevel_t level){
    switch (level){
        case INST_CACHE:{
            return processor_id & (SIZE_OF_L1_CACHES_ARRAY-1);
        }
        break;
        case L1:{
            return processor_id & (SIZE_OF_L1_CACHES_ARRAY-1);
        }    
        break;
        case L2:{
            return processor_id & (SIZE_OF_L2_CACHES_ARRAY-1);
        }    
        break;
        case LLC:{
            return processor_id & (SIZE_OF_LLC_CACHES_ARRAY-1);
        }    
        break;
        case EMC_DATA_CACHE:{
            ERROR_PRINTF("Invalid call generate index");
            }
        break;
    }
    return POSITION_FAIL;
}
void cache_manager_t::statistics(uint32_t core_id){
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
    this->inst_cache[this->generate_index_array(core_id,INST_CACHE)].statistics();
    // ORCS_PRINTF("##############  Data Cache L1 ##################\n")
    this->L1_data_cache[this->generate_index_array(core_id,L1)].statistics();
    // ORCS_PRINTF("##############  LLC Cache ##################\n")
    this->L2_data_cache[this->generate_index_array(core_id,L2)].statistics();
    // ORCS_PRINTF("##############  LLC Cache ##################\n")
    this->LLC_data_cache[this->generate_index_array(core_id,LLC)].statistics();
    // Prefetcher
    #if PREFETCHER_ACTIVE
    this->prefetcher->statistics();
    #endif
}