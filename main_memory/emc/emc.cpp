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
};
// ============================================================================
int32_t emc_t::get_position_uop_buffer()
{

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
void emc_t::remove_front_uop_buffer()
{
	ERROR_ASSERT_PRINTF(this->uop_buffer_used > 0, "Removendo do UOP Buffer sem estar usado\n")
	ERROR_ASSERT_PRINTF(this->uop_buffer[this->uop_buffer_start].reg_deps_ptr_array[0] == NULL, "Removendo sem resolver dependencias\n")
	this->emc_send_back_core(&this->uop_buffer[this->uop_buffer_start]);
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
	uint32_t uop_dispatched = 0;
	uint32_t fu_int_alu = 0;
	uint32_t fu_mem_load = 0;
	uint32_t fu_mem_store = 0;

	for (uint32_t i = 0; i < this->unified_rs.size() && i < EMC_UNIFIED_RS; i++)
	{
		emc_opcode_package_t *emc_opcode = this->unified_rs[i];
		if (uop_dispatched == EMC_DISPATCH_WIDTH)
		{
			break;
		}
	#if EMC_DISPATCH_DEBUG
			ORCS_PRINTF("Clock %lu\n", orcs_engine.get_global_cycle())
			ORCS_PRINTF("EMC Trying dispatch %s\n", emc_opcode->content_to_string().c_str())
			sleep(1);
	#endif
		if ((emc_opcode->uop.readyAt <= orcs_engine.get_global_cycle()) &&
			(emc_opcode->wait_reg_deps_number == 0))
		{
			ERROR_ASSERT_PRINTF(emc_opcode->stage == PROCESSOR_STAGE_RENAME, "Error, EMC uop not renamed\n")
			bool dispatched = false;

			switch (emc_opcode->uop.uop_operation)
			{
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
			if (dispatched)
			{
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
	// ==================================
	// verificar leituras prontas no ciclo,
	// remover do MOB e atualizar os registradores,
	// ==================================
	for (uint32_t i = 0; i < EMC_LSQ_SIZE; i++)
	{
		if (this->unified_lsq[i].status == PACKAGE_STATE_READY &&
			this->unified_lsq[i].readyAt <= orcs_engine.get_global_cycle())
		{
			ERROR_ASSERT_PRINTF(this->unified_lsq[i].uop_executed == true, "Removing memory read before being executed.\n")
			ERROR_ASSERT_PRINTF(this->unified_lsq[i].wait_mem_deps_number == 0, "Number of memory dependencies should be zero.\n")
			this->unified_lsq[i].emc_opcode_ptr->stage=PROCESSOR_STAGE_COMMIT;
			this->unified_lsq[i].emc_opcode_ptr->uop.updatePackageReady(COMMIT_LATENCY);
			this->unified_lsq[i].emc_opcode_ptr->mob_ptr=NULL;
	#if EXECUTE_DEBUG
				ORCS_PRINTF("Solving %s\n", this->unified_lsq[entry].rob_ptr->content_to_string().c_str())
	#endif
			// solving register dependence !!!!!!!!!!!!
	#if EMC_ACTIVE
				this->solve_emc_dependencies(this->unified_lsq[i].emc_opcode_ptr);
	#endif
			this->unified_lsq[i].package_clean();
		} //end if
	}	 //end for lsq
	uint32_t uop_total_executed = 0;
	for (size_t i = 0; i < this->unified_fus.size(); i++)
	{
		emc_opcode_package_t *emc_package = this->unified_fus[i];
		if (uop_total_executed == EMC_EXECUTE_WIDTH)
		{
			break;
		}
		if (emc_package == NULL)
		{
			break;
		}
		if (emc_package->uop.readyAt <= orcs_engine.get_global_cycle())
		{
	#if EMC_EXECUTE_DEBUG
				ORCS_PRINTF("EMC Trying Execute %s\n", emc_package->content_to_string().c_str())
				sleep(1);
	#endif
			ERROR_ASSERT_PRINTF(emc_package->uop.status == PACKAGE_STATE_READY, "FU with Package not in ready state")
			switch (emc_package->uop.uop_operation)
			{
				// =============================================================
			// BRANCHES
			case INSTRUCTION_OPERATION_BRANCH:
			// INTEGERS ===============================================
			case INSTRUCTION_OPERATION_INT_ALU:
			case INSTRUCTION_OPERATION_NOP:
			case INSTRUCTION_OPERATION_OTHER:
			{
				emc_package->stage = PROCESSOR_STAGE_COMMIT;
				emc_package->uop.updatePackageReady(EXECUTE_LATENCY + COMMIT_LATENCY);
				this->solve_emc_dependencies(emc_package);
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
				this->memory_op_executed++;
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
				this->memory_op_executed++;
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
			} //end switch
		}	 //end if ready package
	}		  //end for
	if (this->memory_op_executed > 0)
	{
		this->lsq_read();
	}
};
// ============================================================================
void emc_t::emc_commit(){
	for (uint32_t i = 0; i < EMC_COMMIT_WIDTH; i++)
	{
		int8_t pos_buffer = this->uop_buffer_start;
		if (this->uop_buffer_used != 0 &&
			this->uop_buffer[pos_buffer].stage == PROCESSOR_STAGE_COMMIT &&
			this->uop_buffer[pos_buffer].uop.readyAt <= orcs_engine.get_global_cycle()){
			ERROR_ASSERT_PRINTF(uint32_t(pos_buffer) == this->uop_buffer_start, "EMC sending different position from start\n");
			this->emc_send_back_core(&this->uop_buffer[pos_buffer]);
			this->remove_front_uop_buffer();
		}
		else{
			break;
		}
	}
};
// ============================================================================
void emc_t::solve_emc_dependencies(emc_opcode_package_t *emc_opcode){
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
			/// This update the ready cycle, and it is usefull to compute the time each instruction waits for the functional unit
			if (emc_opcode->reg_deps_ptr_array[j]->uop.readyAt <= orcs_engine.get_global_cycle())
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
void emc_t::clock(){
	if (this->ready_to_execute)
	{
		// if (this->uop_buffer_used > 0)
		// {
		// 	for (size_t i = this->uop_buffer_start; i < this->uop_buffer_end; i++)
		// 	{
		// 		if (i > EMC_UOP_BUFFER)
		// 			i = 0;
		// 		ORCS_PRINTF("%s\n", this->uop_buffer[i].content_to_string().c_str())
		// 	}
		// }
		this->emc_commit();
		this->emc_execute();
		this->emc_dispatch();
	}
}
// ============================================================================
void emc_t::lsq_read(){

	int32_t position_mem = POSITION_FAIL;
	#if MOB_DEBUG
		ORCS_PRINTF("MOB Read")
	#endif
	memory_order_buffer_line_t *mob_line = NULL;
	for (size_t i = 0; i < PARALLEL_LOADS; i++)
	{

		position_mem = memory_order_buffer_line_t::find_old_request_state_ready(this->unified_lsq, EMC_LSQ_SIZE, PACKAGE_STATE_UNTREATED);
		if (position_mem != POSITION_FAIL)
		{
			mob_line = &this->unified_lsq[position_mem];
		}
		if (mob_line != NULL)
		{
		#if EMC_ACTIVE
			if (mob_line->memory_operation == MEMORY_OPERATION_READ){
				uint32_t ttc = 0;
				ttc = orcs_engine.cacheManager->search_EMC_Data(mob_line); //enviar que é do emc
				mob_line->updatePackageReady(ttc);
				mob_line->emc_opcode_ptr->uop.updatePackageReady(ttc);
				this->memory_op_executed--;
				// copy values to mob core (coherence)
				mob_line->emc_opcode_ptr->rob_ptr->mob_ptr->readyAt = mob_line->readyAt;				//ready
				mob_line->emc_opcode_ptr->rob_ptr->mob_ptr->uop_executed = mob_line->uop_executed;		//uop foi executado
				mob_line->emc_opcode_ptr->rob_ptr->mob_ptr->status = mob_line->status;					// ja foi enviado.
				// ======================================================================================================
			}
			else{
				uint32_t ttc = 4;
				// grava no lsq
				mob_line->updatePackageReady(ttc);
				mob_line->emc_opcode_ptr->uop.updatePackageReady(ttc);
				//enviar de volta ao core para notificar ops;
				this->memory_op_executed--;
				// send address ring back
				mob_line->emc_opcode_ptr->rob_ptr->mob_ptr->uop_executed = mob_line->uop_executed;		//uop foi executado
				mob_line->emc_opcode_ptr->rob_ptr->mob_ptr->status = mob_line->status;	//uop pronto
			}
		#endif
		} //end if mob_line null
	}	 //end for
};		  //end method
// ============================================================================
void emc_t::statistics(){
	if (orcs_engine.output_file_name == NULL)
	{
		utils_t::largestSeparator();
		ORCS_PRINTF("EMC - Enhaced Memory Controller")
		utils_t::largestSeparator();
		ORCS_PRINTF("EMC_Access_LLC: %lu\n", this->get_access_LLC())
		ORCS_PRINTF("EMC_Access_LLC_HIT: %lu\n", this->get_access_LLC_Hit())
		ORCS_PRINTF("EMC_Access_LLC_MISS: %lu\n", this->get_access_LLC_Miss())
		utils_t::largestSeparator();
		ORCS_PRINTF("##############  EMC_Data_Cache ##################\n")
		this->data_cache->statistics();
		utils_t::largestSeparator();
	}
	else
	{
		FILE *output = fopen(orcs_engine.output_file_name, "a+");
		if (output != NULL)
		{
			utils_t::largestSeparator(output);
			fprintf(output, "EMC - Enhaced Memory Controller");
			utils_t::largestSeparator(output);
			fprintf(output, "EMC_Access_LLC: %lu\n", this->get_access_LLC());
			fprintf(output, "EMC_Access_LLC_HIT: %lu\n", this->get_access_LLC_Hit());
			fprintf(output, "EMC_Access_LLC_MISS: %lu\n", this->get_access_LLC_Miss());
			utils_t::largestSeparator(output);
			fprintf(output, "##############  EMC_Data_Cache ##################\n");
			this->data_cache->statistics();
			utils_t::largestSeparator(output);
		}
		fclose(output);
	}
};
// ============================================================================
void emc_t::emc_send_back_core(emc_opcode_package_t *emc_opcode){
		emc_opcode->rob_ptr->stage = emc_opcode->stage;
		reorder_buffer_line_t *rob_line = emc_opcode->rob_ptr;
		//Remove from reservation station
		auto itr = std::find_if(orcs_engine.processor->unified_reservation_station.begin(), orcs_engine.processor->unified_reservation_station.end(),
		[rob_line](reorder_buffer_line_t* v) {return rob_line==v;});
		if (itr != orcs_engine.processor->unified_reservation_station.cend()) {
			uint32_t distance = std::distance(orcs_engine.processor->unified_reservation_station.begin(), itr);
			std::cout << "Distance: "<<distance << "\n";
			orcs_engine.processor->unified_reservation_station.erase(itr);
    }
		//		
		orcs_engine.processor->solve_registers_dependency(emc_opcode->rob_ptr);
}