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
};
processor_t::~processor_t()
{
	//Memory
	utils_t::template_delete_array<memory_order_buffer_line_t>(this->memory_order_buffer_read);
	utils_t::template_delete_array<memory_order_buffer_line_t>(this->memory_order_buffer_write);
	utils_t::template_delete_variable<desambiguation_t>(this->desambiguator);

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
};
// =====================================================================
void processor_t::allocate()
{
	//======================================================================
	// Initializating variables
	//======================================================================
	this->traceIsOver = false;
	this->hasBranch = false;
	this->insertError = false;
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
	// // Memory Order Buffer Read
	this->memory_order_buffer_read = utils_t::template_allocate_array<memory_order_buffer_line_t>(MOB_READ);
	for (size_t i = 0; i < MOB_READ; i++)
	{
		this->memory_order_buffer_read[i].mem_deps_ptr_array = utils_t::template_allocate_initialize_array<memory_order_buffer_line_t *>(ROB_SIZE, NULL);
	}
	// LOAD
	this->memory_order_buffer_read_start = 0;
	this->memory_order_buffer_read_end = 0;
	this->memory_order_buffer_read_used =0;
	// // Memory Order Buffer Write
	this->memory_order_buffer_write = utils_t::template_allocate_array<memory_order_buffer_line_t>(MOB_WRITE);
	for (size_t i = 0; i < MOB_WRITE; i++)
	{
		this->memory_order_buffer_write[i].mem_deps_ptr_array = utils_t::template_allocate_initialize_array<memory_order_buffer_line_t *>(ROB_SIZE, NULL);
	}
	// STORE
	this->memory_order_buffer_write_start = 0;
	this->memory_order_buffer_write_end = 0;
	this->memory_order_buffer_write_used =0;
	//desambiguator
	this->desambiguator = new desambiguation_t;
	this->desambiguator->allocate();
	// parallel requests
	
	// =========================================================================================
	//DRAM
	// =========================================================================================
	this->parallel_requests = 0;
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
	this->instrucoes_inter_load_deps = 0;
	this->soma_instrucoes_deps = 0;
	this->numero_load_deps = 0;
	this->on_emc_execution = false;
	// =====================================================================
};
// =====================================================================
bool processor_t::isBusy()
{
	return (this->traceIsOver == false ||
			!this->fetchBuffer.is_empty() ||
			!this->decodeBuffer.is_empty() ||
			this->robUsed != 0);
};

// ======================================
// Require a position to insert on ROB
// The Reorder Buffer behavior is a Circular FIFO
// @return position to insert
// ======================================
int32_t processor_t::searchPositionROB()
{
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
};
// ======================================
// Remove the Head of the reorder buffer
// The Reorder Buffer behavior is a Circular FIFO
// ======================================
void processor_t::removeFrontROB()
{
	#if COMMIT_DEBUG
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("Cycle  %lu\n", orcs_engine.get_global_cycle())
			ORCS_PRINTF("Trying remove \n%s\n", this->reorderBuffer[this->robStart].content_to_string().c_str())
		}
	#endif
	ERROR_ASSERT_PRINTF(this->robUsed > 0, "Removendo do ROB sem estar usado\n")
	ERROR_ASSERT_PRINTF(this->reorderBuffer[this->robStart].reg_deps_ptr_array[0] == NULL, "Removendo sem resolver dependencias\n%s\n",this->reorderBuffer[this->robStart].content_to_string().c_str())
	this->reorderBuffer[this->robStart].package_clean();
	this->robUsed--;
	this->robStart++;
	if (this->robStart >= ROB_SIZE)
	{
		this->robStart = 0;
	}
};
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
};
// ============================================================================
// remove front mob read on commit
// ============================================================================
void processor_t::remove_front_mob_read(){
	#if COMMIT_DEBUG
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("Cycle  %lu\n", orcs_engine.get_global_cycle())
			ORCS_PRINTF("Trying remove \n%s\n", this->memory_order_buffer_read[this->memory_order_buffer_read_start].content_to_string().c_str())
	}
	#endif
	ERROR_ASSERT_PRINTF(this->memory_order_buffer_read_used > 0, "Removendo do MOB_READ sem estar usado\n")
	ERROR_ASSERT_PRINTF(this->memory_order_buffer_read[this->memory_order_buffer_read_start].mem_deps_ptr_array[0] == NULL, "Removendo sem resolver dependencias\n%s\n",this->memory_order_buffer_read[this->memory_order_buffer_read_start].content_to_string().c_str())
	this->memory_order_buffer_read[this->memory_order_buffer_read_start].package_clean();
	this->memory_order_buffer_read_used--;
	this->memory_order_buffer_read_start++;
	if (this->memory_order_buffer_read_start >= MOB_READ)
	{
		this->memory_order_buffer_read_start = 0;
	}
};
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
};
// ============================================================================
// remove front mob read on commit
// ============================================================================
void processor_t::remove_front_mob_write(){
	#if COMMIT_DEBUG
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("Cycle  %lu\n", orcs_engine.get_global_cycle())
			ORCS_PRINTF("Trying remove \n%s\n", this->memory_order_buffer_write[this->memory_order_buffer_write_start].content_to_string().c_str())
	}
	#endif
	ERROR_ASSERT_PRINTF(this->memory_order_buffer_write_used > 0, "Removendo do MOB_WRITE sem estar usado\n")
	ERROR_ASSERT_PRINTF(this->memory_order_buffer_write[this->memory_order_buffer_write_start].mem_deps_ptr_array[0] == NULL, "Removendo sem resolver dependencias\n%s\n",this->memory_order_buffer_write[this->memory_order_buffer_write_start].content_to_string().c_str())
	this->memory_order_buffer_write[this->memory_order_buffer_write_start].package_clean();
	this->memory_order_buffer_write_used--;
	this->memory_order_buffer_write_start++;
	if (this->memory_order_buffer_write_start >= MOB_WRITE)
	{
		this->memory_order_buffer_write_start = 0;
	}
};
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
		if (!orcs_engine.trace_reader->trace_fetch(&operation))
		{
			this->traceIsOver = true;
			break;
		}
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
			uint32_t stallWrongBranch = orcs_engine.branchPredictor->solveBranch(this->previousBranch, operation);
			this->set_stall_wrong_branch(orcs_engine.get_global_cycle() + stallWrongBranch);
			this->hasBranch = false;
			uint32_t ttc = orcs_engine.cacheManager->searchInstruction(operation.opcode_address);
			// ORCS_PRINTF("ready after wrong branch %lu\n",this->get_stall_wrong_branch()+ttc)
			operation.updatePackageReady(stallWrongBranch + ttc);
			updated = true;
			this->previousBranch.package_clean();
			// ORCS_PRINTF("Stall Wrong Branch %u\n",stallWrongBranch)
		}
		//============================
		// Operation Branch, set flag
		//============================
		if (operation.opcode_operation == INSTRUCTION_OPERATION_BRANCH)
		{
			orcs_engine.branchPredictor->branches++;
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
			uint32_t ttc = orcs_engine.cacheManager->searchInstruction(operation.opcode_address);
	#if FETCH_DEBUG
				ORCS_PRINTF("(%lu) (%lu) TTC %u\n", orcs_engine.get_global_cycle(), operation.opcode_address, ttc)
				ORCS_PRINTF("(%lu) (%lu) readyAt After  %lu\n", orcs_engine.get_global_cycle(), operation.readyAt, (operation.readyAt + ttc))
				sleep(1);
	#endif
			this->fetchBuffer.back()->updatePackageReady(FETCH_LATENCY + ttc);
		}
	}
};
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
	#if DECODE_DEBUG
			ORCS_PRINTF("Opcode to decode %s\n", this->fetchBuffer.front()->content_to_string().c_str())
	#endif
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
};

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
};
// ============================================================================
void processor_t::rename()
{
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
			pos_mob = memory_order_buffer_line_t::find_free(this->memory_order_buffer_read, MOB_READ);
			// pos_mob = this->search_position_mob_read();
			if (pos_mob == POSITION_FAIL)
			{
				this->add_stall_full_MOB_Read();
				break;
			}
			mob_line = &this->memory_order_buffer_read[pos_mob];
		}
		//=======================
		// Memory Operation Write
		//=======================
		if (this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_MEM_STORE)
		{
			pos_mob = memory_order_buffer_line_t::find_free(this->memory_order_buffer_write, MOB_WRITE);
			// pos_mob = this->search_position_mob_write();
			if (pos_mob == POSITION_FAIL)
			{
				this->add_stall_full_MOB_Write();
				break;
			}
			mob_line = &this->memory_order_buffer_write[pos_mob];
		}
		//=======================
		// Verificando se tem espaco no ROB se sim bamos inserir
		//=======================
		pos_rob = this->searchPositionROB();
		if (pos_rob == POSITION_FAIL)
		{
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
		}
		//linking rob and mob
		if (this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD ||
			this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_MEM_STORE)
		{
			mob_line->rob_ptr = &this->reorderBuffer[pos_rob];
			mob_line->package_age = orcs_engine.get_global_cycle();
	#if DESAMBIGUATION_ENABLED
				this->desambiguator->make_memory_dependences(this->reorderBuffer[pos_rob].mob_ptr);
	#endif
		}
	} //end for
}
// ============================================================================
void processor_t::dispatch(){
	#if DISPATCH_DEBUG
		ORCS_PRINTF("Dispatch Stage\n")
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

		// ORCS_PRINTF("Size %lu\n", this->unified_reservation_station.size())
		// for
		for (uint32_t i = 0; i < this->unified_reservation_station.size() && i < UNIFIED_RS; i++)
		{
			//pointer to entry
			reorder_buffer_line_t *rob_line = this->unified_reservation_station[i];
	#if DISPATCH_DEBUG
			ORCS_PRINTF("cycle %lu\n", orcs_engine.get_global_cycle())
			ORCS_PRINTF("i = %u UNified RS %lu\n", i, this->unified_reservation_station.size())
			ORCS_PRINTF("Trying Dispatch %s\n", rob_line->content_to_string().c_str())
	#endif
			if (rob_line->is_poisoned == true)
			{
				this->unified_reservation_station.erase(this->unified_reservation_station.begin() + i);
				i--;
				continue;
			}
			if (total_dispatched >= DISPATCH_WIDTH)
			{
				break;
			}
			
			if ((rob_line->uop.readyAt <= orcs_engine.get_global_cycle()) &&
				(rob_line->wait_reg_deps_number == 0) &&
				(rob_line->is_poisoned == false))
			{
				ERROR_ASSERT_PRINTF(rob_line->uop.status == PACKAGE_STATE_READY, "Error, uop not ready being dispatched\n")
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
					ORCS_PRINTF("Dispatched %s\n", rob_line->content_to_string().c_str())
					ORCS_PRINTF("===================================================================\n")
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
		ORCS_PRINTF("Execute Stage\n")
	#endif
	// ==================================
	// verificar leituras prontas no ciclo,
	// remover do MOB e atualizar os registradores,
	// ==================================
	for (uint8_t i = 0; i < MOB_READ; i++){
		if (this->memory_order_buffer_read[i].status == PACKAGE_STATE_READY &&
			this->memory_order_buffer_read[i].readyAt <= orcs_engine.get_global_cycle()){
			ERROR_ASSERT_PRINTF(this->memory_order_buffer_read[i].uop_executed == true, "Removing memory read before being executed.\n")
			ERROR_ASSERT_PRINTF(this->memory_order_buffer_read[i].wait_mem_deps_number <= 0, "Number of memory dependencies should be zero.\n %s\n",this->memory_order_buffer_read[i].rob_ptr->content_to_string().c_str())
			this->memory_order_buffer_read[i].rob_ptr->stage = PROCESSOR_STAGE_COMMIT;
			this->memory_order_buffer_read[i].rob_ptr->uop.updatePackageReady(COMMIT_LATENCY);
			this->memory_order_buffer_read[i].rob_ptr->mob_ptr = NULL;
#if EXECUTE_DEBUG
			ORCS_PRINTF("Solving %s\n", this->memory_order_buffer_read[i].rob_ptr->content_to_string().c_str())
#endif
			// solving register dependence
			this->solve_registers_dependency(this->memory_order_buffer_read[i].rob_ptr);
// solving memory dependency
#if DESAMBIGUATION_ENABLED
			this->desambiguator->solve_memory_dependences(&this->memory_order_buffer_read[i]);
#endif
			this->memory_order_buffer_read[i].package_clean(); //limpa package
#if PARALLEL_LIM_ACTIVE
			(this->parallel_requests <= 0) ? (this->parallel_requests = 0) : (this->parallel_requests--);
#endif
			//controlar aguardo paralelos
			break;
		}
	}
#if EMC_ACTIVE
	if (this->start_emc_module)
	{	
	// ======================================================================
	// Contando numero de loads dependentes nas cadeias elegiveis para execução
		this->instrucoes_inter_load_deps = 0;
		for (uint32_t i = 1; i < this->rob_buffer.size(); i++)
		{
			if(this->rob_buffer[i]->uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD){
				this->numero_load_deps++;
				this->instrucoes_inter_load_deps++;
				this->soma_instrucoes_deps+=this->instrucoes_inter_load_deps;
				this->instrucoes_inter_load_deps=0;
			}else{
				this->instrucoes_inter_load_deps++;
			}
		}
	// ======================================================================
		for(uint32_t i=0;i<this->rob_buffer.size();i++){
			bool renamed_emc=false;
			reorder_buffer_line_t *rob_next = this->rob_buffer.front();
			///verificando se é um reg spill
			// ORCS_PRINTF("\n\nRenaming %s\n",rob_next->content_to_string().c_str())
		
			if(rob_next->uop.uop_operation == INSTRUCTION_OPERATION_MEM_STORE){
				if(this->verify_spill_register(rob_next)){
					this->rob_buffer.erase(this->rob_buffer.begin());
					i--;
					continue;
				}
			}
			////////////////////////////////////////
			if(this->renameEMC(rob_next) == POSITION_FAIL){
				this->rob_buffer.clear();	
			}else{
				rob_next->is_poisoned=true;
				
				renamed_emc=true;
			}
			// orcs_engine.memory_controller->emc->print_structures();
			////////////////////////////////////////
			if(renamed_emc==true){
				this->rob_buffer.erase(this->rob_buffer.begin());
				i--;
			}
			// Verifica se o rob_buffer está vazio, ou uop buffer cheio
			if (orcs_engine.memory_controller->emc->uop_buffer_used >= EMC_UOP_BUFFER || this->rob_buffer.empty()){
				this->start_emc_module = false; // disable emc module CORE
				this->rob_buffer.clear();		// flush core buffer
				orcs_engine.memory_controller->emc->ready_to_execute = true; //execute emc
				orcs_engine.memory_controller->emc->executed = true; //print dep chain emc
				this->clean_rrt(); //Limpa RRT;
				break;
			}
		}
	}
#endif

	uint32_t uop_total_executed = 0;
	for (uint32_t i = 0; i < this->unified_functional_units.size(); i++)
	{
		reorder_buffer_line_t *rob_line = this->unified_functional_units[i];
		if (uop_total_executed == EXECUTE_WIDTH)
		{
			break;
		}
		if (rob_line == NULL)
		{
			break;
		}
		if (rob_line->uop.readyAt <= orcs_engine.get_global_cycle())
		{
#if EXECUTE_DEBUG
			ORCS_PRINTF("Trying Execute %s\n", rob_line->content_to_string().c_str())
#endif
			ERROR_ASSERT_PRINTF(rob_line->stage == PROCESSOR_STAGE_EXECUTION, "ROB not on execution state")
			ERROR_ASSERT_PRINTF(rob_line->uop.status == PACKAGE_STATE_READY, "FU with Package not in ready state")
			switch (rob_line->uop.uop_operation)
			{
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
				;
				rob_line->mob_ptr->uop_executed = true;
				/// Waits for the cache to receive the package
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
			ORCS_PRINTF("Executed %s\n", rob_line->content_to_string().c_str())
#endif
		} //end if ready package
	}	 //end for
		// =========================================================================
		// Verificar se foi executado alguma operação de leitura,
		//  e executar a mais antiga no MOB
		// =========================================================================
		this->mob_read();
	// ==================================
	// Executar o MOB Write, com a escrita mais antiga.
	// depois liberar e tratar as escrita prontas;
	// ==================================
		this->mob_write();
	// =====================================
// 	for (uint8_t i = 0; i < MOB_WRITE; i++)
// 	{
// 		if (this->memory_order_buffer_write[i].status == PACKAGE_STATE_READY &&
// 			this->memory_order_buffer_write[i].readyAt <= orcs_engine.get_global_cycle())
// 		{
// 			ERROR_ASSERT_PRINTF(this->memory_order_buffer_write[i].uop_executed == true, "Removing memory read before being executed.\n")
// 			ERROR_ASSERT_PRINTF(this->memory_order_buffer_write[i].wait_mem_deps_number <= 0, "Number of memory dependencies should be zero.\n")
// 			// ERROR_ASSERT_PRINTF
// 			this->memory_order_buffer_write[i].rob_ptr->stage = PROCESSOR_STAGE_COMMIT;
// 			this->memory_order_buffer_write[i].rob_ptr->uop.updatePackageReady(COMMIT_LATENCY);
// 			this->memory_order_buffer_write[i].rob_ptr->mob_ptr = NULL;
// #if EXECUTE_DEBUG
// 			ORCS_PRINTF("Solving %s\n", this->memory_order_buffer_write[entry].rob_ptr->content_to_string().c_str())
// #endif
// 			// solving register dependence
// 			this->solve_registers_dependency(this->memory_order_buffer_write[i].rob_ptr);
// // solving memory dependency
// #if DESAMBIGUATION_ENABLED
// 			this->desambiguator->solve_memory_dependences(&this->memory_order_buffer_write[i]);
// #endif
// #if PARALLEL_LIM_ACTIVE
// 			(this->parallel_requests <= 0) ? (this->parallel_requests = 0) : (this->parallel_requests--);
// #endif
// 			this->memory_order_buffer_write[i].package_clean();
// 		}
// 	}
} //end method
// ============================================================================
uint32_t processor_t::mob_read(){
	#if MOB_DEBUG
		ORCS_PRINTF("MOB Read")
	#endif

	int32_t position_mem = POSITION_FAIL;
	if(oldest_read_to_send == NULL){
		#if PARALLEL_LIM_ACTIVE
			if (this->parallel_requests >= MAX_PARALLEL_REQUESTS)
			{
				// ORCS_PRINTF("Parallel Requests %d > MAX",this->parallel_requests)
				this->add_times_reach_parallel_requests_read();
				return FAIL;
			}
		#endif
		position_mem = memory_order_buffer_line_t::find_old_request_state_ready(this->memory_order_buffer_read,MOB_READ, PACKAGE_STATE_WAIT);
		if (position_mem != POSITION_FAIL)
			{
				oldest_read_to_send = &this->memory_order_buffer_read[position_mem];
			}
	}
	if (oldest_read_to_send != NULL){
		uint32_t ttc = orcs_engine.cacheManager->searchData(oldest_read_to_send);
		oldest_read_to_send->updatePackageReady(ttc);
		this->memory_read_executed--;
		#if PARALLEL_LIM_ACTIVE
			this->parallel_requests++; //numero de req paralelas, add+1
		#endif
		#if EMC_ACTIVE
			if (this->has_llc_miss)
			{
				this->has_llc_miss = false;
				if (this->isRobHead(oldest_read_to_send->rob_ptr))
				{
					// =====================================================
					// generate chain on home core buffer
					// =====================================================
					// ORCS_PRINTF("Global Cycle %lu\n",orcs_engine.get_global_cycle())
					this->rob_buffer.push_back(oldest_read_to_send->rob_ptr);
					this->make_dependence_chain(oldest_read_to_send->rob_ptr);
					// this->start_emc_module=true;
					oldest_read_to_send->rob_ptr->original_miss = true;
					this->add_llc_miss_rob_head();
				}
			}
		#endif
		#if MOB_DEBUG
			ORCS_PRINTF("On MOB READ Stage\n")
			ORCS_PRINTF("Time to complete READ %u\n", ttc)
			ORCS_PRINTF("MOB Line After EXECUTE %s\n", oldest_read_to_send->content_to_string().c_str())
		#endif
		oldest_read_to_send=NULL;
	} //end if mob_line null
	return OK;
}; //end method
// ============================================================================
uint32_t processor_t::mob_write(){
	#if MOB_DEBUG
		ORCS_PRINTF("MOB Write")
	#endif
	int32_t position_mem = POSITION_FAIL;
	if(oldest_write_to_send==NULL){
		#if PARALLEL_LIM_ACTIVE
			if (this->parallel_requests >= MAX_PARALLEL_REQUESTS)
			{
				// ORCS_PRINTF("Parallel Requests %d > MAX",this->parallel_requests)
				this->add_times_reach_parallel_requests_write();
				return FAIL;
			}
		#endif
		position_mem = memory_order_buffer_line_t::find_old_request_state_ready(this->memory_order_buffer_write,MOB_WRITE, PACKAGE_STATE_WAIT);
		if (position_mem != POSITION_FAIL){
			oldest_write_to_send = &this->memory_order_buffer_write[position_mem];
		}
	}
	if (oldest_write_to_send != NULL){
		// ORCS_PRINTF("iterations on mob Write %hhu \n",i)
		uint32_t ttc = 0;
		ttc = orcs_engine.cacheManager->writeData(oldest_write_to_send);
		// oldest_write_to_send->updatePackageReady(ttc);
		oldest_write_to_send->rob_ptr->stage=PROCESSOR_STAGE_COMMIT;
		oldest_write_to_send->rob_ptr->uop.updatePackageReady(ttc);
		oldest_write_to_send->rob_ptr->mob_ptr=NULL;
		//solving dendences
		this->solve_registers_dependency(oldest_write_to_send->rob_ptr);
		this->desambiguator->solve_memory_dependences(oldest_write_to_send);
		// #if PARALLEL_LIM_ACTIVE
		// 	this->parallel_requests++; //numero de req paralelas, add+1
		// #endif
		#if MOB_DEBUG
			ORCS_PRINTF("On MOB WRITE Stage\n")
			ORCS_PRINTF("Time to complete WRITE %u\n", ttc)
			ORCS_PRINTF("MOB Line After EXECUTE %s\n", oldest_write_to_send->content_to_string().c_str())
		#endif
		oldest_write_to_send->package_clean();
		this->memory_write_executed--; //numero de writes executados
		oldest_write_to_send=NULL;
	} //end if mob_line null
	return OK;
};
// ============================================================================

void processor_t::commit(){
	#if COMMIT_DEBUG
		if (orcs_engine.get_global_cycle() > WAIT_CYCLE)
		{
			ORCS_PRINTF("Commit Stage\n")
			ORCS_PRINTF("Cycle %lu\n", orcs_engine.get_global_cycle())
			ORCS_PRINTF("Rob Head %s\n", this->reorderBuffer[this->robStart].content_to_string().c_str())
		}
	#endif
	int32_t pos_buffer;

	/// Commit the packages
	for (uint32_t i = 0; i < COMMIT_WIDTH; i++)
	{
		pos_buffer = this->robStart;
		if (this->robUsed != 0 &&
			this->reorderBuffer[pos_buffer].stage == PROCESSOR_STAGE_COMMIT &&
			this->reorderBuffer[pos_buffer].uop.status == PACKAGE_STATE_READY &&
			this->reorderBuffer[pos_buffer].uop.readyAt <= orcs_engine.get_global_cycle())
		{

			this->commit_uop_counter++;
			switch (this->reorderBuffer[pos_buffer].uop.uop_operation)
			{
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
			case INSTRUCTION_OPERATION_MEM_LOAD:
				this->add_stat_inst_load_completed();
				break;
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
					ORCS_PRINTF("Commited Instruction\n%s\n", this->reorderBuffer[this->robStart].content_to_string().c_str())
				}
	// sleep(1);
	#endif
			this->removeFrontROB();
		}
		/// Could not commit the older, then stop looking for ready uops
		else
		{
			break;
		}
	}
} //end method
// ============================================================================
void processor_t::solve_registers_dependency(reorder_buffer_line_t *rob_line)
{

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
	#if EXECUTE_DEBUG
				ORCS_PRINTF("register_%u\n", write_register)
	#endif
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
};
// ============================================================================
bool processor_t::isRobHead(reorder_buffer_line_t *rob_line)
{
	// ORCS_PRINTF("rob_line: %p , rob Head: %p\n",rob_line,&this->reorderBuffer[robStart])
	return (rob_line == &this->reorderBuffer[robStart]);
};

// =====================================================================

void processor_t::make_dependence_chain(reorder_buffer_line_t *rob_line)
{
	// ORCS_PRINTF("Miss Original %s\n", rob_line->content_to_string().c_str())
	ERROR_ASSERT_PRINTF(rob_line->uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD, "Error, making dependences from NON-LOAD operation\n%s\n", rob_line->content_to_string().c_str())
	
	while(this->rob_buffer.size()<=EMC_UOP_BUFFER){
		int32_t next_position = this->get_next_uop_dependence();
		// ORCS_PRINTF("position ->%d\n",next_position)
		// sleep(1);
		if(next_position==POSITION_FAIL){
			break;
		}else{
			reorder_buffer_line_t *next_operation = this->rob_buffer[next_position];	
			if(next_operation->op_on_emc_buffer != next_operation->wait_reg_deps_number){
				this->rob_buffer.erase(this->rob_buffer.begin()+next_position);
				continue;
			}	
			for(uint32_t i = 0; i < ROB_SIZE; i++)
			{
				if(next_operation->reg_deps_ptr_array[i]==NULL){
					break;
				}
				if(this->already_exists(next_operation->reg_deps_ptr_array[i])){
					continue;
				}
				if(next_operation->reg_deps_ptr_array[i]->uop.uop_operation == INSTRUCTION_OPERATION_BRANCH ||
				next_operation->reg_deps_ptr_array[i]->uop.uop_operation == INSTRUCTION_OPERATION_INT_ALU ||
				next_operation->reg_deps_ptr_array[i]->uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD){ 
				// || next_operation->reg_deps_ptr_array[i]->uop.uop_operation == INSTRUCTION_OPERATION_MEM_STORE){
					//verify memory ambiguation
					if(
						// next_operation->reg_deps_ptr_array[i]->uop.uop_operation == INSTRUCTION_OPERATION_MEM_STORE || 
						next_operation->reg_deps_ptr_array[i]->uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD){
							if(this->verify_ambiguation(next_operation->reg_deps_ptr_array[i]->mob_ptr)){
								// cancel chain execution
								this->add_cancel_emc_execution();
								this->rob_buffer.clear();
								return;
							}
					}
					this->rob_buffer.push_back(next_operation->reg_deps_ptr_array[i]);
					next_operation->reg_deps_ptr_array[i]->op_on_emc_buffer++;
				}
			}
			next_operation->on_chain=true;
		}
	}	
	if(this->verify_dependent_loads()){
		this->start_emc_module=true;	
		this->add_started_emc_execution();
	}else{
		this->rob_buffer.clear();
		this->add_cancel_emc_execution();
	}
	
};
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
};
// =====================================================================
/*
get the position  on rob to initialize broadcast
@1 rob entry to be evaluated
@return position of rob entry on rob
*/
uint32_t processor_t::get_position_rob_bcast(reorder_buffer_line_t *rob_ready)
{
	uint32_t position = POSITION_FAIL;
	for (uint32_t i = (this->robStart+1);; i++)
	{
		if (i >= ROB_SIZE)
			i = 0;
		if (&this->reorderBuffer[i] == rob_ready)
		{
			position = i;
			return position;
		};
		if (i == this->robStart)
			break;
	}
	return position;
};
// =============================================================================
#if EMC_ACTIVE
int32_t processor_t::renameEMC(reorder_buffer_line_t *rob_line)
{
	// ===========================================================
	//Verificar se todos os registradores estao prontos ou no rrt
	// ===========================================================

	if(this->count_registers_rrt(rob_line->uop)<rob_line->wait_reg_deps_number){
		
		return POSITION_FAIL;
	}

	// ===========================================================
	// get position on emc lsq
	// ===========================================================
	memory_order_buffer_line_t *lsq;
	int32_t pos_lsq = memory_order_buffer_line_t::find_free(orcs_engine.memory_controller->emc->unified_lsq, EMC_LSQ_SIZE);
	if(pos_lsq == POSITION_FAIL){
		return POSITION_FAIL;
	};
	lsq = &orcs_engine.memory_controller->emc->unified_lsq[pos_lsq];
	// ===========================================================
	
	// ===========================================================
	// get position on uop buffer
	// ===========================================================
	int32_t pos_emc = orcs_engine.memory_controller->emc->get_position_uop_buffer();
	if(pos_emc==POSITION_FAIL){
		return POSITION_FAIL;
	}
	emc_opcode_package_t *emc_package = &orcs_engine.memory_controller->emc->uop_buffer[pos_emc];

	// ===========================================================
	// IF CAME HERE, CONGRATULATIONS. NEXT STEP
	// ===========================================================
	// Add the entry of uopbuffer on reservation station of EMC
	orcs_engine.memory_controller->emc->unified_rs.push_back(emc_package);
	// Fill EMC package with info
	emc_package->package_clean();		//clean package 
	emc_package->uop = rob_line->uop;	//copy uop to info operation
	emc_package->rob_ptr = rob_line;	//pointer to rob entry to return
	if (rob_line->mob_ptr != NULL){
		orcs_engine.memory_controller->emc->unified_lsq[pos_lsq] = *(rob_line->mob_ptr);	//copy infos of memory access
		if(rob_line->original_miss){ //if original memory miss, reduce latency
			emc_package->uop.readyAt = emc_package->uop.readyAt-(L1_DATA_LATENCY+LLC_LATENCY);
			lsq->readyAt=lsq->readyAt-(L1_DATA_LATENCY+LLC_LATENCY);
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
	for (uint32_t k = 0; k < MAX_REGISTERS; k++)
	{
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
	for (uint32_t k = 0; k < MAX_REGISTERS; k++)
	{
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
	return OK;
};
#endif
// =======================================================================
/*
 @1 write register to be searched in rrt
 @return position of register in rrt
 */
int32_t processor_t::search_register(int32_t write_register)
{
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
};
// =======================================================================
/*
 @1 write register to be allocated in rrt
 @return position of register allocated in rrt
 */
int32_t processor_t::allocate_new_register(int32_t write_register)
{
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
};
// ============================================================================
/*
Clean register remapping table
*/
void processor_t::clean_rrt()
{
	for (size_t i = 0; i < EMC_REGISTERS; i++)
	{
		this->rrt[i].package_clean();
	}
};
// ============================================================================
/*
Verify register spill to include stores on chain;
@1 rob entry to compare address
*/
bool processor_t::verify_spill_register(reorder_buffer_line_t *rob_line)
{
	bool spill = false;
	for (size_t i = 0; i < MOB_READ; i++)
	{
		if (!(this->memory_order_buffer_read[i].memory_address ^ rob_line->mob_ptr->memory_address))
		{
			spill = true;
			break;
		}
	}
	return spill;
};
// ============================================================================
// @return true if no loads operation is gt 1
bool processor_t::verify_dependent_loads()
{
	uint8_t loads = 1;
	for (uint32_t i = 1; i < this->rob_buffer.size(); i++)
	{
		if(this->rob_buffer[i]->uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD){
			loads++;
		}
	}
	
	return (loads > 1) ? true : false; //if compacto
};
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
};

// ============================================================================

void processor_t::cancel_execution_emc()
{
	for (uint8_t i = 0; i < EMC_UOP_BUFFER; i++)
	{

		if (orcs_engine.memory_controller->emc->uop_buffer[i].rob_ptr != NULL)
		{
			orcs_engine.memory_controller->emc->uop_buffer[i].rob_ptr->on_chain = false;
		}
		if (orcs_engine.memory_controller->emc->uop_buffer[i].rob_ptr != NULL)
		{
			orcs_engine.memory_controller->emc->uop_buffer[i].rob_ptr->is_poisoned = false;
		}
			orcs_engine.memory_controller->emc->unified_lsq[i].package_clean();
			orcs_engine.memory_controller->emc->uop_buffer[i].package_clean();

	}
	//zerando controles
	orcs_engine.memory_controller->emc->uop_buffer_end = 0;
	orcs_engine.memory_controller->emc->uop_buffer_start= 0;
	orcs_engine.memory_controller->emc->uop_buffer_used = 0;
	//pointers containers RS e Unified FUS
	orcs_engine.memory_controller->emc->unified_fus.clear();
	orcs_engine.memory_controller->emc->unified_rs.clear();

	//conferindo estruturas -> comentar futuramente
	orcs_engine.memory_controller->emc->print_structures();
};
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
};
// ============================================================================
bool processor_t::verify_ambiguation(memory_order_buffer_line_t *mob_line){
	
	for(uint32_t i = 0; i < MOB_READ; i++){
		if(mob_line->memory_address == this->memory_order_buffer_read[i].memory_address && 
			mob_line->rob_ptr->uop.opcode_number != this->memory_order_buffer_read[i].rob_ptr->uop.opcode_number)
			return true;
	}
	for(uint32_t i = 0; i < MOB_WRITE; i++){
		if(mob_line->memory_address == this->memory_order_buffer_write[i].memory_address && 
			mob_line->rob_ptr->uop.opcode_number != this->memory_order_buffer_write[i].rob_ptr->uop.opcode_number)
			return true;
	}
	return false;
};
// ============================================================================
void processor_t::statistics()
{
		if (orcs_engine.output_file_name == NULL)
		{
			utils_t::largestSeparator();
			ORCS_PRINTF("Total_Cicle : %lu\n", orcs_engine.get_global_cycle())
			utils_t::largeSeparator();
			ORCS_PRINTF("Stage_Opcode_and_Uop_Counters\n")
			utils_t::largeSeparator();
			ORCS_PRINTF("Stage_Fetch: %lu\n", this->fetchCounter)
			ORCS_PRINTF("Stage_Decode: %lu\n", this->decodeCounter)
			ORCS_PRINTF("Stage_Rename: %lu\n", this->renameCounter)
			ORCS_PRINTF("Register_writes: %lu\n", this->get_registerWrite())
			ORCS_PRINTF("Stage_Commit: %lu\n", this->commit_uop_counter)
			utils_t::largestSeparator();
			ORCS_PRINTF("======================== MEMORY DESAMBIGUATION ===========================\n")
			utils_t::largestSeparator();
			this->desambiguator->statistics();
	#if MAX_PARALLEL_REQUESTS
			ORCS_PRINTF("Times_Reach_MAX_PARALLEL_REQUESTS_READ: %lu\n", this->get_times_reach_parallel_requests_read())
			ORCS_PRINTF("Times_Reach_MAX_PARALLEL_REQUESTS_WRITE: %lu\n", this->get_times_reach_parallel_requests_write())
	#endif
			utils_t::largestSeparator();
			ORCS_PRINTF("Instruction_Per_Cicle: %.4f\n", float(this->fetchCounter) / float(orcs_engine.get_global_cycle()))
	#if EMC_ACTIVE
			ORCS_PRINTF("\n======================== EMC INFOS ===========================\n")
			utils_t::largeSeparator();
			ORCS_PRINTF("times_llc_rob_head: %u\n", this->get_llc_miss_rob_head())
			ORCS_PRINTF("numero_load_deps: %u\n", this->numero_load_deps)
			ORCS_PRINTF("Total_instrucoes_dependentes: %u\n", this->soma_instrucoes_deps)
			ORCS_PRINTF("load_deps_ratio: %.4f\n", float(this->soma_instrucoes_deps) / float(this->numero_load_deps))
			ORCS_PRINTF("started_emc_execution: %d\n", this->get_started_emc_execution())
			ORCS_PRINTF("canceled_emc_execution: %d\n", this->get_cancel_emc_execution())
	#endif
			utils_t::largeSeparator();
		}
		else
		{
			FILE *output = fopen(orcs_engine.output_file_name, "a+");
			if (output != NULL)
			{
				utils_t::largestSeparator(output);
				fprintf(output, "Total_Cycle: %lu\n", orcs_engine.get_global_cycle());
				utils_t::largeSeparator(output);
				fprintf(output, "Stage_Opcode_and_Uop_Counters\n");
				utils_t::largeSeparator(output);
				fprintf(output, "Stage_Fetch: %lu\n", this->fetchCounter);
				fprintf(output, "Stage_Decode: %lu\n", this->decodeCounter);
				fprintf(output, "Stage_Rename: %lu\n", this->renameCounter);
				fprintf(output, "Register_writes: %lu\n", this->get_registerWrite());
				fprintf(output, "Stage_Commit: %lu\n", this->commit_uop_counter);
				utils_t::largestSeparator(output);
				fprintf(output, "======================== MEMORY DESAMBIGUATION ===========================\n");
				utils_t::largestSeparator(output);
				this->desambiguator->statistics();
	#if MAX_PARALLEL_REQUESTS
				fprintf(output, "Times_Reach_MAX_PARALLEL_REQUESTS_READ: %lu\n", this->get_times_reach_parallel_requests_read());
				fprintf(output, "Times_Reach_MAX_PARALLEL_REQUESTS_WRITE: %lu\n", this->get_times_reach_parallel_requests_write());
	#endif
				utils_t::largestSeparator(output);
				fprintf(output, "Instruction_Per_Cycle: %.4f\n", float(this->fetchCounter) / float(orcs_engine.get_global_cycle()));
	#if EMC_ACTIVE
				fprintf(output, "\n======================== EMC INFOS ===========================\n");
				utils_t::largeSeparator(output);
				fprintf(output, "times_llc_rob_head: %u\n", this->get_llc_miss_rob_head());
				fprintf(output, "numero_load_deps: %u\n", this->numero_load_deps);
				fprintf(output, "Total_instrucoes_dependentes: %u\n", this->soma_instrucoes_deps);
				fprintf(output, "load_deps_ratio: %.4f\n", float(this->soma_instrucoes_deps) / float(this->numero_load_deps));
				fprintf(output, "started_emc_execution: %d\n", this->get_started_emc_execution());
				fprintf(output, "canceled_emc_execution: %d\n", this->get_cancel_emc_execution());
	#endif
			}
			fclose(output);
	}
};
// ============================================================================
void processor_t::printConfiguration()
{
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
		fprintf(output, "MAX_PARALLEL_REQUESTS ->%u\n", MAX_PARALLEL_REQUESTS);
	}
}
// ============================================================================
void processor_t::printStructures()
{
	ORCS_PRINTF("Periodic Check -  Structures at %lu\n", orcs_engine.get_global_cycle())
	ORCS_PRINTF("Fetched Opcodes %lu of %lu\n", orcs_engine.trace_reader->get_fetch_instructions(), orcs_engine.trace_reader->get_trace_opcode_max())
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
	ORCS_PRINTF("Parallel Requests %d\n", this->parallel_requests)
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
void processor_t::clock()
{
#if DEBUG
	if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
		ORCS_PRINTF("============================PROCESSOR===============================\n")
		ORCS_PRINTF("Cycle %lu\n",orcs_engine.get_global_cycle())
	}
#endif
	// // ======================================================
	// // Se estiver em execução no EMC trava o home core
	// // ======================================================
	// if(this->on_emc_execution){
	// 	return;
	// }
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
		orcs_engine.simulator_alive = false;
	}
#if DEBUG
	if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
		ORCS_PRINTF("===================================================================\n")
		sleep(1);
	}
#endif
#if PERIODIC_CHECK
	if (orcs_engine.get_global_cycle() % CLOCKS_TO_CHECK == 0)
	{
		this->printStructures();
		// ORCS_PRINTF("Opcodes Processed %lu",orcs_engine.trace_reader->get_fetch_instructions())
	}
#endif
};
