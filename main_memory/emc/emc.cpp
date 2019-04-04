#include "../../simulator.hpp"
emc_t::emc_t()
{
	//alocate uop buffer
	this->uop_buffer = NULL;
	//allocate lsq
	this->unified_lsq = NULL;
	// ========================================
	this->fu_int_alu = NULL;
	this->fu_int_div = NULL;
	this->fu_int_mul = NULL;
	// ========================================
	this->fu_fp_mul = NULL;
	this->fu_fp_div = NULL;
	this->fu_fp_alu = NULL;
	// ========================================
	this->fu_mem_load = NULL;
	this->fu_mem_store = NULL;
	// ========================================
	//MACT
	this->memory_access_counter_table = NULL;
	this->mact_bits_mask = 0;
	
}
emc_t::~emc_t()
{
	// ========================================
	utils_t::template_delete_array<uint64_t>(this->fu_int_alu);
	utils_t::template_delete_array<uint64_t>(this->fu_int_mul);
	utils_t::template_delete_array<uint64_t>(this->fu_int_div);
	// ========================================
	utils_t::template_delete_array<uint64_t>(this->fu_fp_alu);
	utils_t::template_delete_array<uint64_t>(this->fu_fp_mul);
	utils_t::template_delete_array<uint64_t>(this->fu_fp_div);
	// ========================================
	utils_t::template_delete_array<uint64_t>(this->fu_mem_load);
	utils_t::template_delete_array<uint64_t>(this->fu_mem_store);
	// ========================================
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
}
// ============================================================================
// @allocate objects to EMC
void emc_t::allocate()
{
	// ======================= Unified FUs =======================
	this->unified_fus.reserve(EMC_UOP_BUFFER);
	//=======================  Unified RS =======================
	this->unified_rs.reserve(EMC_UOP_BUFFER);


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
	this->fu_int_mul = utils_t::template_allocate_initialize_array<uint64_t>(EMC_INTEGER_MUL, 0);
	this->fu_int_div = utils_t::template_allocate_initialize_array<uint64_t>(EMC_INTEGER_DIV, 0);
	// ========================================
	this->fu_fp_alu = utils_t::template_allocate_initialize_array<uint64_t>(EMC_FP_ALU, 0);
	this->fu_fp_mul = utils_t::template_allocate_initialize_array<uint64_t>(EMC_FP_MUL, 0);
	this->fu_fp_div = utils_t::template_allocate_initialize_array<uint64_t>(EMC_FP_DIV, 0);
	// ========================================
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
	//=======================  wait list all uop completes =======================
	this->uop_wait_finish.allocate(EMC_UOP_BUFFER);
	//pro valgrind nao chiar/debug aid
	this->has_store=false;
}
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
}
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
}
// ============================================================================
void emc_t::emc_dispatch(){	
	#if EMC_DISPATCH_DEBUG
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
		ORCS_PRINTF("====================== EMC Dispatch Stage =========================\n")
		ORCS_PRINTF("Cycle %lu\n",orcs_engine.get_global_cycle())
		}
	#endif
		//control variables
		uint32_t uop_dispatched = 0;
		/// Control the total dispatched per FU
		uint32_t fu_int_alu = 0;
		uint32_t fu_int_mul = 0;
		uint32_t fu_int_div = 0;

		uint32_t fu_fp_alu = 0;
		uint32_t fu_fp_mul = 0;
		uint32_t fu_fp_div = 0;

		uint32_t fu_mem_load = 0;
		uint32_t fu_mem_store = 0;

	for (uint32_t i = 0; i < this->unified_rs.size() && i < EMC_UNIFIED_RS; i++){
		emc_opcode_package_t *emc_opcode = this->unified_rs[i];		
		if (uop_dispatched >= EMC_DISPATCH_WIDTH){
			break;
		}
		if(emc_opcode == NULL){
			break;
		}
		if ((emc_opcode->rob_ptr !=NULL)&&(emc_opcode->rob_ptr->original_miss == true)){
			this->unified_rs.erase(this->unified_rs.begin() + i);
			i--;
			continue;
		}

	#if EMC_DISPATCH_DEBUG
			if (orcs_engine.get_global_cycle() > WAIT_CYCLE)
			{
				ORCS_PRINTF("==================\n")
				ORCS_PRINTF("EMC Trying dispatch %s\n", emc_opcode->content_to_string().c_str())
				ORCS_PRINTF("==================\n")
			}
	#endif
		if ((emc_opcode->uop.readyAt <= orcs_engine.get_global_cycle()) && (emc_opcode->wait_reg_deps_number == 0)){
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
								this->fu_int_alu[k] = orcs_engine.get_global_cycle() + EMC_WAIT_NEXT_INTEGER_ALU;
								fu_int_alu++;
								dispatched = true;
								emc_opcode->stage = PROCESSOR_STAGE_EXECUTION;
								emc_opcode->uop.updatePackageReady(EMC_INTEGER_LATENCY_ALU);
								break;
							}
						}
					}
					break;
				// ====================================================
				// Integer Multiplication
				case INSTRUCTION_OPERATION_INT_MUL:
					if (fu_int_mul < EMC_INTEGER_MUL)
					{
						for (uint8_t k = 0; k < EMC_INTEGER_MUL; k++)
						{
							if (this->fu_int_mul[k] <= orcs_engine.get_global_cycle())
							{
								this->fu_int_mul[k] = orcs_engine.get_global_cycle() + EMC_WAIT_NEXT_INTEGER_MUL;
								fu_int_mul++;
								dispatched = true;
								emc_opcode->stage = PROCESSOR_STAGE_EXECUTION;
								emc_opcode->uop.updatePackageReady(EMC_INTEGER_LATENCY_MUL);
								break;
							}
						}
					}
					break;
				// ====================================================
				// Integer division
				case INSTRUCTION_OPERATION_INT_DIV:
					if (fu_int_div < EMC_INTEGER_DIV)
					{
						for (uint8_t k = 0; k < EMC_INTEGER_DIV; k++)
						{
							if (this->fu_int_div[k] <= orcs_engine.get_global_cycle())
							{
								this->fu_int_div[k] = orcs_engine.get_global_cycle() + EMC_WAIT_NEXT_INTEGER_DIV;
								fu_int_div++;
								dispatched = true;
								emc_opcode->stage = PROCESSOR_STAGE_EXECUTION;
								emc_opcode->uop.updatePackageReady(EMC_INTEGER_LATENCY_DIV);
								break;
							}
						}
					}
					break;
				// ====================================================
				// Floating point ALU operation
				case INSTRUCTION_OPERATION_FP_ALU:
					if (fu_fp_alu < EMC_FP_ALU)
					{
						for (uint8_t k = 0; k < EMC_FP_ALU; k++)
						{
							if (this->fu_fp_alu[k] <= orcs_engine.get_global_cycle())
							{
								this->fu_fp_alu[k] = orcs_engine.get_global_cycle() + EMC_WAIT_NEXT_FP_ALU;
								fu_fp_alu++;
								dispatched = true;
								emc_opcode->stage = PROCESSOR_STAGE_EXECUTION;
								emc_opcode->uop.updatePackageReady(EMC_FP_LATENCY_ALU);
								break;
							}
						}
					}
					break;
				// ====================================================
				// Floating Point Multiplication
				case INSTRUCTION_OPERATION_FP_MUL:
					if (fu_fp_mul < EMC_FP_MUL)
					{
						for (uint8_t k = 0; k < EMC_FP_MUL; k++)
						{
							if (this->fu_fp_mul[k] <= orcs_engine.get_global_cycle())
							{
								this->fu_fp_mul[k] = orcs_engine.get_global_cycle() + EMC_WAIT_NEXT_FP_MUL;
								fu_fp_mul++;
								dispatched = true;
								emc_opcode->stage = PROCESSOR_STAGE_EXECUTION;
								emc_opcode->uop.updatePackageReady(EMC_FP_LATENCY_MUL);
								break;
							}
						}
					}
					break;

				// ====================================================
				// Floating Point Division
				case INSTRUCTION_OPERATION_FP_DIV:
					if (fu_fp_div < EMC_FP_DIV)
					{
						for (uint8_t k = 0; k < EMC_FP_DIV; k++)
						{
							if (this->fu_fp_div[k] <= orcs_engine.get_global_cycle())
							{
								this->fu_fp_div[k] = orcs_engine.get_global_cycle() + EMC_WAIT_NEXT_FP_DIV;
								fu_fp_div++;
								dispatched = true;
								emc_opcode->stage = PROCESSOR_STAGE_EXECUTION;
								emc_opcode->uop.updatePackageReady(EMC_FP_LATENCY_DIV);
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
								this->fu_mem_load[k] = orcs_engine.get_global_cycle() + EMC_WAIT_NEXT_MEM_LOAD;
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
								this->fu_mem_store[k] = orcs_engine.get_global_cycle() + EMC_WAIT_NEXT_MEM_STORE;
								fu_mem_store++;
								dispatched = true;
								emc_opcode->stage = PROCESSOR_STAGE_EXECUTION;
								emc_opcode->uop.updatePackageReady(LATENCY_MEM_STORE);
								break;
							}
						}
					}
					break;
				// ====================================================
				case INSTRUCTION_OPERATION_BARRIER:
				case INSTRUCTION_OPERATION_HMC_ROA:
				case INSTRUCTION_OPERATION_HMC_ROWA:
					ERROR_PRINTF("Invalid instruction being dispatched.\n");
					break;
				} //end switch case
			if (dispatched == true){
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
}
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
				ORCS_PRINTF("==================\n")
				ORCS_PRINTF("EMC Memory Solving %s\n", this->unified_lsq[i].emc_opcode_ptr->content_to_string().c_str())
				ORCS_PRINTF("==================\n")

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
					ORCS_PRINTF("==================\n")
					ORCS_PRINTF("EMC Trying Execute %s\n", emc_package->content_to_string().c_str())
					ORCS_PRINTF("==================\n")
				}
			#endif
			ERROR_ASSERT_PRINTF(emc_package->uop.status == PACKAGE_STATE_READY, "FU with Package not in ready state")
			ERROR_ASSERT_PRINTF(emc_package->stage == PROCESSOR_STAGE_EXECUTION, "FU with Package not in execution stage")
			switch (emc_package->uop.uop_operation)
			{
			case INSTRUCTION_OPERATION_BRANCH:
			// INTEGERS ===============================================
			case INSTRUCTION_OPERATION_INT_ALU:
			case INSTRUCTION_OPERATION_INT_MUL:
			case INSTRUCTION_OPERATION_INT_DIV:
			// FLOAT POINT ===============================================
			case INSTRUCTION_OPERATION_FP_ALU:
			case INSTRUCTION_OPERATION_FP_MUL:
			case INSTRUCTION_OPERATION_FP_DIV:
			// OTHER OPS=============================================================
			case INSTRUCTION_OPERATION_NOP:
			case INSTRUCTION_OPERATION_OTHER:{
				emc_package->stage = PROCESSOR_STAGE_COMMIT;
				emc_package->uop.updatePackageReady(EMC_COMMIT_LATENCY);
				this->solve_emc_dependencies(emc_package);
				uop_total_executed++;
				/// Remove from the Functional Units
				this->unified_fus.erase(this->unified_fus.begin() + i);
				i--;
			}
			break;
			// MEMORY LOAD/STORE ==========================================
			case INSTRUCTION_OPERATION_MEM_LOAD:{
				ERROR_ASSERT_PRINTF(emc_package->mob_ptr != NULL, "Read with a NULL pointer to MOB")
				this->memory_op_executed++;
				//Atualizando operacoes executadas, pois senão trava o envio de operacoes load
				orcs_engine.processor[emc_package->mob_ptr->processor_id].memory_read_executed++;
				//
				emc_package->mob_ptr->uop_executed = true;
				emc_package->uop.updatePackageReady(EMC_EXECUTE_LATENCY);
				uop_total_executed++;
				/// Remove from the Functional Units
				this->unified_fus.erase(this->unified_fus.begin() + i);
				i--;
			}
			break;
			case INSTRUCTION_OPERATION_MEM_STORE:{
				ERROR_ASSERT_PRINTF(emc_package->mob_ptr != NULL, "Write with a NULL pointer to MOB")
				this->memory_op_executed++;
				//Atualizando operacoes executadas, pois senão trava o envio de operacoes store
				orcs_engine.processor[emc_package->mob_ptr->processor_id].memory_write_executed++;
				emc_package->mob_ptr->uop_executed = true;
				emc_package->uop.updatePackageReady(EMC_EXECUTE_LATENCY);
				uop_total_executed++;
				/// Remove from the Functional Units
				this->unified_fus.erase(this->unified_fus.begin() + i);
				i--;
				emc_package->rob_ptr->mob_ptr->uop_executed = true;
			}
			break;
			case INSTRUCTION_OPERATION_BARRIER:
			case INSTRUCTION_OPERATION_HMC_ROA:
			case INSTRUCTION_OPERATION_HMC_ROWA:
				ERROR_PRINTF("Invalid instruction Executed.\n");
			break;
			} //end switch
		}//end if ready package
		#if EMC_EXECUTE_DEBUG
				if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
					ORCS_PRINTF("==================\n")
					ORCS_PRINTF("EMC Executed %s\n", emc_package->content_to_string().c_str())
					ORCS_PRINTF("==================\n")
				}
		#endif
	}		  //end for
	if (this->memory_op_executed > 0)
	{
		this->lsq_read();
	}
}
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
			switch (this->uop_buffer[pos_buffer].uop.uop_operation){
			// INTEGERS ALU
			case INSTRUCTION_OPERATION_INT_ALU:
				this->add_stat_inst_int_alu_completed();
				break;

			// INTEGERS MUL
			case INSTRUCTION_OPERATION_INT_MUL:
				this->add_stat_inst_mul_alu_completed();
				break;

			// INTEGERS DIV
			case INSTRUCTION_OPERATION_INT_DIV:
				this->add_stat_inst_div_alu_completed();
				break;

			// FLOAT POINT ALU
			case INSTRUCTION_OPERATION_FP_ALU:
				this->add_stat_inst_int_fp_completed();
				break;

			// FLOAT POINT MUL
			case INSTRUCTION_OPERATION_FP_MUL:
				this->add_stat_inst_mul_fp_completed();
				break;

			// FLOAT POINT DIV
			case INSTRUCTION_OPERATION_FP_DIV:
				this->add_stat_inst_div_fp_completed();
				break;

			// MEMORY OPERATIONS - READ
			case INSTRUCTION_OPERATION_MEM_LOAD:{
				this->add_stat_inst_load_completed();
				break;
			}
			// MEMORY OPERATIONS - WRITE
			case INSTRUCTION_OPERATION_MEM_STORE:
				this->add_stat_inst_store_completed();
				break;
				// BRANCHES	

			case INSTRUCTION_OPERATION_BRANCH:
				this->add_stat_inst_branch_completed();
				break;

			// NOP
			case INSTRUCTION_OPERATION_NOP:
				this->add_stat_inst_nop_completed();
				break;

			// NOT IDENTIFIED
			case INSTRUCTION_OPERATION_OTHER:
				this->add_stat_inst_other_completed();
				break;

			case INSTRUCTION_OPERATION_BARRIER:
			case INSTRUCTION_OPERATION_HMC_ROWA:
			case INSTRUCTION_OPERATION_HMC_ROA:
				ERROR_PRINTF("Invalid instruction BARRIER| HMC ROA | HMC ROWA.\n");
				break;
		}
			ERROR_ASSERT_PRINTF(uint32_t(pos_buffer) == this->uop_buffer_start, "EMC sending different position from start\n");
			// ==========================================================
			// contar statistics from miss predictor
			if((this->uop_buffer[pos_buffer].mob_ptr != NULL) &&(!this->uop_buffer[pos_buffer].rob_ptr->original_miss)){
				if(this->uop_buffer[pos_buffer].mob_ptr->emc_predict_access_ram && this->uop_buffer[pos_buffer].mob_ptr->emc_generate_miss){
					this->add_direct_ram_access();
					this->update_mact_entry(this->uop_buffer[pos_buffer].uop.opcode_address,1);
				}else if (this->uop_buffer[pos_buffer].mob_ptr->emc_predict_access_ram && !this->uop_buffer[pos_buffer].mob_ptr->emc_generate_miss){
					this->add_incorrect_prediction_ram_access();
					this->update_mact_entry(this->uop_buffer[pos_buffer].uop.opcode_address,-1);
				}
				else if (!this->uop_buffer[pos_buffer].mob_ptr->emc_predict_access_ram && this->uop_buffer[pos_buffer].mob_ptr->emc_generate_miss){
					this->add_incorrect_prediction_LLC_access();
					this->update_mact_entry(this->uop_buffer[pos_buffer].uop.opcode_address,1);
				}else if (!this->uop_buffer[pos_buffer].mob_ptr->emc_predict_access_ram && !this->uop_buffer[pos_buffer].mob_ptr->emc_generate_miss && !this->uop_buffer[pos_buffer].mob_ptr->l1_emc_hit){
					this->add_emc_llc_access();
					this->update_mact_entry(this->uop_buffer[pos_buffer].uop.opcode_address,-1);
				}
			}
			// ==========================================================
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
}
// ============================================================================
void emc_t::solve_emc_dependencies(emc_opcode_package_t *emc_opcode){
	#if EMC_EXECUTE_DEBUG
	if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
		ORCS_PRINTF("==================\n")
		ORCS_PRINTF("Solving %s\n", emc_opcode->content_to_string().c_str())
		ORCS_PRINTF("==================\n")
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
}
// ============================================================================
void emc_t::clock(){
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
		if(this->has_store){
			this->has_store = false;
				if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
					ORCS_PRINTF("Chain to execute %lu\n",orcs_engine.get_global_cycle())
					this->print_structures();
				}
			}
		if(this->uop_buffer_used>0){
			this->emc_commit();
			this->emc_execute();
			this->emc_dispatch();
		}else{
			this->ready_to_execute=false;
			#if EMC_DEBUG
				if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
					ORCS_PRINTF("\t\t EMC %u Unlocking Processor %lu\n",this->processor_id,orcs_engine.get_global_cycle())
				}
			#endif
			// ERROR_ASSERT_PRINTF(orcs_engine.memory_controller->emc_active > 0,"Erro, tentando reduzir EMCs ativos menor que zero\n")
			orcs_engine.processor[this->processor_id].lock_processor=false;
		}
	}
}
// ============================================================================
void emc_t::update_mact_entry(uint64_t pc,int32_t value){
	
	uint64_t index  = utils_t::hash_function(HASH_FUNCTION_INPUT1_ONLY,(pc>>2),0,this->mact_bits_mask);

	this->memory_access_counter_table[index]+=value;

	if(this->memory_access_counter_table[index]>(signed)(this->mact_bits_mask-1)){
		this->memory_access_counter_table[index]=(signed)(this->mact_bits_mask-1);	
	}else if(this->memory_access_counter_table[index]<0){
		this->memory_access_counter_table[index]=0;
	}
}
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
			ORCS_PRINTF("Memory Operation Selected\n==================\n %s\n", emc_mob_line->content_to_string().c_str())
			ORCS_PRINTF("==================\n")

		}
	#endif
		if (emc_mob_line->memory_operation == MEMORY_OPERATION_READ){
			// ==========================================================================================
			// ORACLE
			this->oracle_access_emc(emc_mob_line);
			// ==========================================================================================
			uint32_t ttc = 0;
			ttc = orcs_engine.cacheManager->search_EMC_Data(emc_mob_line); //enviar que é do emc
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
			this->lsq_forward(emc_mob_line);
		}
		#if EMC_LSQ_DEBUG
			if(orcs_engine.get_global_cycle()>WAIT_CYCLE){	
				ORCS_PRINTF("==================\n")
				ORCS_PRINTF("Memory Operation After Send\n %s\n", emc_mob_line->content_to_string().c_str())
				ORCS_PRINTF("==================\n")
			}
	#endif
	} //end if mob_line null
}	//end method
// ============================================================================
void emc_t::statistics(){
	FILE *output = stdout;
	bool close = false;
	if(orcs_engine.output_file_name != NULL){
		close=true;
		output = fopen(orcs_engine.output_file_name,"a+");
	}
	if (output != NULL){
		utils_t::largestSeparator(output);
		fprintf(output, "EMC - Enhaced Memory Controller\n");
		utils_t::largestSeparator(output);
		fprintf(output, "EMC_Access_LLC: %lu\n", this->get_access_LLC());
		fprintf(output, "EMC_Access_LLC_HIT: %lu\n", this->get_access_LLC_Hit());
		fprintf(output, "EMC_Access_LLC_MISS: %lu\n", this->get_access_LLC_Miss());
		fprintf(output, "EMC_Correct_Prediction_RAM: %lu\n", this->get_direct_ram_access());
		fprintf(output, "EMC_Correct_LLC_Prediction: %lu\n", this->get_emc_llc_access());
		fprintf(output, "EMC_Incorrect_RAM_Prediction: %lu\n",this->get_incorrect_prediction_ram_access());
		fprintf(output, "EMC_Incorrect_LLC_Prediction: %lu\n",this->get_incorrect_prediction_LLC_access());
		utils_t::largestSeparator(output);
		fprintf(output, "ORACLE_EMC_DATA_CACHE_MISS: %lu\n",this->get_oracle_emc_data_cache_misses());
		fprintf(output, "ORACLE_EMC_LLC_CACHE_MISS: %lu\n",this->get_oracle_emc_LLC_misses());
		
		utils_t::largestSeparator(output);
		fprintf(output,"INSTRUCTION_OPERATION_INT_ALU %lu\n",this->get_stat_inst_int_alu_completed());
		fprintf(output,"INSTRUCTION_OPERATION_INT_MUL %lu\n",this->get_stat_inst_mul_alu_completed());
		fprintf(output,"INSTRUCTION_OPERATION_INT_DIV %lu\n",this->get_stat_inst_div_alu_completed());
		fprintf(output,"INSTRUCTION_OPERATION_FP_ALU %lu\n",this->get_stat_inst_int_fp_completed());
		fprintf(output,"INSTRUCTION_OPERATION_FP_MUL %lu\n",this->get_stat_inst_mul_fp_completed());
		fprintf(output,"INSTRUCTION_OPERATION_FP_DIV %lu\n",this->get_stat_inst_div_fp_completed());
		fprintf(output,"INSTRUCTION_OPERATION_MEM_LOAD %lu\n",this->get_stat_inst_load_completed());
		fprintf(output,"INSTRUCTION_OPERATION_MEM_STORE %lu\n",this->get_stat_inst_store_completed());
		fprintf(output,"INSTRUCTION_OPERATION_BRANCH %lu\n",this->get_stat_inst_branch_completed());
		fprintf(output,"INSTRUCTION_OPERATION_NOP %lu\n",this->get_stat_inst_nop_completed());
		fprintf(output,"INSTRUCTION_OPERATION_OTHER %lu\n",this->get_stat_inst_other_completed());
		utils_t::largeSeparator(output);
		if(close) fclose(output);
		}

}
// ============================================================================
// void emc_t::emc_send_back_core(){
void emc_t::emc_send_back_core(emc_opcode_package_t *emc_opcode){
		reorder_buffer_line_t *rob_line = emc_opcode->rob_ptr;
		uint32_t processor_id = rob_line->processor_id;
		#if EMC_COMMIT_DEBUG
			if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
				ORCS_PRINTF("==============================\n")
				ORCS_PRINTF("Sending %s\n\n", emc_opcode->content_to_string().c_str())
				ORCS_PRINTF("To %s\n\n",rob_line->content_to_string().c_str())
			}
		#endif
		// Atualizar opcodes de acesso a memoria
		if(emc_opcode->uop.uop_operation  == INSTRUCTION_OPERATION_BRANCH ||
			emc_opcode->uop.uop_operation == INSTRUCTION_OPERATION_INT_ALU||
			emc_opcode->uop.uop_operation == INSTRUCTION_OPERATION_INT_MUL||
			emc_opcode->uop.uop_operation == INSTRUCTION_OPERATION_INT_DIV||
			emc_opcode->uop.uop_operation == INSTRUCTION_OPERATION_FP_ALU||
			emc_opcode->uop.uop_operation == INSTRUCTION_OPERATION_FP_MUL||
			emc_opcode->uop.uop_operation == INSTRUCTION_OPERATION_FP_DIV||
			emc_opcode->uop.uop_operation == INSTRUCTION_OPERATION_OTHER||
			emc_opcode->uop.uop_operation == INSTRUCTION_OPERATION_NOP){
				rob_line->uop = emc_opcode->uop;
				rob_line->stage = emc_opcode->stage;
				rob_line->emc_executed = true;
				orcs_engine.processor[processor_id].solve_registers_dependency(rob_line);
			}else{
				ERROR_ASSERT_PRINTF(emc_opcode->mob_ptr !=NULL, "Error,emc  memory operation without mob value %s\n",emc_opcode->content_to_string().c_str())
				ERROR_ASSERT_PRINTF(rob_line->mob_ptr !=NULL, "Error, rob memory operation without mob value %s\n",rob_line->content_to_string().c_str())
				if(rob_line->original_miss){
					#if EMC_COMMIT_DEBUG
						if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
							ORCS_PRINTF("Skipping -> Miss Original\n")
							}
					#endif
					emc_opcode->mob_ptr->package_clean();
					return;
				}
				if(emc_opcode->uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD){
					*(rob_line->mob_ptr) = *(emc_opcode->mob_ptr);//copiar somente os valores de, uop_executed, ready at, status, sent, is_llc_miss
					rob_line->mob_ptr->sent_to_emc = true;
				}else{
					rob_line->mob_ptr->sent_to_emc=false;
				}
				
				emc_opcode->mob_ptr->package_clean();
				rob_line->mob_ptr->processed = false;
				rob_line->mob_ptr->emc_executed = true;
				rob_line->sent = rob_line->mob_ptr->sent = true;
				rob_line->emc_executed = true;
				// ===========================================================================
				// eliminar a flag para ser executado no core 
				// ===========================================================================
			}
			#if EMC_COMMIT_DEBUG
				if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
					ORCS_PRINTF("Rob Line after send \n%s\n", rob_line->content_to_string().c_str())
					ORCS_PRINTF("==============================\n\n")
				}
			#endif
}
void emc_t::print_structures(){
	ORCS_PRINTF("\nUop Buffer used %d\n",this->uop_buffer_used)
	ORCS_PRINTF("\n ============ Uop Buffer ===============\n")
	uint32_t pos = this->uop_buffer_start;
	for (uint32_t i = 0;i < this->uop_buffer_used;i++){
		ORCS_PRINTF("\n%s\n",this->uop_buffer[pos].content_to_string().c_str())
		pos++;
		if(pos>=EMC_UOP_BUFFER)pos=0;
	}
	ORCS_PRINTF("\n ============ Load Store Queue ===============\n")
	for (uint32_t i = 0;i < EMC_LSQ_SIZE;i++){
		if(this->unified_lsq[i].status == PACKAGE_STATE_FREE){
			continue;
		}
		ORCS_PRINTF("\n%s\n",this->unified_lsq[i].content_to_string().c_str())
	}
}
void emc_t::lsq_forward(memory_order_buffer_line_t *emc_mob_line){
	for (uint16_t i = 0; i < EMC_LSQ_SIZE; i++){
		if(this->unified_lsq[i].memory_address == emc_mob_line->memory_address &&
			this->unified_lsq[i].memory_size == emc_mob_line->memory_size &&
			this->unified_lsq[i].memory_operation == MEMORY_OPERATION_READ){

			this->unified_lsq[i].status = PACKAGE_STATE_READY;
				this->unified_lsq[i].sent = true;
				this->unified_lsq[i].readyAt = orcs_engine.get_global_cycle() + REGISTER_FORWARD;
				this->unified_lsq[i].forwarded_data=true;
				#if EMC_LSQ_DEBUG
					ORCS_PRINTF("Forwarded data\n")
					ORCS_PRINTF("FROM: %s\n",emc_mob_line->content_to_string().c_str())
					ORCS_PRINTF("TO: %s\n",this->unified_lsq[i].content_to_string().c_str())
				#endif
				break;
		}
	}
}
void  emc_t::oracle_access_emc(memory_order_buffer_line_t *emc_mob_line){
	uint32_t index_llc = orcs_engine.cacheManager->generate_index_array(this->processor_id,LLC);
	if(orcs_engine.memory_controller->data_cache->read_oracle(emc_mob_line->memory_address)==MISS){
		this->add_oracle_emc_data_cache_misses();
	}
	if(orcs_engine.cacheManager->LLC_data_cache[index_llc].read_oracle(emc_mob_line->memory_address)==MISS){
		this->add_oracle_emc_LLC_misses();
	}
}