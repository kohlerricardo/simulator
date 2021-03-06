#include "./../simulator.hpp"

// =====================================================================
processor_t::processor_t()
{
	//Setting Pointers to NULL
	// ========OLDEST MEMORY OPERATIONS POINTER======
	this->oldest_read_to_send = NULL;
	this->oldest_write_to_send = NULL;
	// ========MOB======
	this->memory_order_buffer_read = NULL;
	this->memory_order_buffer_write = NULL;
	//=========DESAMBIGUATION ============
	this->desambiguator = NULL;
	// ==========RAT======
	this->register_alias_table = NULL;
	// ==========ROB========
	this->reorderBuffer = NULL;
	// ======FUs=========
	// Integer FUs
	this->fu_int_alu = NULL;
	this->fu_int_mul = NULL;
	this->fu_int_div = NULL;
	// Floating Points FUs
	this->fu_fp_alu = NULL;
	this->fu_fp_mul = NULL;
	this->fu_fp_div = NULL;
	// Memory FUs
	this->fu_mem_load = NULL;
	this->fu_mem_store = NULL;
	// =============== EMC Module ======================
	this->counter_make_dep_chain = 0;
	this->rrt = NULL;
	// =============== EMC Module ======================
}
processor_t::~processor_t()
{
	//Memory structures
	utils_t::template_delete_array<memory_order_buffer_line_t>(this->memory_order_buffer_read);
	utils_t::template_delete_array<memory_order_buffer_line_t>(this->memory_order_buffer_write);
	utils_t::template_delete_variable<desambiguation_t>(this->desambiguator);
	//auxiliar var to maintain status oldest instruction
	utils_t::template_delete_variable<memory_order_buffer_line_t>(this->oldest_read_to_send);
	utils_t::template_delete_variable<memory_order_buffer_line_t>(this->oldest_write_to_send);

	//deleting deps array rob
	for (size_t i = 0; i < ROB_SIZE; i++)
	{
		utils_t::template_delete_array<reorder_buffer_line_t>(this->reorderBuffer[i].reg_deps_ptr_array[0]);
	}
	// deleting rob
	utils_t::template_delete_array<reorder_buffer_line_t>(this->reorderBuffer);
	//delete RAT
	utils_t::template_delete_array<reorder_buffer_line_t *>(this->register_alias_table);
	//deleting fus int
	utils_t::template_delete_array<uint64_t>(this->fu_int_alu);
	utils_t::template_delete_array<uint64_t>(this->fu_int_mul);
	utils_t::template_delete_array<uint64_t>(this->fu_int_div);
	//deleting fus fp
	utils_t::template_delete_array<uint64_t>(this->fu_fp_alu);
	utils_t::template_delete_array<uint64_t>(this->fu_fp_mul);
	utils_t::template_delete_array<uint64_t>(this->fu_fp_div);
	//deleting fus memory
	utils_t::template_delete_array<uint64_t>(this->fu_mem_load);
	utils_t::template_delete_array<uint64_t>(this->fu_mem_store);
// =====================================================================
// EMC Related
#if EMC_ACTIVE

	for (size_t i = 0; i < EMC_REGISTERS; i++)
	{
		delete this->rrt[i].entry;
	}
	utils_t::template_delete_array<register_remapping_table_t>(this->rrt);
#endif
	// =====================================================================
}
// =====================================================================
void processor_t::allocate()
{
	//======================================================================
	// Initializating variables
	//======================================================================
	this->processor_id = 0;
	this->traceIsOver = false;
	this->hasBranch = false;
	this->insertError = false;
	this->snapshoted = false;
	this->fetchCounter = 1;
	this->decodeCounter = 1;
	this->renameCounter = 1;
	this->uopCounter = 1;
	this->commit_uop_counter = 0;
	this->set_stall_wrong_branch(0);
	this->memory_read_executed = 0;
	this->memory_write_executed = 0;
	//======================================================================
	// Initializating structures
	//======================================================================
	//======================================================================
	// FetchBuffer
	this->fetchBuffer.allocate(FETCH_BUFFER);
	// DecodeBuffer
	this->decodeBuffer.allocate(DECODE_BUFFER);
	// Register Alias Table
	this->register_alias_table = utils_t::template_allocate_initialize_array<reorder_buffer_line_t *>(RAT_SIZE, NULL);
	// Reorder Buffer
	this->robStart = 0;
	this->robEnd = 0;
	this->robUsed = 0;
	this->reorderBuffer = utils_t::template_allocate_array<reorder_buffer_line_t>(ROB_SIZE);
	for (uint32_t i = 0; i < ROB_SIZE; i++)
	{
		this->reorderBuffer[i].reg_deps_ptr_array = utils_t::template_allocate_initialize_array<reorder_buffer_line_t *>(ROB_SIZE, NULL);
	}
	// =========================================================================================
	// // Memory Order Buffer Read
	this->memory_order_buffer_read = utils_t::template_allocate_array<memory_order_buffer_line_t>(MOB_READ);
	for (size_t i = 0; i < MOB_READ; i++)
	{
		this->memory_order_buffer_read[i].mem_deps_ptr_array = utils_t::template_allocate_initialize_array<memory_order_buffer_line_t *>(ROB_SIZE, NULL);
	}
	// =========================================================================================
	// LOAD
	this->memory_order_buffer_read_start = 0;
	this->memory_order_buffer_read_end = 0;
	this->memory_order_buffer_read_used = 0;
	// =========================================================================================
	// // Memory Order Buffer Write
	this->memory_order_buffer_write = utils_t::template_allocate_array<memory_order_buffer_line_t>(MOB_WRITE);
	for (size_t i = 0; i < MOB_WRITE; i++)
	{
		this->memory_order_buffer_write[i].mem_deps_ptr_array = utils_t::template_allocate_initialize_array<memory_order_buffer_line_t *>(ROB_SIZE, NULL);
	}
	// =========================================================================================
	// STORE
	this->memory_order_buffer_write_start = 0;
	this->memory_order_buffer_write_end = 0;
	this->memory_order_buffer_write_used = 0;
	// =========================================================================================
	//desambiguator
	this->desambiguator = new desambiguation_t;
	this->desambiguator->allocate();
	// parallel requests
	// =========================================================================================
	//DRAM
	// =========================================================================================
	this->counter_mshr_read = 0;
	this->counter_mshr_write = 0;
	this->request_DRAM=0;
	// =========================================================================================
	//allocating fus int
	this->fu_int_alu = utils_t::template_allocate_initialize_array<uint64_t>(INTEGER_ALU, 0);
	this->fu_int_mul = utils_t::template_allocate_initialize_array<uint64_t>(INTEGER_MUL, 0);
	this->fu_int_div = utils_t::template_allocate_initialize_array<uint64_t>(INTEGER_DIV, 0);
	//allocating fus fp
	this->fu_fp_alu = utils_t::template_allocate_initialize_array<uint64_t>(FP_ALU, 0);
	this->fu_fp_mul = utils_t::template_allocate_initialize_array<uint64_t>(FP_MUL, 0);
	this->fu_fp_div = utils_t::template_allocate_initialize_array<uint64_t>(FP_DIV, 0);
	//allocating fus memory
	this->fu_mem_load = utils_t::template_allocate_initialize_array<uint64_t>(LOAD_UNIT, 0);
	this->fu_mem_store = utils_t::template_allocate_initialize_array<uint64_t>(STORE_UNIT, 0);
	// reserving space to uops on UFs pipeline, waitng to executing ends
	this->unified_reservation_station.reserve(ROB_SIZE);
	// reserving space to uops on UFs pipeline, waitng to executing ends
	this->unified_functional_units.reserve(ROB_SIZE);

// =====================================================================
// EMC Related
#if EMC_ACTIVE
	this->rob_buffer.reserve(ROB_SIZE);
	this->rrt = new register_remapping_table_t[EMC_REGISTERS];
#endif
	//======================================================================
	// Initializating EMC control variables
	//======================================================================
	this->start_emc_module = false;
	this->lock_processor = false;
	this->unable_start = false;
	this->instrucoes_inter_load_deps = 0;
	this->soma_instrucoes_deps = 0;
	this->numero_load_deps = 0;
	this->counter_ambiguation_read = 0;
	this->counter_ambiguation_write = 0;
	this->counter_activate_emc = 0;
	// =====================================================================
	this->cycle_start_mechanism = 0;
	// =====================================================================
}
// =====================================================================
bool processor_t::isBusy(){
	return (this->traceIsOver == false ||
			!this->fetchBuffer.is_empty() ||
			!this->decodeBuffer.is_empty() ||
			this->robUsed != 0);
}

// ======================================
// Require a position to insert on ROB
// The Reorder Buffer behavior is a Circular FIFO
// @return position to insert
// ======================================
int32_t processor_t::searchPositionROB(){
	int32_t position = POSITION_FAIL;
	/// There is free space.
	if (this->robUsed < ROB_SIZE)
	{
		position = this->robEnd;
		this->robUsed++;
		this->robEnd++;
		if (this->robEnd >= ROB_SIZE)
		{
			this->robEnd = 0;
		}
	}
	return position;
}
// ======================================
// Remove the Head of the reorder buffer
// The Reorder Buffer behavior is a Circular FIFO
// ======================================
void processor_t::removeFrontROB(){
	ERROR_ASSERT_PRINTF(this->robUsed > 0, "Removendo do ROB sem estar usado\n")
	ERROR_ASSERT_PRINTF(this->reorderBuffer[this->robStart].reg_deps_ptr_array[0] == NULL, "Removendo sem resolver dependencias\n%s\n",this->reorderBuffer[this->robStart].content_to_string().c_str())
	this->reorderBuffer[this->robStart].package_clean();
	this->robUsed--;
	this->robStart++;
	if (this->robStart >= ROB_SIZE)
	{
		this->robStart = 0;
	}
}
// ============================================================================
// get position on MOB read.
// MOB read is a circular buffer
// ============================================================================
int32_t processor_t::search_position_mob_read(){
	int32_t position = POSITION_FAIL;
	/// There is free space.
	if (this->memory_order_buffer_read_used < MOB_READ)
	{
		position = this->memory_order_buffer_read_end;
		this->memory_order_buffer_read_used++;
		this->memory_order_buffer_read_end++;
		if (this->memory_order_buffer_read_end >= MOB_READ)
		{
			this->memory_order_buffer_read_end = 0;
		}
	}
	return position;
}
// ============================================================================
// remove front mob read on commit
// ============================================================================
void processor_t::remove_front_mob_read(){
	#if COMMIT_DEBUG
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("==========\n")
			ORCS_PRINTF("RM MOB Read Entry \n%s\n", this->memory_order_buffer_read[this->memory_order_buffer_read_start].content_to_string().c_str())
			ORCS_PRINTF("==========\n")
		}
	#endif
	ERROR_ASSERT_PRINTF(this->memory_order_buffer_read_used > 0, "Removendo do MOB_READ sem estar usado\n")
	ERROR_ASSERT_PRINTF(this->memory_order_buffer_read[this->memory_order_buffer_read_start].mem_deps_ptr_array[0] == NULL, "Removendo sem resolver dependencias\n%s\n",this->memory_order_buffer_read[this->memory_order_buffer_read_start].content_to_string().c_str())
	this->memory_order_buffer_read_used--;
	this->memory_order_buffer_read[this->memory_order_buffer_read_start].package_clean();
	this->memory_order_buffer_read_start++;
	if (this->memory_order_buffer_read_start >= MOB_READ)
	{
		this->memory_order_buffer_read_start = 0;
	}
}
// ============================================================================
// get position on MOB write.
// MOB read is a circular buffer
// ============================================================================
int32_t processor_t::search_position_mob_write(){
	int32_t position = POSITION_FAIL;
	/// There is free space.
	if (this->memory_order_buffer_write_used < MOB_WRITE)
	{
		position = this->memory_order_buffer_write_end;
		this->memory_order_buffer_write_used++;
		this->memory_order_buffer_write_end++;
		if (this->memory_order_buffer_write_end >= MOB_WRITE)
		{
			this->memory_order_buffer_write_end = 0;
		}
	}
	return position;
}
// ============================================================================
// remove front mob read on commit
// ============================================================================
void processor_t::remove_front_mob_write(){
	#if COMMIT_DEBUG
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("==========\n")
			ORCS_PRINTF("RM MOB Write Entry \n%s\n", this->memory_order_buffer_write[this->memory_order_buffer_write_start].content_to_string().c_str())
			ORCS_PRINTF("==========\n")
		}
	#endif
	ERROR_ASSERT_PRINTF(this->memory_order_buffer_write_used > 0, "Removendo do MOB_WRITE sem estar usado\n")
	ERROR_ASSERT_PRINTF(this->memory_order_buffer_write[this->memory_order_buffer_write_start].sent == true,"Removendo sem ter sido enviado\n")
	ERROR_ASSERT_PRINTF(this->memory_order_buffer_write[this->memory_order_buffer_write_start].mem_deps_ptr_array[0] == NULL, "Removendo sem resolver dependencias\n%s\n%s\n",this->memory_order_buffer_write[this->memory_order_buffer_write_start].rob_ptr->content_to_string().c_str(),this->memory_order_buffer_write[this->memory_order_buffer_write_start].content_to_string().c_str())
	this->memory_order_buffer_write_used--;
	this->memory_order_buffer_write[this->memory_order_buffer_write_start].package_clean();
	this->memory_order_buffer_write_start++;
	if (this->memory_order_buffer_write_start >= MOB_WRITE)
	{
		this->memory_order_buffer_write_start = 0;
	}
}
// ============================================================================


void processor_t::fetch(){
	#if FETCH_DEBUG
		ORCS_PRINTF("Fetch Stage\n")
	#endif
	opcode_package_t operation;
	// uint32_t position;
	// Trace ->fetchBuffer
	for (int i = 0; i < FETCH_WIDTH; i++)
	{
		operation.package_clean();
		bool updated = false;
		//=============================
		//Stall full fetch buffer
		//=============================
		if (this->fetchBuffer.is_full())
		{
			this->add_stall_full_FetchBuffer();
			break;
		}
		//=============================
		//Stall branch wrong predict
		//=============================
		if (this->get_stall_wrong_branch() > orcs_engine.get_global_cycle())
		{
			break;
		}
		//=============================
		//Get new Opcode
		//=============================
		if (!orcs_engine.trace_reader[this->processor_id].trace_fetch(&operation))
		{
			this->traceIsOver = true;
			break;
		}
		#if FETCH_DEBUG
			
				ORCS_PRINTF("Opcode Fetched %s\n", operation.content_to_string2().c_str())
			
		#endif
		//============================
		//add control variables
		//============================
		operation.opcode_number = this->fetchCounter;
		this->fetchCounter++;
		//============================
		///Solve Branch
		//============================

		if (this->hasBranch)
		{
			//solve
			uint32_t stallWrongBranch = orcs_engine.branchPredictor[this->processor_id].solveBranch(this->previousBranch, operation);
			this->set_stall_wrong_branch(orcs_engine.get_global_cycle() + stallWrongBranch);
			this->hasBranch = false;
			uint32_t ttc = orcs_engine.cacheManager->searchInstruction(this->processor_id,operation.opcode_address);
			// ORCS_PRINTF("ready after wrong branch %lu\n",this->get_stall_wrong_branch()+ttc)
			operation.updatePackageReady(FETCH_LATENCY+stallWrongBranch + ttc);
			updated = true;
			this->previousBranch.package_clean();
			// ORCS_PRINTF("Stall Wrong Branch %u\n",stallWrongBranch)
		}
		//============================
		// Operation Branch, set flag
		//============================
		if (operation.opcode_operation == INSTRUCTION_OPERATION_BRANCH)
		{
			orcs_engine.branchPredictor[this->processor_id].branches++;
			this->previousBranch = operation;
			this->hasBranch = true;
		}
		//============================
		//Insert into fetch buffer
		//============================
		if (POSITION_FAIL == this->fetchBuffer.push_back(operation))
		{
			break;
		}
		if (!updated)
		{
			uint32_t ttc = orcs_engine.cacheManager->searchInstruction(this->processor_id,operation.opcode_address);
			this->fetchBuffer.back()->updatePackageReady(FETCH_LATENCY + ttc);
			
		}
	}
		// #if FETCH_DEBUG
		// 	if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
		// 		for(uint32_t i = 0;i < this->fetchBuffer.size;i++){
		// 			ORCS_PRINTF("Opcode list-> %s\n", this->fetchBuffer[i].content_to_string2().c_str())

		// 		}
		// 	}
		// #endif
}
// ============================================================================
/*
	===========================
	Elimina os elementos do fetch buffer
	============================================================================
	Divide the opcode into
	1st. uop READ MEM. + unaligned
	2st. uop READ 2 MEM. + unaligned
	3rd. uop BRANCH
	4th. uop ALU
	5th. uop WRITE MEM. + unaligned
	============================================================================
	To maintain the right dependencies between the uops and opcodes
	If the opcode generates multiple uops, they must be in this format:

	READ    ReadRegs    = BaseRegs + IndexRegs
			WriteRegs   = 258 (Aux Register)

	ALU     ReadRegs    = * + 258 (Aux Register) (if is_read)
			WriteRegs   = * + 258 (Aux Register) (if is_write)

	WRITE   ReadRegs    = * + 258 (Aux Register)
			WriteRegs   = NULL
	============================================================================
*/
void processor_t::decode(){

	#if DECODE_DEBUG
		ORCS_PRINTF("Decode Stage\n")
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
				ORCS_PRINTF("Opcode to decode %s\n", this->fetchBuffer.front()->content_to_string2().c_str())
			}
	#endif
	uop_package_t new_uop;
	int32_t statusInsert = POSITION_FAIL;
	for (size_t i = 0; i < DECODE_WIDTH; i++)
	{
		if (this->fetchBuffer.is_empty() ||
			this->fetchBuffer.front()->status != PACKAGE_STATE_READY ||
			this->fetchBuffer.front()->readyAt > orcs_engine.get_global_cycle())
		{
			break;
		}
		if (this->decodeBuffer.get_capacity() - this->decodeBuffer.get_size() < MAX_UOP_DECODED)
		{
			this->add_stall_full_DecodeBuffer();
			break;
		}
		ERROR_ASSERT_PRINTF(this->decodeCounter == this->fetchBuffer.front()->opcode_number, "Trying decode out-of-order");
		this->decodeCounter++;

		// =====================
		//Decode Read 1
		// =====================
		if (this->fetchBuffer.front()->is_read)
		{
			new_uop.package_clean();
			//creating uop
			new_uop.opcode_to_uop(this->uopCounter++,
								  INSTRUCTION_OPERATION_MEM_LOAD,
								  this->fetchBuffer.front()->read_address,
								  this->fetchBuffer.front()->read_size,
								  *this->fetchBuffer.front());
			//SE OP DIFERE DE LOAD, ZERA REGISTERS
			if (this->fetchBuffer.front()->opcode_operation != INSTRUCTION_OPERATION_MEM_LOAD)
			{
				// ===== Read Regs =============================================
				/// Clear RRegs
				for (uint32_t i = 0; i < MAX_REGISTERS; i++)
				{
					new_uop.read_regs[i] = POSITION_FAIL;
				}
				/// Insert BASE and INDEX into RReg
				new_uop.read_regs[0] = this->fetchBuffer.front()->base_reg;
				new_uop.read_regs[1] = this->fetchBuffer.front()->index_reg;

				// ===== Write Regs =============================================
				/// Clear WRegs
				for (uint32_t i = 0; i < MAX_REGISTERS; i++)
				{
					new_uop.write_regs[i] = POSITION_FAIL;
				}
				/// Insert 258 into WRegs
				new_uop.write_regs[0] = 258;
			}
			new_uop.updatePackageReady(DECODE_LATENCY);
			// printf("\n UOP Created %s \n",new_uop.content_to_string().c_str());
			statusInsert = this->decodeBuffer.push_back(new_uop);
	#if DECODE_DEBUG
				ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
	#endif
			ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL, "Erro, Tentando decodificar mais uops que o maximo permitido")
		}
		// =====================
		//Decode Read 2
		// =====================
		if (this->fetchBuffer.front()->is_read2)
		{
			new_uop.package_clean();
			//creating uop
			new_uop.opcode_to_uop(this->uopCounter++,
								  INSTRUCTION_OPERATION_MEM_LOAD,
								  this->fetchBuffer.front()->read2_address,
								  this->fetchBuffer.front()->read2_size,
								  *this->fetchBuffer.front());
			//SE OP DIFERE DE LOAD, ZERA REGISTERS
			if (this->fetchBuffer.front()->opcode_operation != INSTRUCTION_OPERATION_MEM_LOAD)
			{
				// ===== Read Regs =============================================
				/// Clear RRegs
				for (uint32_t i = 0; i < MAX_REGISTERS; i++)
				{
					new_uop.read_regs[i] = POSITION_FAIL;
				}
				/// Insert BASE and INDEX into RReg
				new_uop.read_regs[0] = this->fetchBuffer.front()->base_reg;
				new_uop.read_regs[1] = this->fetchBuffer.front()->index_reg;

				// ===== Write Regs =============================================
				/// Clear WRegs
				for (uint32_t i = 0; i < MAX_REGISTERS; i++)
				{
					new_uop.write_regs[i] = POSITION_FAIL;
				}
				/// Insert 258 into WRegs
				new_uop.write_regs[0] = 258;
			}
			new_uop.updatePackageReady(DECODE_LATENCY);
			// printf("\n UOP Created %s \n",new_uop.content_to_string().c_str());
			statusInsert = this->decodeBuffer.push_back(new_uop);
	#if DECODE_DEBUG
				ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
	#endif
			ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL, "Erro, Tentando decodificar mais uops que o maximo permitido")
		}
		// =====================
		//Decode ALU Operation
		// =====================
		if (this->fetchBuffer.front()->opcode_operation != INSTRUCTION_OPERATION_BRANCH &&
			this->fetchBuffer.front()->opcode_operation != INSTRUCTION_OPERATION_MEM_LOAD &&
			this->fetchBuffer.front()->opcode_operation != INSTRUCTION_OPERATION_MEM_STORE)
		{
			new_uop.package_clean();
			new_uop.opcode_to_uop(this->uopCounter++,
								  this->fetchBuffer.front()->opcode_operation,
								  0,
								  0,
								  *this->fetchBuffer.front());

			if (this->fetchBuffer.front()->is_read || this->fetchBuffer.front()->is_read2)
			{
				// printf("\n UOP Created %s \n",new_uop.content_to_string().c_str());
				// ===== Read Regs =============================================
				//registers /258 aux onde pos[i] = fail
				bool inserted_258 = false;
				for (uint32_t i = 0; i < MAX_REGISTERS; i++)
				{
					// ORCS_PRINTF("read reg %d\n",new_uop.read_regs[i])
					if (new_uop.read_regs[i] == POSITION_FAIL)
					{
						// ORCS_PRINTF("read reg2 %d\n",new_uop.read_regs[i])
						new_uop.read_regs[i] = 258;
						inserted_258 = true;
						break;
					}
				}
				ERROR_ASSERT_PRINTF(inserted_258, "Could not insert register_258, all MAX_REGISTERS(%d) used.\n", MAX_REGISTERS)
			}
			if (this->fetchBuffer.front()->is_write)
			{
				// ===== Write Regs =============================================
				//registers /258 aux onde pos[i] = fail
				bool inserted_258 = false;
				for (uint32_t i = 0; i < MAX_REGISTERS; i++)
				{
					if (new_uop.write_regs[i] == POSITION_FAIL)
					{
						new_uop.write_regs[i] = 258;
						inserted_258 = true;
						break;
					}
				}
				ERROR_ASSERT_PRINTF(inserted_258, "Could not insert register_258, all MAX_REGISTERS(%d) used.\n", MAX_REGISTERS)
				// assert(!inserted_258 && "Max registers used");
			}
			new_uop.updatePackageReady(DECODE_LATENCY);
			statusInsert = this->decodeBuffer.push_back(new_uop);
	#if DECODE_DEBUG
				ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
	#endif
			ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL, "Erro, Tentando decodificar mais uops que o maximo permitido")
		}
		// =====================
		//Decode Branch
		// =====================
		if (this->fetchBuffer.front()->opcode_operation == INSTRUCTION_OPERATION_BRANCH)
		{
			new_uop.package_clean();
			new_uop.opcode_to_uop(this->uopCounter++,
								  INSTRUCTION_OPERATION_BRANCH,
								  0,
								  0,
								  *this->fetchBuffer.front());
			if (this->fetchBuffer.front()->is_read || this->fetchBuffer.front()->is_read2)
			{
				// ===== Read Regs =============================================
				/// Insert Reg258 into RReg
				bool inserted_258 = false;
				for (uint32_t i = 0; i < MAX_REGISTERS; i++)
				{
					if (new_uop.read_regs[i] == POSITION_FAIL)
					{
						new_uop.read_regs[i] = 258;
						inserted_258 = true;
						break;
					}
				}
				ERROR_ASSERT_PRINTF(inserted_258, "Could not insert register_258, all MAX_REGISTERS(%d) used.", MAX_REGISTERS)
			}
			if (this->fetchBuffer.front()->is_write)
			{
				// ===== Write Regs =============================================
				//registers /258 aux onde pos[i] = fail
				bool inserted_258 = false;
				for (uint32_t i = 0; i < MAX_REGISTERS; i++)
				{
					if (new_uop.write_regs[i] == POSITION_FAIL)
					{
						new_uop.write_regs[i] = 258;
						inserted_258 = true;
						break;
					}
				}
				ERROR_ASSERT_PRINTF(inserted_258, "Todos Max regs usados. %u \n", MAX_REGISTERS)
			}
			new_uop.updatePackageReady(DECODE_LATENCY);
			statusInsert = this->decodeBuffer.push_back(new_uop);
	#if DECODE_DEBUG
				ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
	#endif
			ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL, "Erro, Tentando decodificar mais uops que o maximo permitido")
		}
		// =====================
		//Decode Write
		// =====================
		if (this->fetchBuffer.front()->is_write)
		{
			new_uop.package_clean();
			// make package
			new_uop.opcode_to_uop(this->uopCounter++,
								  INSTRUCTION_OPERATION_MEM_STORE,
								  this->fetchBuffer.front()->write_address,
								  this->fetchBuffer.front()->write_size,
								  *this->fetchBuffer.front());
			//
			if (this->fetchBuffer.front()->opcode_operation != INSTRUCTION_OPERATION_MEM_STORE)
			{
				bool inserted_258 = false;
				for (uint32_t i = 0; i < MAX_REGISTERS; i++)
				{
					if (new_uop.read_regs[i] == POSITION_FAIL)
					{
						new_uop.read_regs[i] = 258;
						inserted_258 = true;
						break;
					}
				}
				ERROR_ASSERT_PRINTF(inserted_258, "Could not insert register_258, all MAX_REGISTERS(%d) used.", MAX_REGISTERS)
				// assert(!inserted_258 && "Max registers used");
				// ===== Write Regs =============================================
				/// Clear WRegs
				for (uint32_t i = 0; i < MAX_REGISTERS; i++)
				{
					new_uop.write_regs[i] = POSITION_FAIL;
				}
			}
			new_uop.updatePackageReady(DECODE_LATENCY);
			// printf("\n UOP Created %s \n",new_uop.content_to_string().c_str());
			statusInsert = this->decodeBuffer.push_back(new_uop);
	#if DECODE_DEBUG
				ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
	#endif
			ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL, "Erro, Tentando decodificar mais uops que o maximo permitido")
		}
		this->fetchBuffer.pop_front();
	}
}

// ============================================================================
void processor_t::update_registers(reorder_buffer_line_t *new_rob_line){
	/// Control the Register Dependency - Register READ
	for (uint32_t k = 0; k < MAX_REGISTERS; k++)
	{
		if (new_rob_line->uop.read_regs[k] < 0)
		{
			break;
		}
		uint32_t read_register = new_rob_line->uop.read_regs[k];
		ERROR_ASSERT_PRINTF(read_register < RAT_SIZE, "Read Register (%d) > Register Alias Table Size (%d)\n", read_register, RAT_SIZE);
		/// If there is a dependency
		if (this->register_alias_table[read_register] != NULL)
		{
			for (uint32_t j = 0; j < ROB_SIZE; j++)
			{
				if (this->register_alias_table[read_register]->reg_deps_ptr_array[j] == NULL)
				{
					this->register_alias_table[read_register]->wake_up_elements_counter++;
					this->register_alias_table[read_register]->reg_deps_ptr_array[j] = new_rob_line;
					new_rob_line->wait_reg_deps_number++;
					break;
				}
			}
		}
	}

	/// Control the Register Dependency - Register WRITE
	for (uint32_t k = 0; k < MAX_REGISTERS; k++)
	{
		this->add_registerWrite();
		if (new_rob_line->uop.write_regs[k] < 0)
		{
			break;
		}
		uint32_t write_register = new_rob_line->uop.write_regs[k];
		ERROR_ASSERT_PRINTF(write_register < RAT_SIZE, "Write Register (%d) > Register Alias Table Size (%d)\n", write_register, RAT_SIZE);

		this->register_alias_table[write_register] = new_rob_line;
	}
}
// ============================================================================
void processor_t::rename(){
	#if RENAME_DEBUG
		ORCS_PRINTF("Rename Stage\n")
	#endif
	size_t i;
	int32_t pos_rob, pos_mob;

	for (i = 0; i < RENAME_WIDTH; i++)
	{
		memory_order_buffer_line_t *mob_line = NULL;
		// Checando se há uop decodificado, se está pronto, e se o ciclo de pronto
		// é maior ou igual ao atual
		if (this->decodeBuffer.is_empty() ||
			this->decodeBuffer.front()->status != PACKAGE_STATE_READY ||
			this->decodeBuffer.front()->readyAt > orcs_engine.get_global_cycle())
		{
			break;
		}
		ERROR_ASSERT_PRINTF(this->decodeBuffer.front()->uop_number == this->renameCounter, "Erro, renomeio incorreto\n")
		//=======================
		// Memory Operation Read
		//=======================
		if (this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_MEM_LOAD)
		{
			if(	this->memory_order_buffer_read_used>=MOB_READ ||
				this->robUsed>=ROB_SIZE )break;
			
			pos_mob = this->search_position_mob_read();
			if (pos_mob == POSITION_FAIL)
			{
				#if RENAME_DEBUG
					ORCS_PRINTF("Stall_MOB_Read_Full\n")
				#endif
				this->add_stall_full_MOB_Read();
				break;
			}
			#if RENAME_DEBUG
				ORCS_PRINTF("Get_Position_MOB_READ %d\n",pos_mob)
			#endif
			mob_line = &this->memory_order_buffer_read[pos_mob];
		}
		//=======================
		// Memory Operation Write
		//=======================
		if (this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_MEM_STORE)
		{
			if(	this->memory_order_buffer_write_used>=MOB_WRITE ||
				this->robUsed>=ROB_SIZE )break;
			pos_mob = this->search_position_mob_write();
			if (pos_mob == POSITION_FAIL)
			{
				#if RENAME_DEBUG
					ORCS_PRINTF("Stall_MOB_Read_Full\n")
				#endif
				this->add_stall_full_MOB_Write();
				break;
			}
			#if RENAME_DEBUG
				ORCS_PRINTF("Get_Position_MOB_WRITE %d\n",pos_mob)
			#endif
			mob_line = &this->memory_order_buffer_write[pos_mob];
		}
		//=======================
		// Verificando se tem espaco no ROB se sim bamos inserir
		//=======================
		pos_rob = this->searchPositionROB();
		if (pos_rob == POSITION_FAIL)
		{
			#if RENAME_DEBUG
				ORCS_PRINTF("Stall_MOB_Read_Full\n")
			#endif
			this->add_stall_full_ROB();
			break;
		}
		// ===============================================
		// Insserting on ROB
		// ===============================================
		this->reorderBuffer[pos_rob].uop = *this->decodeBuffer.front();
		//remove uop from decodebuffer
		this->decodeBuffer.front()->package_clean();
		this->decodeBuffer.pop_front();
		this->renameCounter++;

		// =======================
		// Setting controls to ROB.
		// =======================
		this->reorderBuffer[pos_rob].stage = PROCESSOR_STAGE_RENAME;
		this->reorderBuffer[pos_rob].uop.updatePackageReady(RENAME_LATENCY + DISPATCH_LATENCY);
		this->reorderBuffer[pos_rob].mob_ptr = mob_line;
		this->reorderBuffer[pos_rob].processor_id = this->processor_id;
		// =======================
		// Making registers dependences
		// =======================
		this->update_registers(&this->reorderBuffer[pos_rob]);
	#if RENAME_DEBUG
			ORCS_PRINTF("Rename %s\n", this->reorderBuffer[pos_rob].content_to_string().c_str())
	#endif
		// =======================
		// Insert into Reservation Station
		// =======================
		this->unified_reservation_station.push_back(&this->reorderBuffer[pos_rob]);
		// =======================
		// Insert into MOB.
		// =======================
		if (this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD)
		{
	#if RENAME_DEBUG
				ORCS_PRINTF("Mem Load\n")
	#endif
			this->reorderBuffer[pos_rob].mob_ptr->opcode_address = this->reorderBuffer[pos_rob].uop.opcode_address;
			this->reorderBuffer[pos_rob].mob_ptr->memory_address = this->reorderBuffer[pos_rob].uop.memory_address;
			this->reorderBuffer[pos_rob].mob_ptr->memory_size = this->reorderBuffer[pos_rob].uop.memory_size;
			this->reorderBuffer[pos_rob].mob_ptr->memory_operation = MEMORY_OPERATION_READ;
			this->reorderBuffer[pos_rob].mob_ptr->status = PACKAGE_STATE_WAIT;
			this->reorderBuffer[pos_rob].mob_ptr->readyToGo = orcs_engine.get_global_cycle() + RENAME_LATENCY + DISPATCH_LATENCY;
			this->reorderBuffer[pos_rob].mob_ptr->uop_number = this->reorderBuffer[pos_rob].uop.uop_number;
			this->reorderBuffer[pos_rob].mob_ptr->processor_id = this->processor_id;
		}
		else if (this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_MEM_STORE)
		{
	#if RENAME_DEBUG
				ORCS_PRINTF("Mem Store\n")
	#endif
			this->reorderBuffer[pos_rob].mob_ptr->opcode_address = this->reorderBuffer[pos_rob].uop.opcode_address;
			this->reorderBuffer[pos_rob].mob_ptr->memory_address = this->reorderBuffer[pos_rob].uop.memory_address;
			this->reorderBuffer[pos_rob].mob_ptr->memory_size = this->reorderBuffer[pos_rob].uop.memory_size;
			this->reorderBuffer[pos_rob].mob_ptr->memory_operation = MEMORY_OPERATION_WRITE;
			this->reorderBuffer[pos_rob].mob_ptr->status = PACKAGE_STATE_WAIT;
			this->reorderBuffer[pos_rob].mob_ptr->readyToGo = orcs_engine.get_global_cycle() + RENAME_LATENCY + DISPATCH_LATENCY;
			this->reorderBuffer[pos_rob].mob_ptr->uop_number = this->reorderBuffer[pos_rob].uop.uop_number;
			this->reorderBuffer[pos_rob].mob_ptr->processor_id = this->processor_id;
		}
		//linking rob and mob
		if (this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD ||
			this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_MEM_STORE)
		{
			mob_line->rob_ptr = &this->reorderBuffer[pos_rob];
	#if DESAMBIGUATION_ENABLED
				this->desambiguator->make_memory_dependences(this->reorderBuffer[pos_rob].mob_ptr);
	#endif
		}
	} //end for
}
// ============================================================================
void processor_t::dispatch(){
	#if DISPATCH_DEBUG
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("====================================================================\n")
			ORCS_PRINTF("Dispatch Stage\n")
			ORCS_PRINTF("====================================================================\n")
		}
	#endif
		//control variables
		uint32_t total_dispatched = 0;
		/// Control the total dispatched per FU
		uint32_t fu_int_alu = 0;
		uint32_t fu_int_mul = 0;
		uint32_t fu_int_div = 0;

		uint32_t fu_fp_alu = 0;
		uint32_t fu_fp_mul = 0;
		uint32_t fu_fp_div = 0;

		uint32_t fu_mem_load = 0;
		uint32_t fu_mem_store = 0;

		for (uint32_t i = 0; i < this->unified_reservation_station.size() && i < UNIFIED_RS; i++)
		{
			//pointer to entry
			reorder_buffer_line_t *rob_line = this->unified_reservation_station[i];
			#if DISPATCH_DEBUG
				if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
					ORCS_PRINTF("cycle %lu\n", orcs_engine.get_global_cycle())
					ORCS_PRINTF("=================\n")
					ORCS_PRINTF("Unified Reservations Station on use: %lu\n",this->unified_reservation_station.size())
					ORCS_PRINTF("Trying Dispatch %s\n", rob_line->content_to_string().c_str())
					ORCS_PRINTF("=================\n")
				}
			#endif
			if (rob_line->emc_executed == true){
				this->unified_reservation_station.erase(this->unified_reservation_station.begin() + i);
				i--;
				#if DISPATCH_DEBUG
					if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
						ORCS_PRINTF("=================\n")
						ORCS_PRINTF("Removed By executed On EMC %s\n", rob_line->content_to_string().c_str())
						ORCS_PRINTF("=================\n")
					}
				#endif
				continue;
			}
			if (rob_line->sent_to_emc == true){
				continue;
			}
			
			if (total_dispatched >= DISPATCH_WIDTH)
			{
				break;
			}
			
			if ((rob_line->uop.readyAt <= orcs_engine.get_global_cycle()) &&
				(rob_line->wait_reg_deps_number == 0)){
				ERROR_ASSERT_PRINTF(rob_line->uop.status == PACKAGE_STATE_READY, "Error, uop not ready being dispatched\n %s\n", rob_line->content_to_string().c_str())
				ERROR_ASSERT_PRINTF(rob_line->stage == PROCESSOR_STAGE_RENAME, "Error, uop not in Rename to rename stage\n %s\n",rob_line->content_to_string().c_str())
				//if dispatched
				bool dispatched = false;
				switch (rob_line->uop.uop_operation)
				{
				// NOP operation
				case INSTRUCTION_OPERATION_NOP:
				// integer alu// add/sub/logical
				case INSTRUCTION_OPERATION_INT_ALU:
				// branch op. como fazer, branch solved on fetch
				case INSTRUCTION_OPERATION_BRANCH:
				// op not defined
				case INSTRUCTION_OPERATION_OTHER:
					if (fu_int_alu < INTEGER_ALU)
					{
						for (uint8_t k = 0; k < INTEGER_ALU; k++)
						{
							if (this->fu_int_alu[k] <= orcs_engine.get_global_cycle())
							{
								//Branch instruction, ????
								if (rob_line->uop.uop_operation == INSTRUCTION_OPERATION_BRANCH)
								{
									//do something
								}
								this->fu_int_alu[k] = orcs_engine.get_global_cycle() + WAIT_NEXT_INT_ALU;
								fu_int_alu++;
								dispatched = true;
								rob_line->stage = PROCESSOR_STAGE_EXECUTION;
								rob_line->uop.updatePackageReady(LATENCY_INTEGER_ALU);
								break;
							}
						}
					}
					break;
				// ====================================================
				// Integer Multiplication
				case INSTRUCTION_OPERATION_INT_MUL:
					if (fu_int_mul < INTEGER_MUL)
					{
						for (uint8_t k = 0; k < INTEGER_MUL; k++)
						{
							if (this->fu_int_mul[k] <= orcs_engine.get_global_cycle())
							{
								this->fu_int_mul[k] = orcs_engine.get_global_cycle() + WAIT_NEXT_INT_MUL;
								fu_int_mul++;
								dispatched = true;
								rob_line->stage = PROCESSOR_STAGE_EXECUTION;
								rob_line->uop.updatePackageReady(LATENCY_INTEGER_MUL);
								break;
							}
						}
					}
					break;
				// ====================================================
				// Integer division
				case INSTRUCTION_OPERATION_INT_DIV:
					if (fu_int_div < INTEGER_DIV)
					{
						for (uint8_t k = 0; k < INTEGER_DIV; k++)
						{
							if (this->fu_int_div[k] <= orcs_engine.get_global_cycle())
							{
								this->fu_int_div[k] = orcs_engine.get_global_cycle() + WAIT_NEXT_INT_DIV;
								fu_int_div++;
								dispatched = true;
								rob_line->stage = PROCESSOR_STAGE_EXECUTION;
								rob_line->uop.updatePackageReady(LATENCY_INTEGER_DIV);
								break;
							}
						}
					}
					break;
				// ====================================================
				// Floating point ALU operation
				case INSTRUCTION_OPERATION_FP_ALU:
					if (fu_fp_alu < FP_ALU)
					{
						for (uint8_t k = 0; k < FP_ALU; k++)
						{
							if (this->fu_fp_alu[k] <= orcs_engine.get_global_cycle())
							{
								this->fu_fp_alu[k] = orcs_engine.get_global_cycle() + WAIT_NEXT_FP_ALU;
								fu_fp_alu++;
								dispatched = true;
								rob_line->stage = PROCESSOR_STAGE_EXECUTION;
								rob_line->uop.updatePackageReady(LATENCY_FP_ALU);
								break;
							}
						}
					}
					break;
				// ====================================================
				// Floating Point Multiplication
				case INSTRUCTION_OPERATION_FP_MUL:
					if (fu_fp_mul < FP_MUL)
					{
						for (uint8_t k = 0; k < FP_MUL; k++)
						{
							if (this->fu_fp_mul[k] <= orcs_engine.get_global_cycle())
							{
								this->fu_fp_mul[k] = orcs_engine.get_global_cycle() + WAIT_NEXT_FP_MUL;
								fu_fp_mul++;
								dispatched = true;
								rob_line->stage = PROCESSOR_STAGE_EXECUTION;
								rob_line->uop.updatePackageReady(LATENCY_FP_MUL);
								break;
							}
						}
					}
					break;

				// ====================================================
				// Floating Point Division
				case INSTRUCTION_OPERATION_FP_DIV:
					if (fu_fp_div < FP_DIV)
					{
						for (uint8_t k = 0; k < FP_DIV; k++)
						{
							if (this->fu_fp_div[k] <= orcs_engine.get_global_cycle())
							{
								this->fu_fp_div[k] = orcs_engine.get_global_cycle() + WAIT_NEXT_FP_DIV;
								fu_fp_div++;
								dispatched = true;
								rob_line->stage = PROCESSOR_STAGE_EXECUTION;
								rob_line->uop.updatePackageReady(LATENCY_FP_DIV);
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
								rob_line->stage = PROCESSOR_STAGE_EXECUTION;
								rob_line->uop.updatePackageReady(LATENCY_MEM_LOAD);
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
								rob_line->stage = PROCESSOR_STAGE_EXECUTION;
								rob_line->uop.updatePackageReady(LATENCY_MEM_STORE);
								break;
							}
						}
					}
					break;

				// ====================================================
				case INSTRUCTION_OPERATION_BARRIER:
				case INSTRUCTION_OPERATION_HMC_ROA:
				case INSTRUCTION_OPERATION_HMC_ROWA:
					ERROR_PRINTF("Invalid instruction BARRIER||HMC_ROA||HMC_ROWA being dispatched.\n");
					break;
				} //end switch
				//remover os postos em execucao aqui
				if (dispatched == true)
				{
			#if DISPATCH_DEBUG
					if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
						ORCS_PRINTF("Dispatched %s\n", rob_line->content_to_string().c_str())
						ORCS_PRINTF("===================================================================\n")
					}
			#endif
					// update Dispatched
					total_dispatched++;
					// insert on FUs waiting structure
					this->unified_functional_units.push_back(rob_line);
					// remove from reservation station
					this->unified_reservation_station.erase(this->unified_reservation_station.begin() + i);
					i--;
				} //end 	if dispatched
			}	 //end if robline is ready
		}		  //end for
		// sleep(1);
} //end method
// ============================================================================
void processor_t::execute()
{
	#if EXECUTE_DEBUG
		if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
			ORCS_PRINTF("=========================================================================\n")
			ORCS_PRINTF("========== Execute Stage ==========\n")
		}
	#endif
	// ==================================
	// verificar leituras prontas no ciclo,
	// remover do MOB e atualizar os registradores,
	// ==================================
	uint32_t pos = this->memory_order_buffer_read_start;
	for (uint8_t i = 0; i < this->memory_order_buffer_read_used; i++){
		if (this->memory_order_buffer_read[pos].status == PACKAGE_STATE_READY &&
			this->memory_order_buffer_read[pos].readyAt <= orcs_engine.get_global_cycle() &&
			this->memory_order_buffer_read[pos].processed == false){
			ERROR_ASSERT_PRINTF(this->memory_order_buffer_read[pos].uop_executed == true, "Removing memory read before being executed.\n")
			ERROR_ASSERT_PRINTF(this->memory_order_buffer_read[pos].wait_mem_deps_number == 0, "Number of memory dependencies should be zero.\n %s\n",this->memory_order_buffer_read[i].rob_ptr->content_to_string().c_str())
			#if EXECUTE_DEBUG
				if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
					ORCS_PRINTF("\nSolving %s\n\n", this->memory_order_buffer_read[pos].rob_ptr->content_to_string().c_str())
				}
			#endif
			#if EMC_ACTIVE_DEBUG
				if(orcs_engine.get_global_cycle()>WAIT_CYCLE && this->memory_order_buffer_read[pos].emc_executed ==true){
					ORCS_PRINTF("\nSolving_EMC_REQ %s\n\n", this->memory_order_buffer_read[pos].rob_ptr->content_to_string().c_str())
				}
			#endif
			this->memory_order_buffer_read[pos].rob_ptr->stage = PROCESSOR_STAGE_COMMIT;
			this->memory_order_buffer_read[pos].rob_ptr->uop.updatePackageReady(COMMIT_LATENCY);
			this->memory_order_buffer_read[pos].processed=true;
			this->memory_read_executed--;
			this->solve_registers_dependency(this->memory_order_buffer_read[pos].rob_ptr);
			#if DESAMBIGUATION_ENABLED
				this->desambiguator->solve_memory_dependences(&this->memory_order_buffer_read[pos]);
			#endif
			#if PARALLEL_LIM_ACTIVE
				if(!this->memory_order_buffer_read[pos].forwarded_data && !this->memory_order_buffer_read[pos].emc_executed){
						ERROR_ASSERT_PRINTF(this->counter_mshr_read > 0,"ERRO, Contador negativo READ\n")
						this->counter_mshr_read--;
				}
			#endif
			if(this->memory_order_buffer_read[pos].waiting_DRAM && !this->memory_order_buffer_read[pos].emc_executed){
				ERROR_ASSERT_PRINTF(this->request_DRAM > 0,"ERRO, Contador negativo Waiting DRAM\n")
				#if EXECUTE_DEBUG
					if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
						ORCS_PRINTF("\nReducing DRAM COUNTER\n\n")
					}
			#endif
				this->request_DRAM--;
			}
		}
		pos++;
		if( pos >= MOB_READ) pos=0;
	}
	
	#if EMC_ACTIVE
		if (this->start_emc_module){	
		#if EMC_ACTIVE_DEBUG
			if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
				for(uint32_t i=0;i<this->rob_buffer.size();i++){
						ORCS_PRINTF("%s\n\n",this->rob_buffer[i]->content_to_string().c_str())
					}
				}
		#endif
		// ======================================================================
		// Contando numero de loads dependentes nas cadeias elegiveis para execução
			this->verify_dependent_loads();
		// ======================================================================
			uint32_t instruction_dispatched_emc = 0;
			for(uint32_t i=0;i<this->rob_buffer.size();i++){
				bool renamed_emc=false;
				reorder_buffer_line_t *rob_next = this->rob_buffer.front();	
				if(rob_next->uop.uop_operation == INSTRUCTION_OPERATION_MEM_STORE){
					if(!this->verify_spill_register(rob_next)){
						this->rob_buffer.erase(this->rob_buffer.begin());
						i--;
						continue;
					}else{
						orcs_engine.memory_controller->emc[this->processor_id].has_store = true;
					}
				}
				#if EMC_ACTIVE_DEBUG
					if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
						ORCS_PRINTF("Renaming to EMC %s\n\n",rob_next->content_to_string().c_str())
					}
				#endif
				////////////////////////////////////////
				int32_t status = this->renameEMC(rob_next);
				if( status == POSITION_FAIL){
					#if EMC_ACTIVE_DEBUG
						if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
							ORCS_PRINTF("Impossivel Renomear EMC, estruturas cheias\n %s\n",rob_next->content_to_string().c_str())
						}
					#endif
					this->rob_buffer.clear();	
				}else if(status == NOT_ALL_REGS){
					this->rob_buffer.erase(this->rob_buffer.begin());
					i--;
				}
				else{
					rob_next->sent_to_emc=true;
					renamed_emc=true;
					instruction_dispatched_emc++;
				}
				////////////////////////////////////////
				if(renamed_emc==true){
					this->rob_buffer.erase(this->rob_buffer.begin());
					i--;
				}
			}
			// Verifica se o rob_buffer está vazio, ou uop buffer cheio
			if ((orcs_engine.memory_controller->emc[this->processor_id].uop_buffer_used >= EMC_UOP_BUFFER) || (this->rob_buffer.empty())){
				this->start_emc_module = false; // disable emc module CORE
				this->rob_buffer.clear();		// flush core buffer
					orcs_engine.memory_controller->emc[this->processor_id].ready_to_execute = true; //execute emc
					orcs_engine.memory_controller->emc[this->processor_id].executed = true; //print dep chain emc //comentar depois
					// orcs_engine.memory_controller->emc[this->processor_id].print_structures(); //print dep chain emc //comentar depois
					
					#if EMC_ACTIVE_DEBUG
					if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
						ORCS_PRINTF("Locking Processor\n")
						#if PRINT_ROB
							this->print_ROB();
						#endif
					}
				#endif
				// ERROR_ASSERT_PRINTF(orcs_engine.memory_controller->emc_active > EMC_PARALLEL_ACTIVATE,"Error, tentando executar mais EMCs em paralelo que o permitido\n ")
				// orcs_engine.memory_controller->emc_active++;
				this->total_instruction_sent_emc+=instruction_dispatched_emc;
				#if LOCKING_COMMIT
					this->lock_processor=true;
				#endif
				this->clean_rrt(); //Limpa RRT;
			}
		}
	#endif

	uint32_t uop_total_executed = 0;
	for (uint32_t i = 0; i < this->unified_functional_units.size(); i++){

		reorder_buffer_line_t *rob_line = this->unified_functional_units[i];
		if (uop_total_executed == EXECUTE_WIDTH){
			break;
		}
		if (rob_line == NULL){
			break;
		}
		if (rob_line->uop.readyAt <= orcs_engine.get_global_cycle()){
			ERROR_ASSERT_PRINTF(rob_line->stage == PROCESSOR_STAGE_EXECUTION, "ROB not on execution state")
			ERROR_ASSERT_PRINTF(rob_line->uop.status == PACKAGE_STATE_READY, "FU with Package not in ready state")
			switch (rob_line->uop.uop_operation){
				// =============================================================
				// BRANCHES
				case INSTRUCTION_OPERATION_BRANCH:
				// INTEGERS ===============================================
				case INSTRUCTION_OPERATION_INT_ALU:
				case INSTRUCTION_OPERATION_NOP:
				case INSTRUCTION_OPERATION_OTHER:
				case INSTRUCTION_OPERATION_INT_MUL:
				case INSTRUCTION_OPERATION_INT_DIV:
				// FLOAT POINT ===============================================
				case INSTRUCTION_OPERATION_FP_ALU:
				case INSTRUCTION_OPERATION_FP_MUL:
				case INSTRUCTION_OPERATION_FP_DIV:
				{
					rob_line->stage = PROCESSOR_STAGE_COMMIT;
					rob_line->uop.updatePackageReady(EXECUTE_LATENCY + COMMIT_LATENCY);
					this->solve_registers_dependency(rob_line);
					uop_total_executed++;
					/// Remove from the Functional Units
					this->unified_functional_units.erase(this->unified_functional_units.begin() + i);
					i--;
				}
				break;
				// MEMORY LOAD/STORE ==========================================
				case INSTRUCTION_OPERATION_MEM_LOAD:
				{
					ERROR_ASSERT_PRINTF(rob_line->mob_ptr != NULL, "Read with a NULL pointer to MOB\n%s\n",rob_line->content_to_string().c_str())
					this->memory_read_executed++;
					rob_line->mob_ptr->uop_executed = true;
					rob_line->uop.updatePackageReady(EXECUTE_LATENCY);
					uop_total_executed++;
					/// Remove from the Functional Units
					this->unified_functional_units.erase(this->unified_functional_units.begin() + i);
					i--;
				}
				break;
				case INSTRUCTION_OPERATION_MEM_STORE:
				{
					ERROR_ASSERT_PRINTF(rob_line->mob_ptr != NULL, "Write with a NULL pointer to MOB\n%s\n",rob_line->content_to_string().c_str())
					this->memory_write_executed++;
					rob_line->mob_ptr->uop_executed = true;
					rob_line->uop.updatePackageReady(EXECUTE_LATENCY);
					uop_total_executed++;
					/// Remove from the Functional Units
					this->unified_functional_units.erase(this->unified_functional_units.begin() + i);
					i--;
				}
				break;
				case INSTRUCTION_OPERATION_BARRIER:
				case INSTRUCTION_OPERATION_HMC_ROA:
				case INSTRUCTION_OPERATION_HMC_ROWA:
					ERROR_PRINTF("Invalid BARRIER | HMC ROA |HMC ROWA.\n");
					break;
			} //end switch
		#if EXECUTE_DEBUG
				if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
					ORCS_PRINTF("Executed %s\n", rob_line->content_to_string().c_str())
				}
		#endif
		} //end if ready package
	}	 //end for
	#if EXECUTE_DEBUG
		if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
			ORCS_PRINTF("Memory Operations Read Executed %u\n",this->memory_read_executed)
			ORCS_PRINTF("Memory Operations Write Executed %u\n",this->memory_write_executed)
			ORCS_PRINTF("Requests to DRAM on the Fly %d \n",this->request_DRAM)
			ORCS_PRINTF("Parallel Request Data %d \n",this->counter_mshr_read)
			ORCS_PRINTF("Parallel Write Data %d \n",this->counter_mshr_write)
		}
	#endif
		// =========================================================================
		// Verificar se foi executado alguma operação de leitura,
		//  e executar a mais antiga no MOB
		// =========================================================================
		if(this->memory_read_executed!=0){
			this->mob_read();
		}	

		// ==================================
		// Executar o MOB Write, com a escrita mais antiga.
		// depois liberar e tratar as escrita prontas;
		// ==================================

		if(this->memory_write_executed!=0){
			this->mob_write();
		}
		// =====================================
	#if EXECUTE_DEBUG
		if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
			ORCS_PRINTF("=========================================================================\n")
		}
	#endif

} //end method
// ============================================================================
memory_order_buffer_line_t* processor_t::get_next_op_load(){
	
	uint32_t pos = this->memory_order_buffer_read_start;
	for(uint32_t i = 0 ; i < this->memory_order_buffer_read_used; i++){
		if(this->memory_order_buffer_read[pos].uop_executed && 
			this->memory_order_buffer_read[pos].status == PACKAGE_STATE_WAIT && 
			this->memory_order_buffer_read[pos].sent==false && 
        	this->memory_order_buffer_read[pos].wait_mem_deps_number == 0 &&
			this->memory_order_buffer_read[pos].sent_to_emc == false &&
			this->memory_order_buffer_read[pos].readyToGo <= orcs_engine.get_global_cycle()){
				#if STORE_ONLY_ROB_HEAD
					if(this->memory_order_buffer_write_used>0){
						if(this->memory_order_buffer_read[pos].uop_number > this->memory_order_buffer_write[this->memory_order_buffer_write_start].uop_number){
							break;
						}
					}	
				#endif				
				return &this->memory_order_buffer_read[pos];
			}
		pos++;
		if( pos >= MOB_READ) pos=0;
	}
	return NULL;
}
// ============================================================================
uint32_t processor_t::mob_read(){
	#if MOB_DEBUG
		if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
			ORCS_PRINTF("==========================================================\n")
			ORCS_PRINTF("=========== MOB Read ===========\n")
			ORCS_PRINTF("Parallel Requests %d > MAX\n",this->counter_mshr_read)
			ORCS_PRINTF("MOB Read Start %u\n",this->memory_order_buffer_read_start)
			ORCS_PRINTF("MOB Read End %u\n",this->memory_order_buffer_read_end)
			ORCS_PRINTF("MOB Read Used %u\n",this->memory_order_buffer_read_used)
			#if PRINT_MOB
				if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
					memory_order_buffer_line_t::printAllOrder(this->memory_order_buffer_read,MOB_READ,this->memory_order_buffer_read_start,this->memory_order_buffer_read_used);
				}
			#endif
			if(oldest_read_to_send!=NULL){
				if(orcs_engine.get_global_cycle() > WAIT_CYCLE){
					ORCS_PRINTF("MOB Read Atual %s\n",this->oldest_read_to_send->content_to_string().c_str())
				}		
			}
		}
	#endif
	if(this->oldest_read_to_send == NULL){
		
			this->oldest_read_to_send = this->get_next_op_load();
			#if MOB_DEBUG
				if(oldest_read_to_send==NULL){
					if(orcs_engine.get_global_cycle() > WAIT_CYCLE){
						ORCS_PRINTF("Oldest Read NULL\n")
					}		
				}
			#endif
	}
	if (this->oldest_read_to_send != NULL){
		#if PARALLEL_LIM_ACTIVE
			if(this->counter_mshr_read >= MAX_PARALLEL_REQUESTS_CORE){
				this->add_times_reach_parallel_requests_read();
				return FAIL;
			}
		#endif
		#if MOB_DEBUG
			if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
				ORCS_PRINTF("=================================\n")
				ORCS_PRINTF("Sending to memory request to data\n")
				ORCS_PRINTF("%s\n",this->oldest_read_to_send->content_to_string().c_str())
				ORCS_PRINTF("=================================\n")
			}
		#endif
		uint32_t ttc = orcs_engine.cacheManager->searchData(this->oldest_read_to_send);
		this->oldest_read_to_send->updatePackageReady(ttc);
		this->oldest_read_to_send->sent=true;
		this->oldest_read_to_send->rob_ptr->sent=true;								///Setting flag which marks sent request. set to remove entry on mob at commit
		#if PARALLEL_LIM_ACTIVE
			this->counter_mshr_read++; //numero de req paralelas, add+1
		#endif
		// #if EMC_ACTIVE
		// 	if (this->has_llc_miss)
		// 	{
		// 		this->has_llc_miss = false;
		// 		#if EMC_ROB_HEAD
		// 			if (this->isRobHead(this->oldest_read_to_send->rob_ptr))
		// 			{					
		// 		#endif	
		// 			#if EMC_ROB_HEAD
		// 				this->add_llc_miss_rob_head();
		// 			#endif	
		// 			#if EMC_ACTIVE_DEBUG
		// 				if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
		// 					ORCS_PRINTF("\n\nLLC MISS on Cycle %lu\n",orcs_engine.get_global_cycle())
		// 					ORCS_PRINTF("ROB Buffer size %lu\n",this->rob_buffer.size())
		// 					ORCS_PRINTF("EMC COUNTER  %hhd\n",this->counter_activate_emc)
		// 				}
		// 			#endif
		// 			// =====================================================
		// 			// generate chain on home core buffer
		// 			// =====================================================
		// 			this->rob_buffer.push_back(oldest_read_to_send->rob_ptr);
		// 			this->make_dependence_chain(oldest_read_to_send->rob_ptr);
		// 			oldest_read_to_send->rob_ptr->original_miss = true;
		// 		#if EMC_ROB_HEAD
		// 			}
		// 		#endif
		// 	}
		// #endif
		this->oldest_read_to_send = NULL;
	} //end if mob_line null
	#if MOB_DEBUG
			if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
			ORCS_PRINTF("==========================================================\n")
		}
	#endif
	return OK;
} //end method
// ============================================================================
memory_order_buffer_line_t* processor_t::get_next_op_store(){
		uint32_t i = this->memory_order_buffer_write_start;
		if(this->memory_order_buffer_write[i].uop_executed &&
			this->memory_order_buffer_write[i].status == PACKAGE_STATE_WAIT &&  
			this->memory_order_buffer_write[i].sent ==false  && 
        	this->memory_order_buffer_write[i].wait_mem_deps_number <= 0 &&
			this->memory_order_buffer_write[i].readyToGo <= orcs_engine.get_global_cycle() &&
			this->memory_order_buffer_write[i].sent_to_emc == false)
		{
			return &this->memory_order_buffer_write[i];
		}
	return NULL;
}
// ============================================================================
uint32_t processor_t::mob_write(){
	#if MOB_DEBUG
		if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
			ORCS_PRINTF("==========================================================\n")
			ORCS_PRINTF("=========== MOB Write ===========\n")
			ORCS_PRINTF("MOB Write Start %u\n",this->memory_order_buffer_write_start)
			ORCS_PRINTF("MOB Write End %u\n",this->memory_order_buffer_write_end)
			ORCS_PRINTF("MOB Write Used %u\n",this->memory_order_buffer_write_used)
			#if PRINT_MOB
				if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
					memory_order_buffer_line_t::printAllOrder(this->memory_order_buffer_write,MOB_WRITE,this->memory_order_buffer_write_start,this->memory_order_buffer_write_used);
				}
			#endif
			if(this->oldest_write_to_send!=NULL){
				if(orcs_engine.get_global_cycle() > WAIT_CYCLE){
					ORCS_PRINTF("MOB write Atual %s\n",this->oldest_write_to_send->content_to_string().c_str())
				}		
			}
		}
	#endif
	if(this->oldest_write_to_send==NULL){
		this->oldest_write_to_send = this->get_next_op_store();
	//////////////////////////////////////
		#if MOB_DEBUG
			if(this->oldest_write_to_send==NULL){
				if(orcs_engine.get_global_cycle() > WAIT_CYCLE){
					ORCS_PRINTF("Oldest Write NULL\n")
				}		
			}
		#endif
	/////////////////////////////////////////////
	}
	if (this->oldest_write_to_send != NULL)
	{
			#if PARALLEL_LIM_ACTIVE
				if (this->counter_mshr_write >= MAX_PARALLEL_REQUESTS_CORE)
				{
					this->add_times_reach_parallel_requests_write();
					return FAIL;
				}
			#endif
		uint32_t ttc = 0;
		#if MOB_DEBUG
			if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
				ORCS_PRINTF("=================================\n")
				ORCS_PRINTF("Sending to memory WRITE to data\n")
				ORCS_PRINTF("%s\n",this->oldest_write_to_send->content_to_string().c_str())
				ORCS_PRINTF("=================================\n")
			}
		#endif

		//sendind to write data
		ttc = orcs_engine.cacheManager->writeData(oldest_write_to_send);
		// updating package
		// =============================================================
		#if STORE_ONLY_ROB_HEAD
			this->oldest_write_to_send->updatePackageReady(ttc);
			this->oldest_write_to_send->sent = true;
			this->oldest_write_to_send->rob_ptr->sent = true;				///Setting flag which marks sent request. set to remove entry on mob at commit
			#if PARALLEL_LIM_ACTIVE
				this->parallel_requests++; //numero de req paralelas, add+1
			#endif
		#else
			//ROB
			this->oldest_write_to_send->rob_ptr->stage = PROCESSOR_STAGE_COMMIT;
			this->oldest_write_to_send->rob_ptr->uop.updatePackageReady(ttc);
			this->oldest_write_to_send->rob_ptr->sent = true;	
			//MOB
			this->oldest_write_to_send->sent = true;
			this->oldest_write_to_send->updatePackageReady(ttc);
			this->solve_registers_dependency(this->oldest_write_to_send->rob_ptr);
			this->desambiguator->solve_memory_dependences(this->oldest_write_to_send);
			this->remove_front_mob_write();
			#if PARALLEL_LIM_ACTIVE
				this->counter_mshr_write++; //numero de req paralelas, add+1
			#endif
		#endif
			this->memory_write_executed--; //numero de writes executados
			this->oldest_write_to_send=NULL;
		// =============================================================	
	} //end if mob_line null
		#if MOB_DEBUG
			if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
				ORCS_PRINTF("Parallel Requests %d > MAX\n",this->counter_mshr_write)
				ORCS_PRINTF("==========================================================\n")
			}
		#endif
	return OK;
}
// ============================================================================
void processor_t::commit(){
	#if COMMIT_DEBUG
		if (orcs_engine.get_global_cycle() > WAIT_CYCLE)
		{
			ORCS_PRINTF("=========================================================================\n")
			ORCS_PRINTF("========== Commit Stage ==========\n")
			ORCS_PRINTF("Cycle %lu\n", orcs_engine.get_global_cycle())
			#if PRINT_ROB
				this->print_ROB();
			#endif
			ORCS_PRINTF("==================================\n")
		}
	#endif
	int32_t pos_buffer;
// #################################################################################
		#if EMC_ACTIVE 
			if( (this->reorderBuffer[this->robStart].uop.uop_operation==INSTRUCTION_OPERATION_MEM_LOAD) &&
				(this->reorderBuffer[this->robStart].mob_ptr->core_generate_miss) &&
				((this->reorderBuffer[this->robStart].mob_ptr->readyAt-RAM_LATENCY) == orcs_engine.get_global_cycle())
			){
				this->add_llc_miss_rob_head();
				#if EMC_ACTIVE_DEBUG
					if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
						ORCS_PRINTF("\n\nLLC MISS on Cycle %lu\n",orcs_engine.get_global_cycle())
						ORCS_PRINTF("ROB Buffer size %lu\n",this->rob_buffer.size())
						ORCS_PRINTF("EMC COUNTER  %hhd\n",this->counter_activate_emc)
					}
				#endif
					this->reorderBuffer[this->robStart].original_miss = true;
					// =====================================================
					// generate chain on home core buffer
					// =====================================================
					this->rob_buffer.push_back(&this->reorderBuffer[this->robStart]);
					this->make_dependence_chain(&this->reorderBuffer[this->robStart]);
					// =====================================================
					// Sorting chain in order
					// =====================================================
					std::sort(this->rob_buffer.begin(),this->rob_buffer.end(),[](const reorder_buffer_line_t *lhs, const reorder_buffer_line_t *rhs){
						return lhs->uop.uop_number < rhs->uop.uop_number;
					});
				if(this->rob_buffer.size()>1){
					if(this->counter_activate_emc >= EMC_THRESHOLD){
						this->start_emc_module = true;
						this->add_started_emc_execution();
						this->verify_started_emc_without_loads();
					}else{
						this->add_cancel_counter_emc_execution();
						this->verify_loads_missed();
						this->rob_buffer.clear();
					}
				}
			}
		#endif
// #################################################################################

	/// Commit the packages
	for (uint32_t i = 0; i < COMMIT_WIDTH; i++){
		#if LOCKING_COMMIT
			if(this->lock_processor){
				break;
			}
		#endif
		pos_buffer = this->robStart;
		if (this->robUsed != 0 &&
			this->reorderBuffer[pos_buffer].stage == PROCESSOR_STAGE_COMMIT &&
			this->reorderBuffer[pos_buffer].uop.status == PACKAGE_STATE_READY &&
			this->reorderBuffer[pos_buffer].uop.readyAt <= orcs_engine.get_global_cycle())
		{
		#if !LOCKING_COMMIT
		if(this->verify_uop_on_emc(&this->reorderBuffer[pos_buffer])){
			break;
		}
		#endif
			this->commit_uop_counter++;
			#if EMC_ACTIVE
				if( (this->reorderBuffer[pos_buffer].mob_ptr != NULL) && 
					(this->reorderBuffer[pos_buffer].is_poisoned)){						
						#if EMC_ACTIVE_DEBUG
						if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
							ORCS_PRINTF("EMC COUNTER  %hhd\n",this->counter_activate_emc)
						}
						#endif
					if(this->reorderBuffer[pos_buffer].mob_ptr->core_generate_miss ||
						this->reorderBuffer[pos_buffer].mob_ptr->emc_generate_miss ){
						this->update_counter_emc(1);
						#if EMC_ACTIVE_DEBUG
							if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
								ORCS_PRINTF("\n Has Dependent Cache Miss\n")
								ORCS_PRINTF("EMC COUNTER  %hhd\n",this->counter_activate_emc)
							}
						#endif
					}else{
						this->update_counter_emc(-1);
						#if EMC_ACTIVE_DEBUG
							if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
								ORCS_PRINTF("\n Has No Dependent Cache Miss\n")
								ORCS_PRINTF("EMC COUNTER  %hhd\n",this->counter_activate_emc)
							}
						#endif
					}
				}
			#endif
			switch (this->reorderBuffer[pos_buffer].uop.uop_operation){
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

			ERROR_ASSERT_PRINTF(uint32_t(pos_buffer) == this->robStart, "Commiting different from the position start\n");
			#if COMMIT_DEBUG
				if (orcs_engine.get_global_cycle() > WAIT_CYCLE)
				{
					ORCS_PRINTF("======================================\n")
					ORCS_PRINTF("RM ROB Entry \n%s\n", this->reorderBuffer[this->robStart].content_to_string().c_str())
				}
			#endif
			if(this->reorderBuffer[this->robStart].sent==true){
				if(this->reorderBuffer[this->robStart].uop.uop_operation==INSTRUCTION_OPERATION_MEM_LOAD){
					this->remove_front_mob_read();
				}
				else if(this->reorderBuffer[this->robStart].uop.uop_operation==INSTRUCTION_OPERATION_MEM_STORE){
					ERROR_ASSERT_PRINTF(this->counter_mshr_write > 0,"Erro, reduzindo requests paralelos abaixo de 0\n")
					this->counter_mshr_write--;
				}
			}
			this->removeFrontROB();
		}
		/// Could not commit the older, then stop looking for ready uops
		else
		{
			break;
		}
	}
		#if COMMIT_DEBUG
		if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
			ORCS_PRINTF("=========================================================================\n")
		}
	#endif

} //end method
// ============================================================================
void processor_t::solve_registers_dependency(reorder_buffer_line_t *rob_line){

		/// Remove pointers from Register Alias Table (RAT)
		for (uint32_t j = 0; j < MAX_REGISTERS; j++)
		{
			if (rob_line->uop.write_regs[j] < 0)
			{
				break;
			}
			uint32_t write_register = rob_line->uop.write_regs[j];
			ERROR_ASSERT_PRINTF(write_register < RAT_SIZE, "Read Register (%d) > Register Alias Table Size (%d)\n",
								write_register, RAT_SIZE);
			if (this->register_alias_table[write_register] != NULL &&
				this->register_alias_table[write_register]->uop.uop_number == rob_line->uop.uop_number)
			{
				this->register_alias_table[write_register] = NULL;
			} //end if
		}	 //end for

		// =========================================================================
		// SOLVE REGISTER DEPENDENCIES - RAT
		// =========================================================================
		for (uint32_t j = 0; j < ROB_SIZE; j++)
		{
			/// There is an unsolved dependency
			if (rob_line->reg_deps_ptr_array[j] != NULL)
			{
				rob_line->wake_up_elements_counter--;
				rob_line->reg_deps_ptr_array[j]->wait_reg_deps_number--;
				/// This update the ready cycle, and it is usefull to compute the time each instruction waits for the functional unit
				if (rob_line->reg_deps_ptr_array[j]->uop.readyAt <= orcs_engine.get_global_cycle())
				{
					rob_line->reg_deps_ptr_array[j]->uop.readyAt = orcs_engine.get_global_cycle();
				}
				rob_line->reg_deps_ptr_array[j] = NULL;
			}
			/// All the dependencies are solved
			else
			{
				break;
			}
		}
}
// ============================================================================
bool processor_t::verify_uop_on_emc(reorder_buffer_line_t *rob_line){
	uint16_t pos = orcs_engine.memory_controller->emc[this->processor_id].uop_buffer_start;
	uint16_t end = orcs_engine.memory_controller->emc[this->processor_id].uop_buffer_used;
	bool is_present = false;
	for (uint16_t i = 0; i < end; i++){
		if(rob_line->uop.uop_number == orcs_engine.memory_controller->emc[this->processor_id].uop_buffer[pos].uop.uop_number){
			is_present = true;
			break;
		}
		pos++;
		if(pos>=EMC_UOP_BUFFER)pos=0;
	}
	return is_present;
}
// ============================================================================
bool processor_t::isRobHead(reorder_buffer_line_t *rob_line){
	// ORCS_PRINTF("rob_line: %p , rob Head: %p\n",rob_line,&this->reorderBuffer[robStart])
	return (rob_line == &this->reorderBuffer[robStart]);
}
// =====================================================================
void processor_t::make_dependence_chain(reorder_buffer_line_t *rob_line){
	// ORCS_PRINTF("Miss Original %s\n", rob_line->content_to_string().c_str())
	ERROR_ASSERT_PRINTF(rob_line->uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD, "Error, making dependences from NON-LOAD operation\n%s\n", rob_line->content_to_string().c_str())
	// ORCS_PRINTF("Cycle %lu\n",orcs_engine.get_global_cycle())
	while(this->rob_buffer.size()<=EMC_UOP_BUFFER){
		int32_t next_position = this->get_next_uop_dependence();
		if(next_position==POSITION_FAIL){
			break;
		}else{
			reorder_buffer_line_t *next_operation = this->rob_buffer[next_position];
			#if EMC_ACTIVE_DEBUG
				if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
					ORCS_PRINTF("==========\n")
					ORCS_PRINTF("Getting deps %s\n",next_operation->content_to_string().c_str())
				}
			#endif	
			// if(next_operation->op_on_emc_buffer != next_operation->wait_reg_deps_number){
			// 	this->rob_buffer.erase(this->rob_buffer.begin()+next_position);
			// 	continue;
			// }	
			for(uint32_t i = 0; i < ROB_SIZE; i++)
			{
				if(next_operation->reg_deps_ptr_array[i]==NULL){
					break;
				}
				if(this->already_exists(next_operation->reg_deps_ptr_array[i])){
					continue;
				}
			#if !ALL_UOPS
				if(
				next_operation->reg_deps_ptr_array[i]->uop.uop_operation == INSTRUCTION_OPERATION_INT_ALU ||
				next_operation->reg_deps_ptr_array[i]->uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD //|| //){
				// next_operation->reg_deps_ptr_array[i]->uop.uop_operation == INSTRUCTION_OPERATION_MEM_STORE
				){
			#endif
					//verify memory ambiguation
					if(next_operation->reg_deps_ptr_array[i]->uop.uop_operation == INSTRUCTION_OPERATION_MEM_STORE || 
						next_operation->reg_deps_ptr_array[i]->uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD){
							if(this->verify_ambiguation(next_operation->reg_deps_ptr_array[i]->mob_ptr)){
								#if EMC_ACTIVE_DEBUG
									if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
									ORCS_PRINTF("\n\nTem_ambig %s\n\n",next_operation->reg_deps_ptr_array[i]->content_to_string().c_str())
								}
								#endif
								// cancel chain execution
								// neste ponto nenhuma das estruturas foram utilizadas ainda
								this->add_cancel_emc_execution();
								this->rob_buffer.clear();
								return;
							}
					}
					this->rob_buffer.push_back(next_operation->reg_deps_ptr_array[i]);
					#if EMC_ACTIVE_DEBUG
						if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
							ORCS_PRINTF("Adding %s\n",next_operation->reg_deps_ptr_array[i]->content_to_string().c_str())
						}
					#endif
					next_operation->reg_deps_ptr_array[i]->op_on_emc_buffer++;
				#if !ALL_UOPS
				}
				#endif
			}
			next_operation->on_chain=true;
			if(!next_operation->original_miss){
				next_operation->is_poisoned=true;
			}
		}
	}
	if(this->rob_buffer.size()<2){
			this->add_cancel_emc_execution_one_op();
			this->rob_buffer.clear();
			#if EMC_ACTIVE_DEBUG
				if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
					ORCS_PRINTF("Cancelado One Op\n")
				}
			#endif
		}else{
			// this->start_emc_module=true;
			#if EMC_ACTIVE_DEBUG
				if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
					ORCS_PRINTF("==========\n")
					ORCS_PRINTF("Chain_made\n")
					for (uint32_t i = 0; i < this->rob_buffer.size(); i++){
						ORCS_PRINTF("%d -> %s\n",i,this->rob_buffer[i]->content_to_string().c_str())
					}
					// INFO_PRINTF()	
					ORCS_PRINTF("==========\n")
				}
			#endif
			// this->add_started_emc_execution();
		}
}
// =====================================================================
// @return return the next element to be catch the dependences
int32_t processor_t::get_next_uop_dependence(){
	int32_t position = POSITION_FAIL;
	for (uint32_t i = 0; i < this->rob_buffer.size(); i++)
	{
		if(this->rob_buffer[i]->on_chain!=true){
			position = i;
			break;
		}
	}
	return position;
}
void processor_t::print_RRT(){
	for(uint32_t i =0;i<EMC_REGISTERS;i++){
		this->rrt[i].print_rrt_entry();
	}
}
void processor_t::print_ROB(){
	uint32_t pos = this->robStart;
	for(size_t i = 0; i < this->robUsed; i++)
	{
		ORCS_PRINTF("%s\n\n",this->reorderBuffer[pos].content_to_string().c_str())
		pos++;
		if(pos>=ROB_SIZE){
			pos=0;
		}
	}
	
}
// =============================================================================
int32_t processor_t::renameEMC(reorder_buffer_line_t *rob_line){
		// ===========================================================
		//Verificar se todos os registradores estao prontos ou no rrt
		// ===========================================================
		if(this->count_registers_rrt(rob_line->uop) < rob_line->wait_reg_deps_number){
			return NOT_ALL_REGS;
		}

		// ===========================================================
		// get position on emc lsq
		// ===========================================================
		memory_order_buffer_line_t *lsq;
		int32_t pos_lsq = memory_order_buffer_line_t::find_free(orcs_engine.memory_controller->emc[this->processor_id].unified_lsq, EMC_LSQ_SIZE);
		if(pos_lsq == POSITION_FAIL){
			return POSITION_FAIL;
		}
		lsq = &orcs_engine.memory_controller->emc[this->processor_id].unified_lsq[pos_lsq];
		// ===========================================================
		
		// ===========================================================
		// get position on uop buffer
		// ===========================================================
		int32_t pos_emc = orcs_engine.memory_controller->emc[this->processor_id].get_position_uop_buffer();
		if(pos_emc==POSITION_FAIL){
			return POSITION_FAIL;
		}
		emc_opcode_package_t *emc_package = &orcs_engine.memory_controller->emc[this->processor_id].uop_buffer[pos_emc];

		// ===========================================================
		// IF CAME HERE, CONGRATULATIONS. NEXT STEP
		// ===========================================================
		// Add the entry of uopbuffer on reservation station of EMC
		orcs_engine.memory_controller->emc[this->processor_id].unified_rs.push_back(emc_package);
		// Fill EMC package with info
		emc_package->package_clean();		//clean package 
		emc_package->uop = rob_line->uop;	//copy uop to info operation
		emc_package->rob_ptr = rob_line;	//pointer to rob entry to return
		if (rob_line->mob_ptr != NULL){		//is memory uop
			orcs_engine.memory_controller->emc[this->processor_id].unified_lsq[pos_lsq] = *(rob_line->mob_ptr);	//copy infos of memory access
			if(rob_line->original_miss){ //if original memory miss, reduce latency
				emc_package->uop.readyAt = emc_package->uop.readyAt-(L1_DATA_LATENCY+L2_LATENCY+LLC_LATENCY);
				lsq->readyAt=lsq->readyAt-(L1_DATA_LATENCY+L2_LATENCY+LLC_LATENCY);
			}else{
				lsq->status = PACKAGE_STATE_WAIT;
				lsq->uop_executed = false;
				lsq->processed = false;
			}
			if(lsq->wait_mem_deps_number>0){
				lsq->wait_mem_deps_number=0;
				rob_line->mob_ptr->wait_mem_deps_number=0;
				}
			// Linking EMC Uop Buffer to EMC LSQ
			emc_package->mob_ptr = lsq;			//Setting buffer to lsq
			lsq->emc_opcode_ptr = emc_package;	//setting lsq to buffer
		}
		// ===========================================================
		/// Control the Register Dependency - Register READ
		for (uint32_t k = 0; k < MAX_REGISTERS; k++){
			if (emc_package->uop.read_regs[k] < 0)
			{
				break;
			}
			int32_t read_register = this->search_register(rob_line->uop.read_regs[k]);
			// ORCS_PRINTF("Read register %d - %d\n",rob_line->uop.read_regs[k],read_register)
			if (read_register == POSITION_FAIL)
				continue;
			/// If there is a dependency
			if (this->rrt[read_register].entry != NULL)
			{
				for (uint32_t j = 0; j < ROB_SIZE; j++)
				{
					if (this->rrt[read_register].entry->reg_deps_ptr_array[j] == NULL)
					{
						this->rrt[read_register].entry->wake_up_elements_counter++;
						this->rrt[read_register].entry->reg_deps_ptr_array[j] = emc_package;
						emc_package->wait_reg_deps_number++;
						break;
					}
				}
			}
		}

		/// Control the Register Dependency - Register WRITE
		for (uint32_t k = 0; k < MAX_REGISTERS; k++){
			// this->add_registerWrite();
			if (rob_line->uop.write_regs[k] < 0)
			{
				break;
			}
			int32_t write_register = this->search_register(rob_line->uop.write_regs[k]);

			if (write_register == POSITION_FAIL)
			{
				// ORCS_PRINTF("Write register Fail %d - %d\n",rob_line->uop.write_regs[k],write_register)
				write_register = this->allocate_new_register(rob_line->uop.write_regs[k]);
			}
			// ORCS_PRINTF("Write register %d - %d\n",rob_line->uop.write_regs[k],write_register)
			this->rrt[write_register].entry = emc_package;
		}
		#if EMC_ACTIVE_DEBUG
			if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
				ORCS_PRINTF("After Renamed Uop its\n %s\n\n",emc_package->content_to_string().c_str())
			}
		#endif
		return OK;
}
// =======================================================================
/*
 @1 write register to be searched in rrt
 @return position of register in rrt
 */
int32_t processor_t::search_register(int32_t write_register){
	int32_t reg_pos = POSITION_FAIL;
	for (int32_t i = 0; i < EMC_REGISTERS; i++)
	{
		if (this->rrt[i].register_core == write_register)
		{
			reg_pos = i;
			return reg_pos;
		}
	}
	return reg_pos;
}
// =======================================================================
/*
 @1 write register to be allocated in rrt
 @return position of register allocated in rrt
 */
int32_t processor_t::allocate_new_register(int32_t write_register){
	int32_t reg_pos = POSITION_FAIL;
	for (int32_t i = 0; i < EMC_REGISTERS; i++)
	{
		// ORCS_PRINTF("Register_Core %d\n",this->rrt[i].register_core)
		if (this->rrt[i].register_core == POSITION_FAIL)
		{
			reg_pos = i;
			this->rrt[i].register_core = write_register;
			break;
		}
	}
	return reg_pos;
}
// ============================================================================
/*
Clean register remapping table
*/
void processor_t::clean_rrt(){
	for (size_t i = 0; i < EMC_REGISTERS; i++)
	{
		this->rrt[i].package_clean();
	}
}
// ============================================================================
void processor_t::verify_loads_missed(){
	for(uint8_t i = 1; i < this->rob_buffer.size();i++){
		if(this->rob_buffer[i]->uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD){
			this->add_loads_missed_counter();
		}
	}
}
// ============================================================================
void processor_t::verify_started_emc_without_loads(){	

	if(this->rob_buffer.size()>2){
		for(uint8_t i = 1; i < this->rob_buffer.size();i++){
			if(this->rob_buffer[i]->uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD){
				return;
			}
		}
	this->add_started_emc_without_loads();
	}
}
// ============================================================================
/*
Verify register spill to include stores on chain;
@1 rob entry to compare address
*/
bool processor_t::verify_spill_register(reorder_buffer_line_t *rob_line){
	bool spill = false;
	for (size_t i = 0; i < this->rob_buffer.size(); i++){
		if (this->rob_buffer[i]->uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD && 
			this->rob_buffer[i]->mob_ptr->memory_address == rob_line->mob_ptr->memory_address &&
			this->rob_buffer[i]->mob_ptr->memory_size == rob_line->mob_ptr->memory_size &&
			this->rob_buffer[i]->uop.uop_number > rob_line->uop.uop_number){
			spill = true;
			break;
		}
	}
	return spill;
}
// ============================================================================
// @return true if no loads operation is gt 1
bool processor_t::verify_dependent_loads(){
	uint8_t loads = 1;
	this->instrucoes_inter_load_deps=0;
		// ORCS_PRINTF("\n\n%s\n",this->rob_buffer[0]->content_to_string().c_str())
	for (uint32_t i = 1; i < this->rob_buffer.size(); i++){
		// ORCS_PRINTF("%s\n",this->rob_buffer[i]->content_to_string().c_str())
		if(this->rob_buffer[i]->uop.uop_operation != INSTRUCTION_OPERATION_MEM_LOAD){
			this->instrucoes_inter_load_deps++;
			// ORCS_PRINTF("InterLoads %u\n",this->instrucoes_inter_load_deps)
		}else{
			loads++;
			this->numero_load_deps++;
			this->soma_instrucoes_deps+=this->instrucoes_inter_load_deps;
			this->instrucoes_inter_load_deps=0;
			// ORCS_PRINTF("\nLOADS %u\nSoma_Inst %u\n",this->numero_load_deps,this->soma_instrucoes_deps)
		}
	}
	// this->start_emc_module=false;
	// this->rob_buffer.clear();
	return (loads > 1) ? true : false; //if compacto
}
// ============================================================================
// @1 receive uop to verify if registers wait is on RRT
// @return number of registers on RRT
uint32_t processor_t::count_registers_rrt(uop_package_t uop){
	uint32_t registers_on_rrt=0;
	for (uint8_t i = 0; i < MAX_REGISTERS ; i++){
		if(uop.read_regs[i]<0){
			break;
		}
		for (uint8_t j= 0; j < EMC_REGISTERS; j++){
			if(uop.read_regs[i] == this->rrt[j].register_core){
				registers_on_rrt++;
				break;
			}
		}
	}
	return registers_on_rrt;
}
// ============================================================================
void processor_t::update_counter_emc(int32_t value){
	this->counter_activate_emc+=value;
	if(this->counter_activate_emc>7){
		this->counter_activate_emc=7;	
	}else if(this->counter_activate_emc<0){
		this->counter_activate_emc=0;
	}
}
// ============================================================================
void processor_t::cancel_execution_emc(){
	for (uint8_t i = 0; i < EMC_UOP_BUFFER; i++)
	{

		if (orcs_engine.memory_controller->emc[this->processor_id].uop_buffer[i].rob_ptr != NULL)
		{
			orcs_engine.memory_controller->emc[this->processor_id].uop_buffer[i].rob_ptr->on_chain = false;
		}
		if (orcs_engine.memory_controller->emc[this->processor_id].uop_buffer[i].rob_ptr != NULL)
		{
			orcs_engine.memory_controller->emc[this->processor_id].uop_buffer[i].rob_ptr->is_poisoned = false;
		}
			orcs_engine.memory_controller->emc[this->processor_id].unified_lsq[i].package_clean();
			orcs_engine.memory_controller->emc[this->processor_id].uop_buffer[i].package_clean();

	}
	//zerando controles
	orcs_engine.memory_controller->emc[this->processor_id].uop_buffer_end = 0;
	orcs_engine.memory_controller->emc[this->processor_id].uop_buffer_start= 0;
	orcs_engine.memory_controller->emc[this->processor_id].uop_buffer_used = 0;
	//pointers containers RS e Unified FUS
	orcs_engine.memory_controller->emc[this->processor_id].unified_fus.clear();
	orcs_engine.memory_controller->emc[this->processor_id].unified_rs.clear();

	//conferindo estruturas -> comentar futuramente
	// orcs_engine.memory_controller->emc[this->processor_id].print_structures();
}
// ============================================================================
bool processor_t::already_exists(reorder_buffer_line_t *candidate){
	for (uint32_t i = 0; i <this->rob_buffer.size(); i++)
	{
		if (this->rob_buffer[i]->uop.uop_number == candidate->uop.uop_number)
		{
			// ORCS_PRINTF("Opcode ja existente %s\n",candidate->content_to_string().c_str())
			return true;
		}
	}
	return false;
}
// ============================================================================
bool processor_t::verify_ambiguation(memory_order_buffer_line_t *mob_line){
	uint32_t pos = this->memory_order_buffer_write_start;
	for(uint32_t i = 0; i < this->memory_order_buffer_write_used; i++){
		if(this->memory_order_buffer_write[pos].memory_address == mob_line->memory_address &&
			this->memory_order_buffer_write[pos].uop_number < mob_line->uop_number){
			this->add_counter_ambiguation_write();
			return true;
		}
	
		pos++;
		if(pos>=MOB_WRITE)
			pos=0;
	}
	return false;
}

// ============================================================================
void processor_t::statistics(){
	bool close = false;
	FILE *output = stdout;
	if(orcs_engine.output_file_name != NULL){
		output = fopen(orcs_engine.output_file_name,"a+");
		close=true;
	}
	if (output != NULL){
		utils_t::largestSeparator(output);
		fprintf(output, "Total_Cycle: %lu\n", this->get_ended_cycle());
		utils_t::largeSeparator(output);
		fprintf(output, "Stage_Opcode_and_Uop_Counters\n");
		utils_t::largeSeparator(output);
		fprintf(output, "Stage_Fetch: %lu\n", this->fetchCounter);
		fprintf(output, "Stage_Decode: %lu\n", this->decodeCounter);
		fprintf(output, "Stage_Rename: %lu\n", this->renameCounter);
		fprintf(output, "Stage_Commit: %lu\n", this->commit_uop_counter);
		utils_t::largestSeparator(output);
			#if MAX_PARALLEL_REQUESTS_CORE
				fprintf(output, "Times_Reach_MAX_PARALLEL_REQUESTS_CORE_READ: %lu\n", this->get_times_reach_parallel_requests_read());
				fprintf(output, "Times_Reach_MAX_PARALLEL_REQUESTS_CORE_WRITE: %lu\n", this->get_times_reach_parallel_requests_write());
				fprintf(output, "Loads_sent_at_ROB_HEAD: %u\n", this->get_loads_sent_at_rob_head());

			#endif
		utils_t::largestSeparator(output);
		fprintf(output, "Instruction_Per_Cycle_After_Warmup: %1.6lf\n", (float)this->fetchCounter/this->get_ended_cycle());	
		fprintf(output, "MPKI: %lf\n", (float)orcs_engine.cacheManager->LLC_data_cache[orcs_engine.cacheManager->generate_index_array(this->processor_id,LLC)].get_cacheMiss()/((float)this->fetchCounter/1000));
		utils_t::largestSeparator(output);
			#if EMC_ACTIVE
				fprintf(output, "\n======================== EMC INFOS ===========================\n");
				utils_t::largeSeparator(output);
				fprintf(output, "times_llc_rob_head: %u\n", this->get_llc_miss_rob_head());
				fprintf(output, "numero_load_deps: %u\n", this->numero_load_deps);
				fprintf(output, "Total_instrucoes_dependentes: %u\n", this->soma_instrucoes_deps);
				fprintf(output, "load_deps_ratio: %.4f\n", float(this->soma_instrucoes_deps) / float(this->numero_load_deps));
				fprintf(output, "total_instruction_sent_emc: %d\n", this->get_total_instruction_sent_emc());
				fprintf(output, "avg_inst_sent_emc: %.4f\n",static_cast<double> (this->get_total_instruction_sent_emc())/static_cast<double> (this->get_started_emc_execution()-this->get_cancel_emc_execution_one_op()));
				utils_t::smallSeparator(output);
				fprintf(output, "started_emc_execution: %d\n", this->get_started_emc_execution());
				fprintf(output, "canceled_counter_emc_execution: %d\n", this->get_cancel_counter_emc_execution());
				fprintf(output, "canceled_ambiguation_emc_execution: %d\n", this->get_cancel_emc_execution());
				fprintf(output, "canceled_emc_execution_one_op: %d\n", this->get_cancel_emc_execution_one_op());
				fprintf(output, "total_ambiguation_read: %d\n", this->get_counter_ambiguation_read());
				fprintf(output, "total_ambiguation_write: %d\n", this->get_counter_ambiguation_write());
				utils_t::smallSeparator(output);
				fprintf(output, "loads_missed_counter: %d\n", this->get_loads_missed_counter());
				fprintf(output, "started_emc_without_loads: %d\n", this->get_started_emc_without_loads());
				utils_t::smallSeparator(output);
			#endif
			}
		if(close) fclose(output);
		this->desambiguator->statistics();
}
// ============================================================================
void processor_t::printConfiguration(){
	FILE *output = fopen(orcs_engine.output_file_name, "a+");
	if (output != NULL)
	{
		fprintf(output, "===============Stages Width============\n");
		fprintf(output, "FETCH Width %d\n", FETCH_WIDTH);
		fprintf(output, "DECODE Width %d\n", DECODE_WIDTH);
		fprintf(output, "RENAME Width %d\n", RENAME_WIDTH);
		fprintf(output, "DISPATCH Width %d\n", DISPATCH_WIDTH);
		fprintf(output, "EXECUTE Width %d\n", EXECUTE_WIDTH);
		fprintf(output, "COMMIT Width %d\n", COMMIT_WIDTH);

		fprintf(output, "===============Structures Sizes============\n");
		fprintf(output, "Fetch Buffer ->%u\n", FETCH_BUFFER);
		fprintf(output, "Decode Buffer ->%u\n", DECODE_BUFFER);
		fprintf(output, "RAT ->%u\n", RAT_SIZE);
		fprintf(output, "ROB ->%u\n", ROB_SIZE);
		fprintf(output, "MOB Read ->%u\n", MOB_READ);
		fprintf(output, "MOB Write->%u\n", MOB_WRITE);
		fprintf(output, "Reservation Station->%u\n", UNIFIED_RS);
		fprintf(output, "===============Memory Configuration============\n");
		fprintf(output, "===============Instruction $============\n");
		fprintf(output, "L1_INST_SIZE ->%u\n", L1_INST_SIZE);
		fprintf(output, "L1_INST_ASSOCIATIVITY ->%u\n", L1_INST_ASSOCIATIVITY);
		fprintf(output, "L1_INST_LATENCY ->%u\n", L1_INST_LATENCY);
		fprintf(output, "L1_INST_SETS ->%u\n", L1_INST_SETS);
		fprintf(output, "===============Data $ L1============\n");
		fprintf(output, "L1_DATA_SIZE ->%u\n", L1_DATA_SIZE);
		fprintf(output, "L1_DATA_ASSOCIATIVITY ->%u\n", L1_DATA_ASSOCIATIVITY);
		fprintf(output, "L1_DATA_LATENCY ->%u\n", L1_DATA_LATENCY);
		fprintf(output, "L1_DATA_SETS ->%u\n", L1_DATA_SETS);
		fprintf(output, "===============LLC ============\n");
		fprintf(output, "LLC_SIZE ->%u\n", LLC_SIZE);
		fprintf(output, "LLC_ASSOCIATIVITY ->%u\n", LLC_ASSOCIATIVITY);
		fprintf(output, "LLC_LATENCY ->%u\n", LLC_LATENCY);
		fprintf(output, "LLC_SETS ->%u\n", LLC_SETS);
		fprintf(output, "=============== PREFETCHER ============\n");
		fprintf(output, "PREFETCHER_ACTIVE ->%u\n", PREFETCHER_ACTIVE);

		fprintf(output, "===============RAM ============\n");
		fprintf(output, "RAM_LATENCY ->%u\n", RAM_LATENCY);
		fprintf(output, "=============== Limits ============\n");
		fprintf(output, "PARALLEL_LIM_ACTIVE ->%u\n", PARALLEL_LIM_ACTIVE);
		fprintf(output, "MAX_PARALLEL_REQUESTS_CORE ->%u\n", MAX_PARALLEL_REQUESTS_CORE);
	}
}
// ============================================================================
void processor_t::printStructures(){
	ORCS_PRINTF("Periodic Check -  Structures at %lu\n", orcs_engine.get_global_cycle())
	ORCS_PRINTF("Fetched Opcodes %lu of %lu\n", orcs_engine.trace_reader[this->processor_id].get_fetch_instructions(), orcs_engine.trace_reader[this->processor_id].get_trace_opcode_max())
	utils_t::largestSeparator();
	ORCS_PRINTF("Front end Buffers\n")
	utils_t::largeSeparator();
	ORCS_PRINTF("Fetch Buffer ==> %u\n", this->fetchBuffer.get_size())
	utils_t::smallSeparator();
	ORCS_PRINTF("DecodeBuffer ==> %u\n", this->decodeBuffer.get_size())
	utils_t::largestSeparator();
	ORCS_PRINTF("ROB and MOB usage\n")
	ORCS_PRINTF("ROB used %u of %u \n", this->robUsed, ROB_SIZE)
	utils_t::largeSeparator();
	ORCS_PRINTF("Counter MSHR Read %d\n", this->counter_mshr_read)
	ORCS_PRINTF("Counter MSHR Write %d\n", this->counter_mshr_write)
	// ORCS_PRINTF("Dispatch and execute usage\n")
	// utils_t::smallSeparator();
	// ORCS_PRINTF("Dispatch Use: %lu\n",this->unified_reservation_station.size())
	utils_t::largestSeparator();
	ORCS_PRINTF("Completed Operations %lu\n", this->commit_uop_counter)
	// utils_t::smallSeparator();
	// ORCS_PRINTF("FUs Use: %lu\n",this->unified_functional_units.size())
	// utils_t::largestSeparator();
	// ORCS_PRINTF("Commit Status uOPs\n")
	// utils_t::largeSeparator();
	// // ==============================================================
	// ORCS_PRINTF("INSTRUCTION_OPERATION_INT_ALU %lu\n",this->get_stat_inst_int_alu_completed())
	// utils_t::largeSeparator();
	// ORCS_PRINTF("INSTRUCTION_OPERATION_INT_MUL %lu\n",this->get_stat_inst_mul_alu_completed())
	// utils_t::largeSeparator();
	// ORCS_PRINTF("INSTRUCTION_OPERATION_INT_DIV %lu\n",this->get_stat_inst_div_alu_completed())
	// utils_t::largeSeparator();
	// ORCS_PRINTF("INSTRUCTION_OPERATION_FP_ALU %lu\n",this->get_stat_inst_int_fp_completed())
	// utils_t::largeSeparator();
	// ORCS_PRINTF("INSTRUCTION_OPERATION_FP_MUL %lu\n",this->get_stat_inst_mul_fp_completed())
	// utils_t::largeSeparator();
	// ORCS_PRINTF("INSTRUCTION_OPERATION_FP_DIV %lu\n",this->get_stat_inst_div_fp_completed())
	// utils_t::largeSeparator();
	// ORCS_PRINTF("INSTRUCTION_OPERATION_MEM_LOAD %lu\n",this->get_stat_inst_load_completed())
	// utils_t::largeSeparator();
	// ORCS_PRINTF("INSTRUCTION_OPERATION_MEM_STORE %lu\n",this->get_stat_inst_store_completed())
	// utils_t::largeSeparator();
	// ORCS_PRINTF("INSTRUCTION_OPERATION_BRANCH %lu\n",this->get_stat_inst_branch_completed())
	// utils_t::largeSeparator();
	// ORCS_PRINTF("INSTRUCTION_OPERATION_NOP %lu\n",this->get_stat_inst_nop_completed())
	// utils_t::largeSeparator();
	// ORCS_PRINTF("INSTRUCTION_OPERATION_OTHER %lu\n",this->get_stat_inst_other_completed())
	// utils_t::largeSeparator();
	// ORCS_PRINTF("\n\n\n")
	// ==============================================================
	sleep(2);
}
// ============================================================================
void processor_t::clock(){
	#if DEBUG
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("============================PROCESSOR %u===============================\n",this->processor_id)
			ORCS_PRINTF("Cycle %lu\n",orcs_engine.get_global_cycle())
		}
	#endif
	/////////////////////////////////////////////////
	//// Verifica se existe coisas no ROB
	//// CommitStage
	//// ExecuteStage
	//// DispatchStage
	/////////////////////////////////////////////////
		if (this->robUsed != 0)
		{
			this->commit();   //commit instructions -> remove from ROB
			this->execute();  //verify Uops ready on UFs, then remove
			this->dispatch(); //dispath ready uops to UFs
		}
		/////////////////////////////////////////////////
		//// Verifica se existe coisas no DecodeBuffer
		//// Rename
		/////////////////////////////////////////////////
		if (!this->decodeBuffer.is_empty())
		{
			this->rename();
		}
	/////////////////////////////////////////////////
	//// Verifica se existe coisas no FetchBuffer
	//// Decode
	/////////////////////////////////////////////////
	if (!this->fetchBuffer.is_empty())
	{
		this->decode();
	}
	/////////////////////////////////////////////////
	//// Verifica se trace is over
	//// Fetch
	/////////////////////////////////////////////////
	if ((!this->traceIsOver))
	{
		this->fetch();
	}

	if (!this->isBusy())
	{
		if(!this->snapshoted){
			this->set_ended_cycle(orcs_engine.get_global_cycle());
			this->snapshoted=true;
		}
	}
	#if DEBUG
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("===================================================================\n")
			// sleep(1);
		}
	#endif
}
