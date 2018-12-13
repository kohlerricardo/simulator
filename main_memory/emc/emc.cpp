#include "../../simulator.hpp"
emc_t::emc_t()
{
	//alocate uop buffer
	this->uop_buffer = NULL;
	//allocate lsq
	this->unified_lsq = NULL;
	//allocate structures to fus
	this->fu_int_alu = NULL;
	this->fu_mem_load = NULL;
	this->fu_mem_store = NULL;
	// Data Cache
	this->data_cache = NULL;
	//MACT
	this->memory_access_counter_table = NULL;
	this->mact_bits_mask = 0;
	
};
emc_t::~emc_t()
{
	//deletting data cache
	if (this->data_cache != NULL)
		delete this->data_cache;
	// deleting fus
	utils_t::template_delete_array<uint64_t>(this->fu_int_alu);
	utils_t::template_delete_array<uint64_t>(this->fu_mem_load);
	utils_t::template_delete_array<uint64_t>(this->fu_mem_store);
	// deleting deps array
	for (size_t i = 0; i < EMC_UOP_BUFFER; i++)
	{
		utils_t::template_delete_array<emc_opcode_package_t>(this->uop_buffer[i].reg_deps_ptr_array[0]);
	}
	// deleting emc_opcode uop buffer
	utils_t::template_delete_array<emc_opcode_package_t>(this->uop_buffer);
	// delete load store queue
	utils_t::template_delete_array<memory_order_buffer_line_t>(this->unified_lsq);
	// MACT dellocate
	utils_t::template_delete_array<int8_t>(this->memory_access_counter_table);
};
// ============================================================================
// @allocate objects to EMC
void emc_t::allocate()
{
	// ======================= Unified FUs =======================
	this->unified_fus.reserve(EMC_UOP_BUFFER);
	//=======================  Unified RS =======================
	this->unified_rs.reserve(EMC_UOP_BUFFER);
	// ======================= data cache =======================
	this->data_cache = new cache_t;
	this->data_cache->allocate(EMC_DATA_CACHE);
	// ======================= alocate uop buffer =======================
	this->uop_buffer_end = 0;
	this->uop_buffer_start = 0;
	this->uop_buffer_used = 0;
	this->uop_buffer = utils_t::template_allocate_array<emc_opcode_package_t>(EMC_UOP_BUFFER);
	for (size_t i = 0; i < EMC_UOP_BUFFER; i++)
	{
		this->uop_buffer[i].reg_deps_ptr_array = utils_t::template_allocate_initialize_array<emc_opcode_package_t *>(ROB_SIZE, NULL);
	}
	// ======================= allocate lsq =======================
	this->unified_lsq = utils_t::template_allocate_array<memory_order_buffer_line_t>(EMC_LSQ_SIZE);
	// ======================= allocate structures to fus =======================
	this->fu_int_alu = utils_t::template_allocate_initialize_array<uint64_t>(EMC_INTEGER_ALU, 0);
	this->fu_mem_load = utils_t::template_allocate_initialize_array<uint64_t>(LOAD_UNIT, 0);
	this->fu_mem_store = utils_t::template_allocate_initialize_array<uint64_t>(STORE_UNIT, 0);
	// ======================= Memory Ops executed =======================
	this->memory_op_executed = 0;
	// ======================= execute control =======================
	this->ready_to_execute = false;
	this->executed = false;
	// MACT Allocate
	this->memory_access_counter_table = utils_t::template_allocate_initialize_array<int8_t>(MACT_SIZE,0);
	this->mact_bits_mask = utils_t::get_power_of_two(MACT_SIZE);
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
	ERROR_ASSERT_PRINTF(this->uop_buffer_used > 0, "Removendo do UOP Buffer sem estar usado\n")
	ERROR_ASSERT_PRINTF(this->uop_buffer[this->uop_buffer_start].reg_deps_ptr_array[0] == NULL, "Removendo sem resolver dependencias\n")
	this->uop_buffer[this->uop_buffer_start].package_clean();
	this->uop_buffer_used--;
	this->uop_buffer_start++;
	if (this->uop_buffer_start >= EMC_UOP_BUFFER)
	{
		this->uop_buffer_start = 0;
	}
};
// ============================================================================
void emc_t::emc_dispatch(){	
	#if EMC_DISPATCH_DEBUG
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
		ORCS_PRINTF("====================== EMC Dispatch Stage =========================\n")
		ORCS_PRINTF("Cycle %lu\n",orcs_engine.get_global_cycle())
		}
	#endif
	uint32_t uop_dispatched = 0;
	uint32_t fu_int_alu = 0;
	uint32_t fu_mem_load = 0;
	uint32_t fu_mem_store = 0;

	for (uint32_t i = 0; i < this->unified_rs.size() && i < EMC_UNIFIED_RS; i++)
	{
		emc_opcode_package_t *emc_opcode = this->unified_rs[i];

		
		if (uop_dispatched >= EMC_DISPATCH_WIDTH)
		{
			break;
		}
		if(emc_opcode == NULL){
			break;
		}
		if ((emc_opcode->rob_ptr !=NULL)&&(emc_opcode->rob_ptr->original_miss == true))
		{
			this->unified_rs.erase(this->unified_rs.begin() + i);
			i--;
			continue;
		}

	#if EMC_DISPATCH_DEBUG
			if (orcs_engine.get_global_cycle() > WAIT_CYCLE)
			{
				ORCS_PRINTF("EMC Trying dispatch %s\n", emc_opcode->content_to_string().c_str())
			}
	#endif
		if ((emc_opcode->uop.readyAt <= orcs_engine.get_global_cycle()) && (emc_opcode->wait_reg_deps_number == 0))
		{
			ERROR_ASSERT_PRINTF(emc_opcode->stage == PROCESSOR_STAGE_RENAME, "Error, EMC uop not renamed\n")
			bool dispatched = false;
			switch (emc_opcode->uop.uop_operation){
				// NOP operation
				case INSTRUCTION_OPERATION_NOP:
				// integer alu// add/sub/logical
				case INSTRUCTION_OPERATION_INT_ALU:
				// branch op. como fazer, branch solved on fetch
				case INSTRUCTION_OPERATION_BRANCH:
				// op not defined
				case INSTRUCTION_OPERATION_OTHER:
					if (fu_int_alu < EMC_INTEGER_ALU)
					{
						for (uint8_t k = 0; k < EMC_INTEGER_ALU; k++)
						{
							if (this->fu_int_alu[k] <= orcs_engine.get_global_cycle())
							{
								this->fu_int_alu[k] = orcs_engine.get_global_cycle() + WAIT_NEXT_INT_ALU;
								fu_int_alu++;
								dispatched = true;
								emc_opcode->stage = PROCESSOR_STAGE_EXECUTION;
								emc_opcode->uop.updatePackageReady(LATENCY_INTEGER_ALU);
								break;
							}
						}
					}
					break;
				// ====================================================
				// Operation LOAD
				case INSTRUCTION_OPERATION_MEM_LOAD:
					if (fu_mem_load < LOAD_UNIT)
					{
						for (uint8_t k = 0; k < LOAD_UNIT; k++)
						{
							if (this->fu_mem_load[k] <= orcs_engine.get_global_cycle())
							{
								this->fu_mem_load[k] = orcs_engine.get_global_cycle() + WAIT_NEXT_MEM_LOAD;
								fu_mem_load++;
								dispatched = true;
								emc_opcode->stage = PROCESSOR_STAGE_EXECUTION;
								emc_opcode->uop.updatePackageReady(LATENCY_MEM_LOAD);
								break;
							}
						}
					}
					break;
				// ====================================================
				// Operation STORE
				case INSTRUCTION_OPERATION_MEM_STORE:
					if (fu_mem_store < STORE_UNIT)
					{
						for (uint8_t k = 0; k < STORE_UNIT; k++)
						{
							if (this->fu_mem_store[k] <= orcs_engine.get_global_cycle())
							{
								this->fu_mem_store[k] = orcs_engine.get_global_cycle() + WAIT_NEXT_MEM_STORE;
								fu_mem_store++;
								dispatched = true;
								emc_opcode->stage = PROCESSOR_STAGE_EXECUTION;
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
				} //end switch case
			if (dispatched == true)
			{
				#if EMC_DISPATCH_DEBUG
					if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
						ORCS_PRINTF("EMC Dispatched %s\n", emc_opcode->content_to_string().c_str())
					}
				#endif
				uop_dispatched++;
				// insert on FUs waiting structure
				this->unified_fus.push_back(emc_opcode);
				// remove from reservation station
				this->unified_rs.erase(this->unified_rs.begin() + i);
				i--;
			} //end if dispatched
		}	 //end if pode despachar
	}		  //end for
};
// ============================================================================
void emc_t::emc_execute(){
	#if EMC_EXECUTE_DEBUG
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
		ORCS_PRINTF("======================== EMC Execute Stage ===========================\n")
		ORCS_PRINTF("Cycle %lu\n",orcs_engine.get_global_cycle())
		}
	#endif
	// ==================================
	// verificar leituras prontas no ciclo,
	// remover do MOB e atualizar os registradores,
	// ==================================
	for (uint32_t i = 0; i < EMC_LSQ_SIZE; i++)
	{
		if (this->unified_lsq[i].status == PACKAGE_STATE_READY &&
			this->unified_lsq[i].readyAt <= orcs_engine.get_global_cycle() &&
			this->unified_lsq[i].processed == false){
			#if EMC_EXECUTE_DEBUG 
			if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
				ORCS_PRINTF("EMC Memory Solving %s\n", this->unified_lsq[i].emc_opcode_ptr->content_to_string().c_str())
			}
			#endif
			ERROR_ASSERT_PRINTF(this->unified_lsq[i].uop_executed == true, "Removing memory read before being executed.\n")
			ERROR_ASSERT_PRINTF(this->unified_lsq[i].wait_mem_deps_number == 0, "Number of memory dependencies should be zero.\n")
			this->unified_lsq[i].emc_opcode_ptr->stage = PROCESSOR_STAGE_COMMIT;
			this->unified_lsq[i].emc_opcode_ptr->uop.updatePackageReady(EMC_COMMIT_LATENCY);
			this->unified_lsq[i].processed = true;
			// =============================================================
			// solving register dependence EMC !!!!!!!!!!!!
			this->solve_emc_dependencies(this->unified_lsq[i].emc_opcode_ptr);
			// =============================================================
		} //end if
	}	 //end for lsq
	uint32_t uop_total_executed = 0;
	for (size_t i = 0; i < this->unified_fus.size(); i++)
	{
		emc_opcode_package_t *emc_package = this->unified_fus[i];
		if (uop_total_executed >= EMC_EXECUTE_WIDTH)
		{
			break;
		}
		if(emc_package == NULL){
			break;
		}
		if (emc_package->uop.readyAt <= orcs_engine.get_global_cycle())
		{
	#if EMC_EXECUTE_DEBUG
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("EMC Trying Execute %s\n", emc_package->content_to_string().c_str())
		}
	#endif
			ERROR_ASSERT_PRINTF(emc_package->uop.status == PACKAGE_STATE_READY, "FU with Package not in ready state")
			ERROR_ASSERT_PRINTF(emc_package->stage == PROCESSOR_STAGE_EXECUTION, "FU with Package not in execution stage")
			switch (emc_package->uop.uop_operation)
			{
			// =============================================================
			// BRANCHES
			case INSTRUCTION_OPERATION_BRANCH:
			// INTEGERS ===============================================
			case INSTRUCTION_OPERATION_INT_ALU:
			case INSTRUCTION_OPERATION_NOP:
			case INSTRUCTION_OPERATION_OTHER:{
				emc_package->stage = PROCESSOR_STAGE_COMMIT;
				emc_package->uop.updatePackageReady(1);
				this->solve_emc_dependencies(emc_package);
				uop_total_executed++;
				/// Remove from the Functional Units
				this->unified_fus.erase(this->unified_fus.begin() + i);
				i--;
				emc_package->rob_ptr->emc_executed = true;
			}
			break;
			// MEMORY LOAD/STORE ==========================================
			case INSTRUCTION_OPERATION_MEM_LOAD:{
				ERROR_ASSERT_PRINTF(emc_package->mob_ptr != NULL, "Read with a NULL pointer to MOB")
				this->memory_op_executed++;
				//Atualizando operacoes executadas, pois senão trava o envio de operacoes load
				orcs_engine.processor->memory_read_executed++;
				//
				emc_package->mob_ptr->uop_executed = true;
				emc_package->uop.updatePackageReady(EXECUTE_LATENCY);
				uop_total_executed++;
				/// Remove from the Functional Units
				this->unified_fus.erase(this->unified_fus.begin() + i);
				i--;
				emc_package->rob_ptr->emc_executed = true;
			}
			break;
			case INSTRUCTION_OPERATION_MEM_STORE:{
				ERROR_ASSERT_PRINTF(emc_package->mob_ptr != NULL, "Write with a NULL pointer to MOB")
				this->memory_op_executed++;
				//Atualizando operacoes executadas, pois senão trava o envio de operacoes store
				orcs_engine.processor->memory_write_executed++;
				emc_package->mob_ptr->uop_executed = true;
				emc_package->uop.updatePackageReady(EXECUTE_LATENCY);
				uop_total_executed++;
				/// Remove from the Functional Units
				this->unified_fus.erase(this->unified_fus.begin() + i);
				i--;
				emc_package->rob_ptr->mob_ptr->uop_executed = true;
				emc_package->rob_ptr->emc_executed = true;
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
			} //end switch
		}//end if ready package
		#if EMC_EXECUTE_DEBUG
				if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
					ORCS_PRINTF("EMC Executed %s\n", emc_package->content_to_string().c_str())
				}
		#endif
	}		  //end for
	if (this->memory_op_executed > 0)
	{
		this->lsq_read();
	}
};
// ============================================================================
void emc_t::emc_commit(){
	#if EMC_COMMIT_DEBUG
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
		ORCS_PRINTF("======================== EMC Commit Stage ===========================\n")
		ORCS_PRINTF("Cycle %lu\n",orcs_engine.get_global_cycle())
		}
	#endif
	for (uint32_t i = 0; i < EMC_COMMIT_WIDTH; i++){
		int8_t pos_buffer = this->uop_buffer_start;
		if (this->uop_buffer_used != 0 &&
			this->uop_buffer[pos_buffer].stage == PROCESSOR_STAGE_COMMIT &&
			this->uop_buffer[pos_buffer].uop.status == PACKAGE_STATE_READY &&
			this->uop_buffer[pos_buffer].uop.readyAt <= orcs_engine.get_global_cycle())
		{
			ERROR_ASSERT_PRINTF(uint32_t(pos_buffer) == this->uop_buffer_start, "EMC sending different position from start\n");
			this->emc_send_back_core(&this->uop_buffer[pos_buffer]);
			this->remove_front_uop_buffer();
		}
		else
		{
			break;
		}
	}
	#if EMC_COMMIT_DEBUG
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			this->print_structures();
		}
	#endif
};
// ============================================================================
void emc_t::solve_emc_dependencies(emc_opcode_package_t *emc_opcode){
	#if EMC_EXECUTE_DEBUG
	if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
		ORCS_PRINTF("Solving %s\n", emc_opcode->content_to_string().c_str())
	}
	#endif
	// =========================================================================
	// SOLVE REGISTER DEPENDENCIES - RRT
	// =========================================================================
	for (uint32_t j = 0; j < ROB_SIZE; j++)
	{
		/// There is an unsolved dependency
		if (emc_opcode->reg_deps_ptr_array[j] != NULL)
		{
			emc_opcode->wake_up_elements_counter--;
			emc_opcode->reg_deps_ptr_array[j]->wait_reg_deps_number--;
			
			if(emc_opcode->reg_deps_ptr_array[j]->uop.readyAt <= orcs_engine.get_global_cycle())
			{
				emc_opcode->reg_deps_ptr_array[j]->uop.readyAt = orcs_engine.get_global_cycle();
			}
			emc_opcode->reg_deps_ptr_array[j] = NULL;
		}
		/// All the dependencies are solved
		else
		{
			break;
		}
	}
};
// ============================================================================
void emc_t::clock()
{
	#if EMC_DEBUG
	if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
		ORCS_PRINTF("================================= EMC ====================================\n")
		ORCS_PRINTF("Cycle %lu\n",orcs_engine.get_global_cycle())
	}
	#endif
	if (this->ready_to_execute)
	{
		#if EMC_DEBUG
		if(this->executed){
			this->executed = false;
			if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
				ORCS_PRINTF("Chain to execute %lu\n",orcs_engine.get_global_cycle())
				this->print_structures();
			}
		}
		#endif
		if(this->uop_buffer_used>0){
			this->emc_commit();
			this->emc_execute();
			this->emc_dispatch();
		}else{
			this->ready_to_execute=false;
			#if EMC_DEBUG
				ORCS_PRINTF("\t\t EMC Unlocking Processor %lu\n",orcs_engine.get_global_cycle())
			#endif
			orcs_engine.processor->lock_processor=false;
		}
	}
};
// ============================================================================
void emc_t::update_mact_entry(uint64_t pc,int32_t value){
	uint64_t index  = utils_t::hash_function(HASH_FUNCTION_INPUT1_ONLY,pc,0,this->mact_bits_mask);
	this->memory_access_counter_table[index]+=value;
	if(this->memory_access_counter_table[index]>7){
		this->memory_access_counter_table[index]=0;	
	}else if(this->memory_access_counter_table[index]<0){
		this->memory_access_counter_table[index]=7;
	}
};
// ============================================================================
void emc_t::lsq_read(){
	#if EMC_LSQ_DEBUG
	if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
		ORCS_PRINTF("======================== EMC LSQ Operation ===========================\n")
	}
	#endif
	int32_t position_mem = POSITION_FAIL;
	memory_order_buffer_line_t *emc_mob_line = NULL;

	position_mem = memory_order_buffer_line_t::find_old_request_state_ready(this->unified_lsq, EMC_LSQ_SIZE, PACKAGE_STATE_WAIT);
	if (position_mem != POSITION_FAIL){
		emc_mob_line = &this->unified_lsq[position_mem];
	}
	#if EMC_LSQ_DEBUG
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("Position On EMC LSQ catch %d\n",position_mem)
		}
	#endif
	if (emc_mob_line != NULL){
	#if EMC_LSQ_DEBUG
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("Mem Op %s\n", emc_mob_line->content_to_string().c_str())
		}
	#endif
		if (emc_mob_line->memory_operation == MEMORY_OPERATION_READ){
			// ==========================================
			//Utilizando o miss predictor 
			uint64_t index = utils_t::hash_function(HASH_FUNCTION_INPUT1_ONLY,emc_mob_line->emc_opcode_ptr->uop.opcode_address,0,this->mact_bits_mask);
			if(this->memory_access_counter_table[index]>=MACT_THRESHOLD){
				//add statistics do preditor				
				this->add_direct_ram_access();
				this->update_mact_entry(emc_mob_line->opcode_address,1);
			}
			// ==========================================
			uint32_t ttc = 0;
			ttc = orcs_engine.cacheManager->search_EMC_Data(emc_mob_line); //enviar que é do emc
			if(ttc<RAM_LATENCY){
				this->add_incorrect_prediction_ram_access();
				this->update_mact_entry(emc_mob_line->opcode_address,-1);
			}
			emc_mob_line->updatePackageReady(ttc);
			emc_mob_line->emc_executed=true;
			emc_mob_line->sent=true;
			ERROR_ASSERT_PRINTF(this->memory_op_executed > 0,"Erro, tentando reduzir MEM_Operation executado abaixo de zero")
			this->memory_op_executed--;
			// ==========================================
		}
		else{
			uint32_t ttc = 1;
			// grava no lsq
			emc_mob_line->updatePackageReady(ttc);
			emc_mob_line->emc_opcode_ptr->uop.updatePackageReady(ttc);
			ERROR_ASSERT_PRINTF(this->memory_op_executed > 0,"Erro, tentando reduzir MEM_Operation executado abaixo de zero")
			this->memory_op_executed--;
		}
	} //end if mob_line null
};	//end method
// ============================================================================
void emc_t::statistics(){
	FILE *output = stdout;

	if(orcs_engine.output_file_name != NULL)
		output = fopen(orcs_engine.output_file_name,"a+");
	if (output != NULL){
		utils_t::largestSeparator(output);
		fprintf(output, "EMC - Enhaced Memory Controller\n");
		utils_t::largestSeparator(output);
		fprintf(output, "EMC_Access_LLC: %lu\n", this->get_access_LLC());
		fprintf(output, "EMC_Access_LLC_HIT: %lu\n", this->get_access_LLC_Hit());
		fprintf(output, "EMC_Access_LLC_HIT: %lu\n", this->get_access_LLC_Hit());
		fprintf(output, "EMC_Direct_RAM_ACCESS_Predictor: %lu\n", this->get_direct_ram_access());
		fprintf(output, "EMC_Incorrect_Prediction: %lu\n", this->get_incorrect_prediction_ram_access());

		utils_t::largestSeparator(output);
		fprintf(output, "##############  EMC_Data_Cache ##################\n");
		this->data_cache->statistics();
		utils_t::largestSeparator(output);
		}
	fclose(output);
};
// ============================================================================
void emc_t::emc_send_back_core(emc_opcode_package_t *emc_opcode){
	reorder_buffer_line_t *rob_line = emc_opcode->rob_ptr;
	#if EMC_COMMIT_DEBUG
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("==============================\n")
			ORCS_PRINTF("Sending %s\n\n", emc_opcode->content_to_string().c_str())
			ORCS_PRINTF("To %s\n\n",rob_line->content_to_string().c_str())
		}
	#endif
	// Atualizar opcodes de acesso a memoria
	if(emc_opcode->uop.uop_operation==INSTRUCTION_OPERATION_INT_ALU ||
		emc_opcode->uop.uop_operation==INSTRUCTION_OPERATION_BRANCH){
			rob_line->uop = emc_opcode->uop;
			rob_line->stage = emc_opcode->stage;
			rob_line->is_poisoned = false;
			orcs_engine.processor->solve_registers_dependency(rob_line);

		}else{
			// rob_line->uop = emc_opcode->uop;
			*(rob_line->mob_ptr) = *(emc_opcode->mob_ptr);
			rob_line->mob_ptr->processed = false;
			rob_line->is_poisoned = false;
		}
	#if EMC_COMMIT_DEBUG
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("Rob Line after send %s\n", rob_line->content_to_string().c_str())
			ORCS_PRINTF("==============================\n\n")
		}
	#endif
	//Remove from reservation station
	// 	auto itr = std::find_if(orcs_engine.processor->unified_reservation_station.begin(), orcs_engine.processor->unified_reservation_station.end(),
	// 	[rob_line](reorder_buffer_line_t* v) {return rob_line==v;});
	// 	if (itr != orcs_engine.processor->unified_reservation_station.cend()) {
	// 		uint32_t distance = std::distance(orcs_engine.processor->unified_reservation_station.begin(), itr);
	// 		std::cout << "Distance: "<<distance << "\n";
	// 		orcs_engine.processor->unified_reservation_station.erase(itr);
	// }
}
void emc_t::print_structures(){
	ORCS_PRINTF("UopBuffer used %d\n",this->uop_buffer_used)
	for (uint32_t i = this->uop_buffer_start;; i++)
	{
		// sleep(1);
		if(i>=EMC_UOP_BUFFER)i=0;
		if(i==this->uop_buffer_end)break;
		ORCS_PRINTF("%s\n",this->uop_buffer[i].content_to_string().c_str())
	}

	// memory_order_buffer_line_t::printAll(this->unified_lsq,EMC_LSQ_SIZE);
}