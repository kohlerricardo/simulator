#include "../../simulator.hpp"
emc_t::emc_t(){
    //alocate uop buffer
    this->uop_buffer = NULL;
    //allocate lsq 
    this->unified_lsq = NULL;
	//allocate structures to fus
	this->fu_int_alu = NULL;
	this->fu_mem_load = NULL;
	this->fu_mem_store = NULL;
};
emc_t::~emc_t(){
    if(this->data_cache !=NULL) delete this->data_cache;
	utils_t::template_delete_array<uint64_t>(this->fu_int_alu);
	utils_t::template_delete_array<uint64_t>(this->fu_mem_load);
	utils_t::template_delete_array<uint64_t>(this->fu_mem_store);
};
// ============================================================================
// @allocate objects to EMC
void emc_t::allocate(){
    // Unified FUs
    this->unified_fus.reserve(EMC_UOP_BUFFER);
    // Unified RS
    this->unified_rs.reserve(EMC_UOP_BUFFER);
    // data cache
    this->data_cache->allocate(EMC_DATA_CACHE);
    //alocate uop buffer
    this->uop_buffer = new emc_opcode_package_t[EMC_UOP_BUFFER];
    //allocate lsq 
    this->unified_lsq = new memory_order_buffer_line_t[EMC_LSQ_SIZE];
	//allocate structures to fus
	this->fu_int_alu = utils_t::template_allocate_initialize_array<uint64_t>(EMC_INTEGER_ALU,0);
	this->fu_mem_load = utils_t::template_allocate_initialize_array<uint64_t>(LOAD_UNIT, 0);
	this->fu_mem_store = utils_t::template_allocate_initialize_array<uint64_t>(STORE_UNIT, 0);
	// Memory Ops executed
	this->memory_read_executed = 0;
	this->memory_write_executed = 0;
};
// ============================================================================
void emc_t::statistics(){
    if(orcs_engine.output_file_name == NULL){
        utils_t::largestSeparator();
        ORCS_PRINTF("EMC - Enhaced Memory Controller")
        utils_t::largestSeparator();
    }else{
        FILE *output = fopen(orcs_engine.output_file_name,"a+");
		if(output != NULL){
        utils_t::largestSeparator(output);
        fprintf(output,"EMC - Enhaced Memory Controller");
        utils_t::largestSeparator(output);
        }
    }
};
// ============================================================================
int32_t emc_t::get_position_uop_buffer(){
    
	int32_t position = POSITION_FAIL;
	/// There is free space.
	if (this->uop_buffer_used < EMC_UOP_BUFFER)
	{
		position = this->uop_buffer_end;
		this->uop_buffer_used++;
		this->uop_buffer_end++;
		if (this->uop_buffer_end >= EMC_UOP_BUFFER)
		{
			this->uop_buffer_end = 0;
		}
	}
	return position;
};
// ============================================================================
void emc_t::remove_front_uop_buffer(){
	ERROR_ASSERT_PRINTF(this->uop_buffer_used > 0, "Removendo do ROB sem estar usado\n")
	ERROR_ASSERT_PRINTF(this->uop_buffer[this->uop_buffer_start].reg_deps_ptr_array[0] == NULL, "Removendo sem resolver dependencias\n")
	this->uop_buffer[this->uop_buffer_start].package_clean();
	this->uop_buffer_used--;
	this->uop_buffer_start++;
	if (this->uop_buffer_start >= ROB_SIZE)
	{
		this->uop_buffer_start = 0;
	}
};
// ============================================================================
void emc_t::dispatch(){
    uint32_t uop_dispatched = 0;
    uint32_t fu_int_alu = 0;
	uint32_t fu_mem_load = 0;
	uint32_t fu_mem_store = 0;

    for (uint32_t i = 0; i < this->unified_rs.size() && i < EMC_UNIFIED_RS; i++)
    {
        emc_opcode_package_t *emc_opcode = this->unified_rs[i];
        if(uop_dispatched == EMC_DISPATCH_WIDTH){
            break;
        }
        if((emc_opcode->uop.readyAt <= orcs_engine.get_global_cycle())&&
			(emc_opcode->wait_reg_deps_number == 0)){
            bool dispatched=false;
            
            switch (emc_opcode->uop.uop_operation){   
                // NOP operation
				case INSTRUCTION_OPERATION_NOP:
				// integer alu// add/sub/logical
				case INSTRUCTION_OPERATION_INT_ALU:
				// branch op. como fazer, branch solved on fetch
				case INSTRUCTION_OPERATION_BRANCH:
				// op not defined
				case INSTRUCTION_OPERATION_OTHER:
                    if(fu_int_alu < EMC_INTEGER_ALU){
						for (uint8_t k = 0; k < EMC_INTEGER_ALU; k++){
							if(this->fu_int_alu[k]<=orcs_engine.get_global_cycle()){
							this->fu_int_alu[k]= orcs_engine.get_global_cycle()+WAIT_NEXT_INT_ALU;
							fu_int_alu++;
							dispatched=true;
							// emc_opcode->stage = PROCESSOR_STAGE_EXECUTION;
							emc_opcode->uop.updatePackageReady(LATENCY_INTEGER_ALU);
							break;
							}
						}
					}
                    break;
                // ====================================================
				// Operation LOAD
				case INSTRUCTION_OPERATION_MEM_LOAD:
					if(fu_mem_load < LOAD_UNIT){
						for(uint8_t k= 0;k < LOAD_UNIT;k++){
							if(this->fu_mem_load[k] <= orcs_engine.get_global_cycle()){
								this->fu_mem_load[k]=orcs_engine.get_global_cycle()+WAIT_NEXT_MEM_LOAD;
								fu_mem_load++;
								dispatched=true;
								// emc_opcode->stage=PROCESSOR_STAGE_EXECUTION;
								emc_opcode->uop.updatePackageReady(LATENCY_MEM_LOAD);
								break;
							}
						}
					}
				break;
			
				// ====================================================
				// Operation STORE
				case INSTRUCTION_OPERATION_MEM_STORE:
					if(fu_mem_store < STORE_UNIT){
						for(uint8_t k= 0;k < STORE_UNIT;k++){
							if(this->fu_mem_store[k]<=orcs_engine.get_global_cycle()){
								this->fu_mem_store[k]=orcs_engine.get_global_cycle()+WAIT_NEXT_MEM_STORE;
								fu_mem_store++;
								dispatched=true;
								// emc_opcode->stage=PROCESSOR_STAGE_EXECUTION;
								emc_opcode->uop.updatePackageReady(LATENCY_MEM_STORE);
								break;
							}
						}
					}
				break;
                case INSTRUCTION_OPERATION_BARRIER:
				case INSTRUCTION_OPERATION_HMC_ROA:
				case INSTRUCTION_OPERATION_HMC_ROWA:
                case INSTRUCTION_OPERATION_INT_MUL:
                case INSTRUCTION_OPERATION_INT_DIV:
                case INSTRUCTION_OPERATION_FP_ALU:
                case INSTRUCTION_OPERATION_FP_MUL:
                case INSTRUCTION_OPERATION_FP_DIV:
				ERROR_PRINTF("Invalid instruction being dispatched.\n");
				break;
            }//end switch case
            if(dispatched){
                uop_dispatched++;
				// insert on FUs waiting structure
				this->unified_fus.push_back(emc_opcode);
				// remove from reservation station
				this->unified_rs.erase(this->unified_rs.begin()+i);
				i--;
            }//end 
        }//end if pode despachar
    } //end for

};
// ============================================================================
void emc_t::execute(){
    // ==================================
	// verificar leituras prontas no ciclo,
	// remover do MOB e atualizar os registradores, 
	// ==================================
	for (uint32_t i=0 ;i<EMC_LSQ_SIZE;i++){
		if(this->unified_lsq[i].status == PACKAGE_STATE_READY && 
			this->unified_lsq[i].readyAt <=orcs_engine.get_global_cycle()){
			ERROR_ASSERT_PRINTF(this->unified_lsq[i].uop_executed == true, "Removing memory read before being executed.\n")
			ERROR_ASSERT_PRINTF(this->unified_lsq[i].wait_mem_deps_number == 0, "Number of memory dependencies should be zero.\n")
			// this->unified_lsq[i].rob_ptr->stage=PROCESSOR_STAGE_COMMIT;
			// this->unified_lsq[i].rob_ptr->uop.updatePackageReady(COMMIT_LATENCY);
			// this->unified_lsq[i].rob_ptr->mob_ptr=NULL;
			#if EXECUTE_DEBUG
			ORCS_PRINTF("Solving %s\n",this->unified_lsq[entry].rob_ptr->content_to_string().c_str())
			#endif
			// solving register dependence !!!!!!!!!!!!
			// this->solve_registers_dependency(this->unified_lsq[i].rob_ptr);
			this->unified_lsq[i].package_clean();
		}//end if
	}//end for lsq
    uint32_t uop_total_executed = 0;
	for (size_t i = 0; i < this->unified_fus.size(); i++){
        emc_opcode_package_t *emc_package = this->unified_fus[i];
		if(uop_total_executed == EXECUTE_WIDTH){
			break;
		}
		if(emc_package == NULL){
			break;
		}
		if(emc_package->uop.readyAt<=orcs_engine.get_global_cycle()){
			#if EXECUTE_DEBUG
				ORCS_PRINTF("Trying Execute %s\n",emc_opcode->content_to_string().c_str())
			#endif
			ERROR_ASSERT_PRINTF(emc_package->uop.status == PACKAGE_STATE_READY,"FU with Package not in ready state")
			switch(emc_package->uop.uop_operation){
				 // =============================================================
                // BRANCHES
                case INSTRUCTION_OPERATION_BRANCH:
                // INTEGERS ===============================================
                case INSTRUCTION_OPERATION_INT_ALU:
                case INSTRUCTION_OPERATION_NOP:
                case INSTRUCTION_OPERATION_OTHER:
				{
                    // emc_opcode->stage = PROCESSOR_STAGE_COMMIT;
                    // emc_opcode->uop.updatePackageReady(EXECUTE_LATENCY+COMMIT_LATENCY);
                    // this->solve_registers_dependency(emc_opcode);
                    uop_total_executed++;
                    /// Remove from the Functional Units
                    this->unified_fus.erase(this->unified_fus.begin() + i);
                    i--;
				}
                break;
				// MEMORY LOAD/STORE ==========================================
                case INSTRUCTION_OPERATION_MEM_LOAD:
                {
                    ERROR_ASSERT_PRINTF(emc_package->mob_ptr != NULL, "Read with a NULL pointer to MOB")
                    this->memory_read_executed++;
					emc_package->mob_ptr->uop_executed = true;
                    emc_package->uop.updatePackageReady(EXECUTE_LATENCY);
                    uop_total_executed++;
                    /// Remove from the Functional Units
                    this->unified_fus.erase(this->unified_fus.begin() + i);
                    i--;
                }
                break;
                case INSTRUCTION_OPERATION_MEM_STORE:
                {
                    ERROR_ASSERT_PRINTF(emc_package->mob_ptr != NULL, "Write with a NULL pointer to MOB")
                    this->memory_write_executed++;
					emc_package->mob_ptr->uop_executed = true;
                    /// Waits for the cache to receive the package
                    emc_package->uop.updatePackageReady(EXECUTE_LATENCY);
                    uop_total_executed++;
                    /// Remove from the Functional Units
                    this->unified_fus.erase(this->unified_fus.begin() + i);
                   	i--;
                }
               	break;
				case INSTRUCTION_OPERATION_BARRIER:
				case INSTRUCTION_OPERATION_HMC_ROA:
				case INSTRUCTION_OPERATION_HMC_ROWA:
                // INT OPs ===============================================
                case INSTRUCTION_OPERATION_INT_MUL:
                case INSTRUCTION_OPERATION_INT_DIV:
                // FLOAT POINT ===============================================
                case INSTRUCTION_OPERATION_FP_ALU:
                case INSTRUCTION_OPERATION_FP_MUL:
                case INSTRUCTION_OPERATION_FP_DIV:
					ERROR_PRINTF("Invalid instruction Executed.\n");
                	break;
			}//end switch
		} //end if ready package
	}//end for 
	if(this->memory_read_executed>0){
		//alterar cache logic
	}
	if(this->memory_write_executed>0){
		//alterar cache logic
	}
};
// ============================================================================
void emc_t::emc_to_core(){};
// ============================================================================
void emc_t::solve_emc_dependencies(emc_opcode_package_t *emc_opcode){
    /// Remove pointers from Register Alias Table (RAT)
    for (uint32_t j = 0; j < MAX_REGISTERS; j++) {
        if (emc_opcode->uop.write_regs[j] < 0) {
            break;
        }
        uint32_t write_register = emc_opcode->uop.write_regs[j];
        ERROR_ASSERT_PRINTF(write_register <RAT_SIZE, "Read Register (%d) > Register Alias Table Size (%d)\n",
                                                                            write_register, RAT_SIZE);
		if (this->register_alias_table[write_register] != NULL &&
        this->register_alias_table[write_register]->uop.uop_number == emc_opcode->uop.uop_number) {
            this->register_alias_table[write_register] = NULL;
	#if EXECUTE_DEBUG
			ORCS_PRINTF("register_%u\n",write_register)
	#endif
        }//end if
    }//end for

  	// =========================================================================
    // SOLVE REGISTER DEPENDENCIES - RAT
    // =========================================================================
    for (uint32_t j = 0; j < ROB_SIZE; j++) {
        /// There is an unsolved dependency
        if (emc_opcode->reg_deps_ptr_array[j] != NULL) {
			emc_opcode->wake_up_elements_counter--;
            emc_opcode->reg_deps_ptr_array[j]->wait_reg_deps_number--;
            /// This update the ready cycle, and it is usefull to compute the time each instruction waits for the functional unit
            if (emc_opcode->reg_deps_ptr_array[j]->uop.readyAt <= orcs_engine.get_global_cycle()) {
                emc_opcode->reg_deps_ptr_array[j]->uop.readyAt = orcs_engine.get_global_cycle();
            }
            emc_opcode->reg_deps_ptr_array[j] = NULL;
        }
        /// All the dependencies are solved
        else {
            break;
        }
	}
};
// ============================================================================
void emc_t::clock(){

}