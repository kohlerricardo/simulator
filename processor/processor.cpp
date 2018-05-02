#include "../simulator.hpp"

// =====================================================================
processor_t::processor_t(){
	//Setting Pointers to NULL
	// ========MOB======
	this->memory_order_buffer_read = NULL;
	this->memory_order_buffer_write = NULL;
	//=========DESAMBIGUATION HASH ============
	this->disambiguation_store_hash = NULL;
	this->disambiguation_load_hash = NULL;
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

};
processor_t::~processor_t(){
	//NULLing Pointers
	//deleting MOB read and MOB write
	utils_t::template_delete_array<memory_order_buffer_line_t*>(this->disambiguation_load_hash);
    utils_t::template_delete_array<memory_order_buffer_line_t*>(this->disambiguation_store_hash);
	utils_t::template_delete_array<memory_order_buffer_line_t>(this->memory_order_buffer_read);
	utils_t::template_delete_array<memory_order_buffer_line_t>(this->memory_order_buffer_write);
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

};
// =====================================================================
void processor_t::allocate(){
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
	this->memory_read_executed=0;
	this->memory_write_executed=0;
	//======================================================================
	// Initializating structures
	//======================================================================
	//======================================================================
	// FetchBuffer
	this->fetchBuffer.allocate(FETCH_BUFFER);
	// DecodeBuffer
	this->decodeBuffer.allocate(DECODE_BUFFER);
	// Register Alias Table
	this->register_alias_table = utils_t::template_allocate_initialize_array<reorder_buffer_line_t*>(RAT_SIZE, NULL);
	// Reorder Buffer
	this->robStart = 0;
	this->robEnd = 0;
	this->robUsed = 0;
	this->reorderBuffer = utils_t::template_allocate_array<reorder_buffer_line_t>(ROB_SIZE);
	for (uint32_t i = 0; i < ROB_SIZE; i++)
	{
		this->reorderBuffer[i].reg_deps_ptr_array = utils_t::template_allocate_initialize_array<reorder_buffer_line_t*>(ROB_SIZE, NULL);
	}
	// // Memory Order Buffer Read
	this->memory_order_buffer_read = utils_t::template_allocate_array<memory_order_buffer_line_t>(MOB_READ);
	for (size_t i = 0; i < MOB_READ; i++)
	{
		this->memory_order_buffer_read[i].mem_deps_ptr_array = utils_t::template_allocate_initialize_array<memory_order_buffer_line_t*>(ROB_SIZE,NULL);
	}
	// this->memory_order_buffer_read_used = 0; 
	// this->memory_order_buffer_read_start = 0; 
	// this->memory_order_buffer_read_end = 0; 
	// // Memory Order Buffer Write
	this->memory_order_buffer_write = utils_t::template_allocate_array<memory_order_buffer_line_t>(MOB_WRITE);
	for (size_t i = 0; i < MOB_READ; i++)
	{
		this->memory_order_buffer_write[i].mem_deps_ptr_array = utils_t::template_allocate_initialize_array<memory_order_buffer_line_t*>(ROB_SIZE,NULL);
	}
	// this->memory_order_buffer_write_used = 0;
	// this->memory_order_buffer_write_start = 0;
	// this->memory_order_buffer_write_end = 0;
	// =========================================================================================
	    /// DISAMBIGUATION OFFSET MASK
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(LOAD_HASH_SIZE), "Wrong disambiguation_load_hash_size.\n")
    this->disambiguation_load_hash_bits_mask = 0;
    for (uint32_t i = 0; i < utils_t::get_power_of_two(LOAD_HASH_SIZE); i++) {
        this->disambiguation_load_hash_bits_mask |= 1 << i;
    }
    this->disambiguation_load_hash_bits_shift = utils_t::get_power_of_two(DESAMBIGUATION_BLOCK_SIZE);
    this->disambiguation_load_hash_bits_mask <<= this->disambiguation_load_hash_bits_shift;
    this->disambiguation_load_hash = utils_t::template_allocate_initialize_array<memory_order_buffer_line_t*>(LOAD_HASH_SIZE, NULL);


    /// DISAMBIGUATION OFFSET MASK
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(STORE_HASH_SIZE), "Wrong disambiguation_store_hash_size.\n")
    this->disambiguation_store_hash_bits_mask = 0;
    for (uint32_t i = 0; i < utils_t::get_power_of_two(STORE_HASH_SIZE); i++) {
        this->disambiguation_store_hash_bits_mask |= 1 << i;
    }
    this->disambiguation_store_hash_bits_shift <<= utils_t::get_power_of_two(DESAMBIGUATION_BLOCK_SIZE);
    this->disambiguation_store_hash_bits_mask <<= this->disambiguation_store_hash_bits_shift;
    this->disambiguation_store_hash = utils_t::template_allocate_initialize_array<memory_order_buffer_line_t*>(STORE_HASH_SIZE, NULL);

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

	// this->printConfiguration();
};
// =====================================================================
bool processor_t::isBusy(){
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
};
// ======================================
// Remove the Head of the reorder buffer
// The Reorder Buffer behavior is a Circular FIFO
// ======================================
void processor_t::removeFrontROB(){
	#if COMMIT_DEBUG
		ORCS_PRINTF("Cycle  %lu\n",orcs_engine.get_global_cycle())
		ORCS_PRINTF("Trying remove \n%s\n",this->reorderBuffer[this->robStart].content_to_string().c_str())
	#endif
	ERROR_ASSERT_PRINTF(this->robUsed > 0, "Removendo do ROB sem estar usado\n")
	ERROR_ASSERT_PRINTF(this->reorderBuffer[this->robStart].reg_deps_ptr_array[0] == NULL, "Removendo sem resolver dependencias\n")
	this->reorderBuffer[this->robStart].package_clean();
	this->robUsed--;
	this->robStart++;
	if (this->robStart >= ROB_SIZE)
	{
		this->robStart = 0;
	}
};
// =====================================================================

void processor_t::fetch(){
		#if FETCH_DEBUG
			ORCS_PRINTF("Fetch Stage\n")
		#endif
	opcode_package_t operation;
	// uint32_t position;
	// Trace ->fetchBuffer
	for (int i = 0; i < FETCH_WIDTH; i++){
		operation.package_clean();
		bool updated=false;
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
			operation.updatePackageReady(stallWrongBranch+ttc);
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
		if(!updated){
			uint32_t ttc = orcs_engine.cacheManager->searchInstruction(operation.opcode_address);
			#if FETCH_DEBUG
			ORCS_PRINTF("(%lu) (%lu) TTC %u\n",orcs_engine.get_global_cycle(),operation.opcode_address,ttc)
			ORCS_PRINTF("(%lu) (%lu) readyAt After  %lu\n",orcs_engine.get_global_cycle(),operation.readyAt,(operation.readyAt+ttc))
			sleep(1);
			#endif
			this->fetchBuffer.back()->updatePackageReady(FETCH_LATENCY+ttc);
		}
	}
};

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
			ORCS_PRINTF("Opcode to decode %s\n",this->fetchBuffer.front()->content_to_string().c_str())
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
                for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
                    new_uop.read_regs[i] = POSITION_FAIL;
                }
                /// Insert BASE and INDEX into RReg
                new_uop.read_regs[0] = this->fetchBuffer.front()->base_reg;
                new_uop.read_regs[1] = this->fetchBuffer.front()->index_reg;

                // ===== Write Regs =============================================
                /// Clear WRegs
                for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
                    new_uop.write_regs[i] = POSITION_FAIL;
                }
                /// Insert 258 into WRegs
                new_uop.write_regs[0] = 258;
			}
			new_uop.updatePackageReady(DECODE_LATENCY);
			// printf("\n UOP Created %s \n",new_uop.content_to_string().c_str());
			statusInsert = this->decodeBuffer.push_back(new_uop);
	#if DECODE_DEBUG
				ORCS_PRINTF("uop created %s\n",this->decodeBuffer.back()->content_to_string2().c_str())
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
                for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
                    new_uop.read_regs[i] = POSITION_FAIL;
                }
                /// Insert BASE and INDEX into RReg
                new_uop.read_regs[0] = this->fetchBuffer.front()->base_reg;
                new_uop.read_regs[1] = this->fetchBuffer.front()->index_reg;

                // ===== Write Regs =============================================
                /// Clear WRegs
                for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
                    new_uop.write_regs[i] = POSITION_FAIL;
                }
                /// Insert 258 into WRegs
                new_uop.write_regs[0] = 258;
			}
			new_uop.updatePackageReady(DECODE_LATENCY);
			// printf("\n UOP Created %s \n",new_uop.content_to_string().c_str());
			statusInsert = this->decodeBuffer.push_back(new_uop);
	#if DECODE_DEBUG
				ORCS_PRINTF("uop created %s\n",this->decodeBuffer.back()->content_to_string2().c_str())
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
				ORCS_PRINTF("uop created %s\n",this->decodeBuffer.back()->content_to_string2().c_str())
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
                for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
                    if (new_uop.read_regs[i] == POSITION_FAIL) {
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
				ORCS_PRINTF("uop created %s\n",this->decodeBuffer.back()->content_to_string2().c_str())
	#endif
			ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL,"Erro, Tentando decodificar mais uops que o maximo permitido")
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
				ORCS_PRINTF("uop created %s\n",this->decodeBuffer.back()->content_to_string2().c_str())
	#endif
			ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL, "Erro, Tentando decodificar mais uops que o maximo permitido")
		}
		this->fetchBuffer.pop_front();
		
	}

};

// ============================================================================
void processor_t::update_registers(reorder_buffer_line_t *new_rob_line){
    /// Control the Register Dependency - Register READ
    for (uint32_t k = 0; k < MAX_REGISTERS; k++) {
        if (new_rob_line->uop.read_regs[k] < 0) {
            break;
        }
        uint32_t read_register = new_rob_line->uop.read_regs[k];
        ERROR_ASSERT_PRINTF(read_register < RAT_SIZE, "Read Register (%d) > Register Alias Table Size (%d)\n", read_register, RAT_SIZE);
        /// If there is a dependency
        if (this->register_alias_table[read_register] != NULL) {
            for (uint32_t j = 0; j < ROB_SIZE; j++) {
				#if RENAME_DEBUG	
				ORCS_PRINTF("register %u, dep %u\n",read_register,j)
				#endif
                if (this->register_alias_table[read_register]->reg_deps_ptr_array[j] == NULL) {
					this->register_alias_table[read_register]->wake_up_elements_counter++;
                    this->register_alias_table[read_register]->reg_deps_ptr_array[j] = new_rob_line;
                    new_rob_line->wait_reg_deps_number++;
                    break;
                }
            }
        }
    }

    /// Control the Register Dependency - Register WRITE
    for (uint32_t k = 0; k < MAX_REGISTERS; k++) {
        if (new_rob_line->uop.write_regs[k] < 0) {
            break;
        }
        uint32_t write_register = new_rob_line->uop.write_regs[k];
        ERROR_ASSERT_PRINTF(write_register < RAT_SIZE, "Write Register (%d) > Register Alias Table Size (%d)\n", write_register, RAT_SIZE);

        this->register_alias_table[write_register] = new_rob_line;
    }
};
// ============================================================================
void processor_t::make_memory_dependencies(memory_order_buffer_line_t *new_mob_line){

    uint64_t load_hash = new_mob_line->memory_address & this->disambiguation_load_hash_bits_mask;
    uint64_t store_hash = new_mob_line->memory_address & this->disambiguation_store_hash_bits_mask;
	// ORCS_PRINTF("Memory address -> %lu\n",new_mob_line->memory_address)
    load_hash >>= this->disambiguation_load_hash_bits_shift;
    store_hash >>= this->disambiguation_store_hash_bits_shift;

    memory_order_buffer_line_t *old_mob_line = NULL;

    /// Check if LOAD_HASH matches
    ERROR_ASSERT_PRINTF(load_hash < LOAD_HASH_SIZE, "load_hash (%lu) > disambiguation_load_hash_size (%d)\n",
                                                                    load_hash, LOAD_HASH_SIZE);
    /// Check if STORE_HASH matches
    ERROR_ASSERT_PRINTF(store_hash < STORE_HASH_SIZE, "store_hash (%lu) > disambiguation_store_hash_size (%d)\n",
                                                                        store_hash, STORE_HASH_SIZE);



    /// Create R -> W,  R -> R
    if (this->disambiguation_load_hash[load_hash] != NULL){
        old_mob_line = disambiguation_load_hash[load_hash];
        for (uint32_t k = 0; k < ROB_SIZE; k++) {
            if (old_mob_line->mem_deps_ptr_array[k] == NULL) {
                old_mob_line->mem_deps_ptr_array[k] = new_mob_line;
                new_mob_line->wait_mem_deps_number++;
                break;
            }
        }
    }


    /// Create W -> R, W -> W deps.
    if (this->disambiguation_store_hash[store_hash] != NULL){
        old_mob_line = disambiguation_store_hash[store_hash];
        for (uint32_t k = 0; k < ROB_SIZE; k++) {
            if (old_mob_line->mem_deps_ptr_array[k] == NULL) {
                old_mob_line->mem_deps_ptr_array[k] = new_mob_line;
                new_mob_line->wait_mem_deps_number++;
                break;
            }
        }
    }

    /// Add the new entry into LOAD or STORE hash
    if (new_mob_line->memory_operation == MEMORY_OPERATION_READ){
        this->disambiguation_load_hash[load_hash] = new_mob_line;
    }
    else {
        this->disambiguation_store_hash[store_hash] = new_mob_line;
    }
};
void processor_t::solve_memory_dependency(memory_order_buffer_line_t *mob_line) {

    /// Remove pointers from disambiguation_hash
    /// Add the new entry into LOAD or STORE hash
    if (mob_line->memory_operation == MEMORY_OPERATION_READ){
        uint64_t load_hash = mob_line->memory_address & this->disambiguation_load_hash_bits_mask;
        load_hash >>= this->disambiguation_load_hash_bits_shift;

        ERROR_ASSERT_PRINTF(load_hash < LOAD_HASH_SIZE, "load_hash (%" PRIu64 ") > disambiguation_load_hash_size (%d)\n",
                                                                            load_hash, LOAD_HASH_SIZE);
        if (this->disambiguation_load_hash[load_hash] == mob_line){
            this->disambiguation_load_hash[load_hash] = NULL;
        }
    }
    else {
        uint64_t store_hash = mob_line->memory_address & this->disambiguation_store_hash_bits_mask;
        store_hash >>= this->disambiguation_store_hash_bits_shift;

        ERROR_ASSERT_PRINTF(store_hash < STORE_HASH_SIZE, "store_hash (%" PRIu64 ") > disambiguation_store_hash_size (%d)\n",
                                                                            store_hash, STORE_HASH_SIZE);

        if (this->disambiguation_store_hash[store_hash] == mob_line){
            this->disambiguation_store_hash[store_hash] = NULL;
        }
    }


    // =========================================================================
    /// SOLVE MEMORY DEPENDENCIES - MOB
    // =========================================================================
    /// Send message to acknowledge the dependency is over
    for (uint32_t j = 0; j < ROB_SIZE; j++) {
        /// All the dependencies are solved
        if (mob_line->mem_deps_ptr_array[j] == NULL) {
            break;
        }

        /// Keep track of false positives
        if (mob_line->mem_deps_ptr_array[j]->memory_address != mob_line->memory_address) {
            if (mob_line->memory_operation == MEMORY_OPERATION_READ) {
                this->add_stat_disambiguation_read_false_positive();
            }
            else {
                this->add_stat_disambiguation_write_false_positive();
            }
        }

        /// There is an unsolved dependency
        mob_line->mem_deps_ptr_array[j]->wait_mem_deps_number--;
	
        if (ADDRESS_TO_ADDRESS == 1) {
            if (mob_line->mem_deps_ptr_array[j]->uop_executed == true &&
            mob_line->mem_deps_ptr_array[j]->wait_mem_deps_number == 0 &&
            mob_line->mem_deps_ptr_array[j]->memory_operation == MEMORY_OPERATION_READ &&
            mob_line->mem_deps_ptr_array[j]->memory_address == mob_line->memory_address &&
            mob_line->mem_deps_ptr_array[j]->memory_size == mob_line->memory_size) {
                this->add_stat_address_to_address();
                mob_line->mem_deps_ptr_array[j]->status = PACKAGE_STATE_READY;
                mob_line->mem_deps_ptr_array[j]->readyAt =  orcs_engine.get_global_cycle() + REGISTER_FORWARD;

            }
        }
        /// This update the ready cycle, and it is usefull to compute the time each instruction waits for the functional unit
        mob_line->mem_deps_ptr_array[j] = NULL;
    }
};
// =================================================================

// ===================MOB READ RELATED==============================
/*
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
	void processor_t::remove_front_mob_read(){
		ERROR_ASSERT_PRINTF(this->memory_order_buffer_read_used > 0, "Removendo do MOB_READ sem estar usado")
		ERROR_ASSERT_PRINTF(this->memory_order_buffer_read[this->memory_order_buffer_read_start].uop_executed == true, "Removendo sem Uop estar executado")
		this->memory_order_buffer_read[this->memory_order_buffer_read_start].package_clean();
		this->memory_order_buffer_read_used--;
		this->memory_order_buffer_read_start++;
		if (this->memory_order_buffer_read_start >= MOB_READ)
		{
			this->memory_order_buffer_read_start = 0;
		}
	};
	// =================================================================
	// ======================MOB WRITE RELATED==========================
	int32_t processor_t::search_position_mob_write(){
		int32_t position = POSITION_FAIL;
		/// There is free space.
		if (this->memory_order_buffer_read_used < MOB_WRITE)
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
	void processor_t::remove_front_mob_write()
	{
		ERROR_ASSERT_PRINTF(this->memory_order_buffer_write_used > 0, "Removendo do MOB_WRITE sem estar usado")
		ERROR_ASSERT_PRINTF(this->memory_order_buffer_write[this->memory_order_buffer_write_start].uop_executed == true, "Removendo sem Uop estar executado")
		this->memory_order_buffer_write[this->memory_order_buffer_write_start].package_clean();
		this->memory_order_buffer_write_used--;
		this->memory_order_buffer_write_start++;
		if (this->memory_order_buffer_write_start >= MOB_WRITE)
		{
			this->memory_order_buffer_write_start = 0;
		}
	};
*/
// =================================================

void processor_t::rename(){
		#if RENAME_DEBUG
			ORCS_PRINTF("Rename Stage\n")
		#endif
	size_t i;
	int32_t pos_rob,pos_mob;

	
	for (i = 0; i < RENAME_WIDTH;i++)
	{
		memory_order_buffer_line_t *mob_line = NULL;
		// Checando se há uop decodificado, se está pronto, e se o ciclo de pronto
		// é maior ou igual ao atual
		if (this->decodeBuffer.is_empty() ||
			this->decodeBuffer.front()->status != PACKAGE_STATE_READY ||
			this->decodeBuffer.front()->readyAt > orcs_engine.get_global_cycle()){
			break;
		}
		ERROR_ASSERT_PRINTF(this->decodeBuffer.front()->uop_number == this->renameCounter, "Erro, renomeio incorreto\n")
		//=======================
		// Memory Operation Read
		//=======================
		if (this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_MEM_LOAD){
			pos_mob = memory_order_buffer_line_t::find_free(this->memory_order_buffer_read,MOB_READ);
			if(pos_mob == POSITION_FAIL){
				this->add_stall_full_MOB_Read();
				break;
			}
			mob_line = &this->memory_order_buffer_read[pos_mob];
		}
		//=======================
		// Memory Operation Write
		//=======================
		if (this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_MEM_STORE){
			pos_mob = memory_order_buffer_line_t::find_free(this->memory_order_buffer_write,MOB_WRITE);
			if(pos_mob == POSITION_FAIL){
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
		this->reorderBuffer[pos_rob].uop.updatePackageReady(RENAME_LATENCY+DISPATCH_LATENCY);
		this->reorderBuffer[pos_rob].mob_ptr = mob_line;
		// =======================
		// Making registers dependences
		// =======================
		this->update_registers(&this->reorderBuffer[pos_rob]);
		#if RENAME_DEBUG
			ORCS_PRINTF("Rename %s\n",this->reorderBuffer[pos_rob].content_to_string().c_str())
		#endif
		// =======================
		// Insert into Reservation Station
		// =======================
		this->unified_reservation_station.push_back(&this->reorderBuffer[pos_rob]);
		// =======================
		// Insert into MOB.
		// =======================
		if (this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD){	
			#if RENAME_DEBUG
			ORCS_PRINTF("Mem Load\n")
			#endif 
			this->reorderBuffer[pos_rob].mob_ptr->opcode_address = this->reorderBuffer[pos_rob].uop.opcode_address;
			this->reorderBuffer[pos_rob].mob_ptr->memory_address = this->reorderBuffer[pos_rob].uop.memory_address;
			this->reorderBuffer[pos_rob].mob_ptr->memory_size = this->reorderBuffer[pos_rob].uop.memory_size;
			this->reorderBuffer[pos_rob].mob_ptr->memory_operation = MEMORY_OPERATION_READ;
			this->reorderBuffer[pos_rob].mob_ptr->status = PACKAGE_STATE_UNTREATED;
			this->reorderBuffer[pos_rob].mob_ptr->readyToGo = orcs_engine.get_global_cycle()+RENAME_LATENCY+DISPATCH_LATENCY+EXECUTE_LATENCY;
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
			this->reorderBuffer[pos_rob].mob_ptr->status = PACKAGE_STATE_UNTREATED;
			this->reorderBuffer[pos_rob].mob_ptr->readyToGo = orcs_engine.get_global_cycle()+RENAME_LATENCY+DISPATCH_LATENCY+EXECUTE_LATENCY;
			this->reorderBuffer[pos_rob].mob_ptr->uop_number = this->reorderBuffer[pos_rob].uop.uop_number;
		}
		//linking rob and mob
		if(this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD || 
			this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_MEM_STORE ){
			mob_line->rob_ptr = &this->reorderBuffer[pos_rob];
			#if DESAMBIGUATION_ENABLED
			this->make_memory_dependencies(this->reorderBuffer[pos_rob].mob_ptr);
			#endif
		}
	} //end for

} //end method""

/*
void processor_t::rename(){
        #if RENAME_DEBUG
            ORCS_PRINTF("Rename Stage\n")
        #endif
    size_t i;
    int32_t pos_rob;
 
     
    for (i = 0; i < RENAME_WIDTH;i++)
    {
        // Checando se há uop decodificado, se está pronto, e se o ciclo de pronto
        // é maior ou igual ao atual
        if (this->decodeBuffer.is_empty() ||
            this->decodeBuffer.front()->status != PACKAGE_STATE_READY ||
            this->decodeBuffer.front()->readyAt > orcs_engine.get_global_cycle()){
            break;
        }
        ERROR_ASSERT_PRINTF(this->decodeBuffer.front()->uop_number == this->renameCounter, "Erro, renomeio incorreto\n")
        memory_order_buffer_line_t mob_line;
 
        //=======================
        // Memory Operation Read
        //=======================
        if (this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_MEM_LOAD)
        {
             
            if (this->memory_order_buffer_read.size() == MOB_READ)
            {
                this->add_stall_full_MOB_Read();
                break;
            }
        }
        //=======================
        // Memory Operation Write
        //=======================
        if (this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_MEM_STORE)
        {
            if (this->memory_order_buffer_write.size() == MOB_WRITE)
            {
                this->add_stall_full_MOB_Write();
                break;
            }
        }
        //=======================
        // Verificando se tem espaco no ROB se sim bamos inserir
        //=======================
        pos_rob = this->searchPositionROB();
        #if RENAME_DEBUG
			if(orcs_engine.get_global_cycle()>=WAIT_CYCLE){
				ORCS_PRINTF("pos_rob %u\n",pos_rob)
				ORCS_PRINTF("Rob Start %u\n",this->robStart)
				ORCS_PRINTF("Rob Used %u\n",this->robUsed)
				ORCS_PRINTF("uop %s\n",this->decodeBuffer.front()->content_to_string().c_str())
				// ORCS_PRINTF("decode buffer size %u\n",this->decodeBuffer.get_size())
				sleep(1);
			}
        #endif 
 
        if (pos_rob == POSITION_FAIL)
        {
            this->add_stall_full_ROB();
            break;
        }
         
        this->reorderBuffer[pos_rob].uop = *this->decodeBuffer.front();
        //remove uop from decodebuffer
        this->decodeBuffer.front()->package_clean();
        this->decodeBuffer.pop_front();
        //////////////////
        //add counter renamed uops
        ERROR_ASSERT_PRINTF(this->reorderBuffer[pos_rob].uop.uop_number == this->renameCounter,"Error, Renaming Out of order")
        this->renameCounter++;
        // =======================
        // Setting controls to ROB.
        // =======================
        this->reorderBuffer[pos_rob].stage = PROCESSOR_STAGE_RENAME;
        this->reorderBuffer[pos_rob].uop.updatePackageReady(RENAME_LATENCY+DISPATCH_LATENCY);
 
        // acertar as dependencias de registradores
        // =======================
        // Insert into MOB.
        // =======================
        if (this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD)
        {   		
            #if RENAME_DEBUG
            ORCS_PRINTF("Mem Load\n")
            #endif 
			this->memory_order_buffer_read.push_back(mob_line);
            this->memory_order_buffer_read.back().opcode_address = this->reorderBuffer[pos_rob].uop.opcode_address;
            this->memory_order_buffer_read.back().memory_address = this->reorderBuffer[pos_rob].uop.memory_address;
            this->memory_order_buffer_read.back().memory_size = this->reorderBuffer[pos_rob].uop.memory_size;
            this->memory_order_buffer_read.back().memory_operation = MEMORY_OPERATION_READ;
            this->memory_order_buffer_read.back().status = PACKAGE_STATE_UNTREATED;
            this->memory_order_buffer_read.back().born_cicle = orcs_engine.get_global_cycle();
            this->memory_order_buffer_read.back().rob_ptr = &this->reorderBuffer[pos_rob];
            this->reorderBuffer[pos_rob].mob_ptr = &this->memory_order_buffer_read.back();
			this->make_memory_dependencies(&this->memory_order_buffer_read.back());
        }
        else if (this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_MEM_STORE)
        {
            #if RENAME_DEBUG
            ORCS_PRINTF("Mem Store\n")
            #endif 
			this->memory_order_buffer_write.push_back(mob_line);
            this->memory_order_buffer_write.back().opcode_address = this->reorderBuffer[pos_rob].uop.opcode_address;
            this->memory_order_buffer_write.back().memory_address = this->reorderBuffer[pos_rob].uop.memory_address;
            this->memory_order_buffer_write.back().memory_size = this->reorderBuffer[pos_rob].uop.memory_size;
            this->memory_order_buffer_write.back().memory_operation = MEMORY_OPERATION_WRITE;
            this->memory_order_buffer_write.back().status = PACKAGE_STATE_UNTREATED;
            this->memory_order_buffer_write.back().born_cicle = orcs_engine.get_global_cycle();
            this->memory_order_buffer_write.back().rob_ptr = &this->reorderBuffer[pos_rob];
            this->reorderBuffer[pos_rob].mob_ptr = &this->memory_order_buffer_write.back();
			this->make_memory_dependencies(&this->memory_order_buffer_read.back());
        }
        //====================
        // Settign status
        //====================
        this->update_registers(&this->reorderBuffer[pos_rob]);
        //insert pointer to unified Reservoir station
        this->unified_reservation_station.push_back(&this->reorderBuffer[pos_rob]);
    #if RENAME_DEBUG
        ORCS_PRINTF("Rename %s\n",this->reorderBuffer[pos_rob].content_to_string().c_str())
    #endif
    } //end for
 
} //end method
*/
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
	for (uint32_t i = 0; i < this->unified_reservation_station.size() && i < UNIFIED_RS; i++){
		//pointer to entry
		reorder_buffer_line_t *rob_line = this->unified_reservation_station[i];
		#if DISPATCH_DEBUG
			ORCS_PRINTF("cycle %lu\n",orcs_engine.get_global_cycle())
			ORCS_PRINTF("Trying Dispatch %s\n",rob_line->content_to_string().c_str())
		#endif
		if (total_dispatched>=PROCESSOR_STAGE_DISPATCH){
			break;
		}
		if((rob_line->uop.readyAt <= orcs_engine.get_global_cycle())&&
			(rob_line->wait_reg_deps_number == 0)){
			ERROR_ASSERT_PRINTF(rob_line->uop.status == PACKAGE_STATE_READY,"Error, uop not ready being dispatched")
			ERROR_ASSERT_PRINTF(rob_line->stage == PROCESSOR_STAGE_RENAME,"Error, uop not in Rename to rename stage")
			// ERROR_ASSERT_PRINTF(rob_line->wait_reg_deps_number == 0,"Error, uop with dependences not 0")

			//if dispatched
			bool dispatched = false;
			switch (rob_line->uop.uop_operation){	
				// NOP operation
				case INSTRUCTION_OPERATION_NOP:
				// integer alu// add/sub/logical
				case INSTRUCTION_OPERATION_INT_ALU:
				// branch op. como fazer, branch solved on fetch
				case INSTRUCTION_OPERATION_BRANCH:
				// op not defined
				case INSTRUCTION_OPERATION_OTHER:
					if(fu_int_alu < INTEGER_ALU){
						for (uint8_t k = 0; k < INTEGER_ALU; k++){
							if(this->fu_int_alu[k]<=orcs_engine.get_global_cycle()){
								//Branch instruction, ????
								if(rob_line->uop.uop_operation == INSTRUCTION_OPERATION_BRANCH){
									//do something
								}
							this->fu_int_alu[k]= orcs_engine.get_global_cycle()+WAIT_NEXT_INT_ALU;
							fu_int_alu++;
							dispatched=true;
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
					if(fu_int_mul < INTEGER_MUL){
						for(uint8_t k = 0;k < INTEGER_MUL;k++){
							if(this->fu_int_mul[k]<=orcs_engine.get_global_cycle()){
								this->fu_int_mul[k]=orcs_engine.get_global_cycle()+WAIT_NEXT_INT_MUL;
								fu_int_mul++;
								dispatched=true;
								rob_line->stage=PROCESSOR_STAGE_EXECUTION;
								rob_line->uop.updatePackageReady(LATENCY_INTEGER_MUL);
								break;
							}
						}
					}
				break;
				// ====================================================
				// Integer division
				case INSTRUCTION_OPERATION_INT_DIV:
					if(fu_int_div < INTEGER_DIV){
						for(uint8_t k = 0;k < INTEGER_DIV;k++){
							if(this->fu_int_div[k]<=orcs_engine.get_global_cycle()){
								this->fu_int_div[k]=orcs_engine.get_global_cycle()+WAIT_NEXT_INT_DIV;
								fu_int_div++;
								dispatched=true;
								rob_line->stage=PROCESSOR_STAGE_EXECUTION;
								rob_line->uop.updatePackageReady(LATENCY_INTEGER_DIV);
								break;
							}
						}
					}
				break;
				// ====================================================
				// Floating point ALU operation
				case INSTRUCTION_OPERATION_FP_ALU:
					if(fu_fp_alu < FP_ALU){
						for(uint8_t k = 0;k < FP_ALU;k++){
							if(this->fu_fp_alu[k]<=orcs_engine.get_global_cycle()){
								this->fu_fp_alu[k]=orcs_engine.get_global_cycle()+WAIT_NEXT_FP_ALU;
								fu_fp_alu++;
								dispatched=true;
								rob_line->stage=PROCESSOR_STAGE_EXECUTION;
								rob_line->uop.updatePackageReady(LATENCY_FP_ALU);
								break;
							}
						}
					}
				break;
				// ====================================================
				// Floating Point Multiplication
				case INSTRUCTION_OPERATION_FP_MUL:
					if(fu_fp_mul < FP_MUL){
						for(uint8_t k = 0;k < FP_MUL;k++){
							if(this->fu_fp_mul[k]<=orcs_engine.get_global_cycle()){
								this->fu_fp_mul[k]=orcs_engine.get_global_cycle()+WAIT_NEXT_FP_MUL;
								fu_fp_mul++;
								dispatched=true;
								rob_line->stage=PROCESSOR_STAGE_EXECUTION;
								rob_line->uop.updatePackageReady(LATENCY_FP_MUL);
								break;
							}
						}
					}
				break;
				
				// ====================================================
				// Floating Point Division
				case INSTRUCTION_OPERATION_FP_DIV:
					if(fu_fp_div < FP_DIV){
						for(uint8_t k= 0;k < FP_DIV;k++){
							if(this->fu_fp_div[k]<=orcs_engine.get_global_cycle()){
								this->fu_fp_div[k]=orcs_engine.get_global_cycle()+WAIT_NEXT_FP_DIV;
								fu_fp_div++;
								dispatched=true;
								rob_line->stage=PROCESSOR_STAGE_EXECUTION;
								rob_line->uop.updatePackageReady(LATENCY_FP_DIV);
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
								rob_line->stage=PROCESSOR_STAGE_EXECUTION;
								rob_line->uop.updatePackageReady(LATENCY_MEM_LOAD);
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
								rob_line->stage=PROCESSOR_STAGE_EXECUTION;
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
			}//end switch
			//remover os postos em execucao aqui
			if(dispatched == true){
				#if DISPATCH_DEBUG
					ORCS_PRINTF("Dispatched %s\n",rob_line->content_to_string().c_str())
					ORCS_PRINTF("===================================================================\n")
				#endif
				// update Dispatched
				total_dispatched++;
				// insert on FUs waiting structure
				this->unified_functional_units.push_back(rob_line);
				// remove from reservation station
				this->unified_reservation_station.erase(this->unified_reservation_station.begin()+i);
				i--;
			}//end 	if dispatched
		}//end if robline is ready
	}//end for
// sleep(1);
} //end method

void processor_t::execute(){
	uint32_t i =0;
		#if EXECUTE_DEBUG
			ORCS_PRINTF("Execute Stage\n")
		#endif   
	// ==================================
	// verificar leituras prontas no ciclo,
	// remover do MOB e atualizar os registradores, 
	// ==================================
	
	for (i=0 ; i<MOB_READ;i++){
		if(this->memory_order_buffer_read[i].status == PACKAGE_STATE_READY && 
			this->memory_order_buffer_read[i].readyAt <=orcs_engine.get_global_cycle()){
			ERROR_ASSERT_PRINTF(this->memory_order_buffer_read[i].uop_executed == true, "Removing memory read before being executed.\n")
			ERROR_ASSERT_PRINTF(this->memory_order_buffer_read[i].wait_mem_deps_number == 0, "Number of memory dependencies should be zero.\n")
			this->memory_order_buffer_read[i].rob_ptr->stage=PROCESSOR_STAGE_COMMIT;
			this->memory_order_buffer_read[i].rob_ptr->uop.updatePackageReady(COMMIT_LATENCY);
			this->memory_order_buffer_read[i].rob_ptr->mob_ptr=NULL;
			#if EXECUTE_DEBUG
			ORCS_PRINTF("Solving %s\n",this->memory_order_buffer_read[entry].rob_ptr->content_to_string().c_str())
			#endif
			// solving register dependence
			this->solve_registers_dependency(this->memory_order_buffer_read[i].rob_ptr);
			// solving memory dependency
			#if DESAMBIGUATION_ENABLED
			this->solve_memory_dependency(&this->memory_order_buffer_read[i]);
			#endif
			this->memory_order_buffer_read[i].package_clean();
		}
	}
	// =====================================	
	uint32_t uop_total_executed = 0;
	for (size_t i = 0; i < this->unified_functional_units.size(); i++){
		if(this->has_llc_miss==MISS){
			// if(this->halt_execute_chain >= orcs_engine.get_global_cycle()){
			// 	break;
			// }
		}
		reorder_buffer_line_t *rob_line = this->unified_functional_units[i];
		if(uop_total_executed == EXECUTE_WIDTH){
			break;
		}
		if(rob_line == NULL){
			break;
		}
		if(rob_line->uop.readyAt<=orcs_engine.get_global_cycle()){
			#if EXECUTE_DEBUG
				ORCS_PRINTF("Trying Execute %s\n",rob_line->content_to_string().c_str())
			#endif
			ERROR_ASSERT_PRINTF(rob_line->stage == PROCESSOR_STAGE_EXECUTION,"ROB not on execution state")
			ERROR_ASSERT_PRINTF(rob_line->uop.status == PACKAGE_STATE_READY,"FU with Package not in ready state")
			switch(rob_line->uop.uop_operation){
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
                    rob_line->uop.updatePackageReady(EXECUTE_LATENCY+COMMIT_LATENCY);
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
                    ERROR_ASSERT_PRINTF(rob_line->mob_ptr != NULL, "Read with a NULL pointer to MOB")
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
                    ERROR_ASSERT_PRINTF(rob_line->mob_ptr != NULL, "Write with a NULL pointer to MOB")
                    this->memory_write_executed++;
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
			}//end switch
			#if EXECUTE_DEBUG
				ORCS_PRINTF("Executed %s\n",rob_line->content_to_string().c_str())
			#endif
		} //end if ready package
	}//end for 
	// =========================================================================
	// Verificar se foi executado alguma operação de leitura,
	//  e executar a mais antiga no MOB
	// =========================================================================
	if(this->memory_read_executed > 0){
		this->mob_read();
	}


	// ==================================
	// Executar o MOB Write, com a escrita mais antiga.
	// depois liberar e tratar as escrita prontas;
	// ==================================
	if(this->memory_write_executed > 0){
		this->mob_write();
	}
	// =====================================
	for (i=0; i< MOB_WRITE;i++){
	if(this->memory_order_buffer_write[i].status == PACKAGE_STATE_READY && 
		this->memory_order_buffer_write[i].uop_executed == true &&
		this->memory_order_buffer_write[i].readyAt <=orcs_engine.get_global_cycle()){
		// ERROR_ASSERT_PRINTF
		this->memory_order_buffer_write[i].rob_ptr->stage=PROCESSOR_STAGE_COMMIT;
		this->memory_order_buffer_write[i].rob_ptr->uop.updatePackageReady(COMMIT_LATENCY);
		this->memory_order_buffer_write[i].rob_ptr->mob_ptr=NULL;
		#if EXECUTE_DEBUG
		ORCS_PRINTF("Solving %s\n",this->memory_order_buffer_write[entry].rob_ptr->content_to_string().c_str())
		#endif
		// solving register dependence
		this->solve_registers_dependency(this->memory_order_buffer_write[i].rob_ptr);
		// solving memory dependency
		#if DESAMBIGUATION_ENABLED
		this->solve_memory_dependency(&this->memory_order_buffer_write[i]);
		#endif
		this->memory_order_buffer_write[i].package_clean();
	}
}
} //end method

void processor_t::mob_read(){	
	
	int32_t position_mem = POSITION_FAIL;
	memory_order_buffer_line_t *mob_line = NULL; 
	#if MOB_DEBUG
	ORCS_PRINTF("MOB Read")
	#endif
	for (size_t i = 0; i < PARALLEL_LOADS; i++)
	{
		position_mem = memory_order_buffer_line_t::find_old_request_state_ready(this->memory_order_buffer_read,
							MOB_READ,PACKAGE_STATE_UNTREATED);
		if(position_mem != POSITION_FAIL){
			mob_line = &this->memory_order_buffer_read[position_mem];
		}
		if(mob_line != NULL){
				uint32_t ttc=0;
				ttc = orcs_engine.cacheManager->searchData(mob_line,&this->has_llc_miss);
				mob_line->updatePackageReady(ttc);
				mob_line->rob_ptr->uop.updatePackageReady(ttc);
			this->memory_read_executed--;
			#if MOB_DEBUG
				ORCS_PRINTF("On MOB READ Stage\n")
				ORCS_PRINTF("Time to complete READ %u\n",ttc)
				ORCS_PRINTF("MOB Line After EXECUTE %s\n",mob_line->content_to_string().c_str())
			#endif
		}//end if mob_line null
	}
	
}; //end method
void processor_t::mob_write(){
	
	int32_t position_mem = POSITION_FAIL;
	memory_order_buffer_line_t *mob_line = NULL; 
	#if MOB_DEBUG
	ORCS_PRINTF("MOB Read")
	#endif
	for (size_t i = 0; i < PARALLEL_STORES; i++)
	{
		position_mem = memory_order_buffer_line_t::find_old_request_state_ready(this->memory_order_buffer_write,
							MOB_WRITE,PACKAGE_STATE_UNTREATED);
		if(position_mem != POSITION_FAIL){
			mob_line = &this->memory_order_buffer_write[position_mem];
		}
		if(mob_line != NULL){
				uint32_t ttc=0;
				ttc = orcs_engine.cacheManager->writeData(mob_line);
				mob_line->updatePackageReady(ttc);
				mob_line->rob_ptr->uop.updatePackageReady(ttc);
		this->memory_read_executed--;
		#if MOB_DEBUG
				ORCS_PRINTF("On MOB READ Stage\n")
				ORCS_PRINTF("Time to complete READ %u\n",ttc)
				ORCS_PRINTF("MOB Line After EXECUTE %s\n",mob_line->content_to_string().c_str())
		#endif
		}//end if mob_line null
	}
	
};

void processor_t::commit(){
		#if COMMIT_DEBUG
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("Commit Stage\n")
			ORCS_PRINTF("Cycle %lu\n",orcs_engine.get_global_cycle())
			ORCS_PRINTF("Rob Head %s\n",this->reorderBuffer[this->robStart].content_to_string().c_str())
		}
		#endif
    int32_t pos_buffer;

    /// Commit the packages
    for (uint32_t i = 0 ; i < COMMIT_WIDTH ; i++) {
        pos_buffer = this->robStart;
        if (this->robUsed != 0 &&
        this->reorderBuffer[pos_buffer].stage == PROCESSOR_STAGE_COMMIT &&
        this->reorderBuffer[pos_buffer].uop.status == PACKAGE_STATE_READY &&
        this->reorderBuffer[pos_buffer].uop.readyAt <= orcs_engine.get_global_cycle()) {

            this->commit_uop_counter++;
            switch (this->reorderBuffer[pos_buffer].uop.uop_operation) {
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
				if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
				ORCS_PRINTF("Commited Instruction\n%s\n",this->reorderBuffer[this->robStart].content_to_string().c_str())
				}
			// sleep(1);
			#endif
			this->removeFrontROB();	

        }
        /// Could not commit the older, then stop looking for ready uops
        else {
            break;
        }
    }
} //end method

// ============================================================================
void processor_t::solve_registers_dependency(reorder_buffer_line_t *rob_line) {

    /// Remove pointers from Register Alias Table (RAT)
    for (uint32_t j = 0; j < MAX_REGISTERS; j++) {
        if (rob_line->uop.write_regs[j] < 0) {
            break;
        }
        uint32_t write_register = rob_line->uop.write_regs[j];
        ERROR_ASSERT_PRINTF(write_register <RAT_SIZE, "Read Register (%d) > Register Alias Table Size (%d)\n",
                                                                            write_register, RAT_SIZE);
		if (this->register_alias_table[write_register] != NULL &&
        this->register_alias_table[write_register]->uop.uop_number == rob_line->uop.uop_number) {
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
        if (rob_line->reg_deps_ptr_array[j] != NULL) {
			rob_line->wake_up_elements_counter--;
            rob_line->reg_deps_ptr_array[j]->wait_reg_deps_number--;
            /// This update the ready cycle, and it is usefull to compute the time each instruction waits for the functional unit
            if (rob_line->reg_deps_ptr_array[j]->uop.readyAt <= orcs_engine.get_global_cycle()) {
                rob_line->reg_deps_ptr_array[j]->uop.readyAt = orcs_engine.get_global_cycle();
            }
            rob_line->reg_deps_ptr_array[j] = NULL;
        }
        /// All the dependencies are solved
        else {
            break;
        }
	}
};


void processor_t::clock()
{
	#if DEBUG
	// ORCS_PRINTF("====================================================================\n")
	// ORCS_PRINTF("Cycle %lu\n",orcs_engine.get_global_cycle())
	#endif
	/////////////////////////////////////////////////
	//// Verifica se existe coisas no ROB
	//// CommitStage
	//// ExecuteStage
	//// DispatchStage
	/////////////////////////////////////////////////
	if (this->robUsed != 0)
	{	
		
		this->commit();//commit instructions -> remove from ROB
		this->execute(); //verify Uops ready on UFs, then remove
		this->dispatch();//dispath ready uops to UFs
 
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
	// ORCS_PRINTF("===================================================================\n")
	#endif
	#if PERIODIC_CHECK
		if(orcs_engine.get_global_cycle()%CLOCKS_TO_CHECK==0){
			this->printStructures();
			// ORCS_PRINTF("Opcodes Processed %lu",orcs_engine.trace_reader->get_fetch_instructions())
		}
	#endif
};
// =====================================================================
void processor_t::statistics()
{
	if(orcs_engine.output_file_name == NULL){
	utils_t::largestSeparator();
	ORCS_PRINTF("Total_Cicle : %lu\n",orcs_engine.get_global_cycle())
	utils_t::largeSeparator();
	ORCS_PRINTF("Stage_Opcode_and_Uop_Counters\n")
	utils_t::largeSeparator();
	ORCS_PRINTF("Stage_Fetch: %lu\n",this->fetchCounter)
	ORCS_PRINTF("Stage_Decode: %lu\n",this->decodeCounter)
	ORCS_PRINTF("Stage_Rename: %lu\n",this->renameCounter)
	ORCS_PRINTF("Stage_Commit: %lu\n",this->commit_uop_counter)
	utils_t::largestSeparator();
	ORCS_PRINTF("======================== MEMORY DESAMBIGUATION ===========================\n")
	utils_t::largestSeparator();
	ORCS_PRINTF("Read_False_Positive: %lu\n",this->get_stat_disambiguation_read_false_positive())
	ORCS_PRINTF("Write_False_Positive: %lu\n",this->get_stat_disambiguation_write_false_positive())
	ORCS_PRINTF("Solve_Address_to_Address: %lu\n",this->get_stat_address_to_address())
	utils_t::largestSeparator();
	ORCS_PRINTF("Instruction_Per_Cicle: %.4f\n",float(this->fetchCounter)/float(orcs_engine.get_global_cycle()))
	}
	else{
		FILE *output = fopen(orcs_engine.output_file_name,"a+");
		if(output != NULL){
			utils_t::largestSeparator(output);
			fprintf(output,"Total_Cycle_: %lu\n",orcs_engine.get_global_cycle());
			utils_t::largeSeparator(output);
			fprintf(output,"Stage_Opcode_and_Uop_Counters\n");
			utils_t::largeSeparator(output);
			fprintf(output,"Stage_Fetch: %lu\n",this->fetchCounter);
			fprintf(output,"Stage_Decode: %lu\n",this->decodeCounter);
			fprintf(output,"Stage_Rename: %lu\n",this->renameCounter);
			fprintf(output,"Stage_Commit: %lu\n",this->commit_uop_counter);
			utils_t::largestSeparator(output);
			fprintf(output,"======================== MEMORY DESAMBIGUATION ===========================\n");
			utils_t::largestSeparator(output);
			fprintf(output,"Read_False_Positive: %lu\n",this->get_stat_disambiguation_read_false_positive());
			fprintf(output,"Write_False_Positive: %lu\n",this->get_stat_disambiguation_write_false_positive());
			fprintf(output,"Solve_Address_to_Address: %lu\n",this->get_stat_address_to_address());
			utils_t::largestSeparator(output);
			fprintf(output,"Instruction_Per_Cycle: %.4f\n",float(this->fetchCounter)/float(orcs_engine.get_global_cycle()));
		}
		fclose(output);
	}
};

void processor_t::printConfiguration(){
	ORCS_PRINTF("===============Stages Width============\n")
	ORCS_PRINTF("FETCH Width %d\n",FETCH_WIDTH)
	ORCS_PRINTF("DECODE Width %d\n",DECODE_WIDTH)
	ORCS_PRINTF("RENAME Width %d\n",RENAME_WIDTH)
	ORCS_PRINTF("DISPATCH Width %d\n",DISPATCH_WIDTH)
	ORCS_PRINTF("EXECUTE Width %d\n",EXECUTE_WIDTH)
	ORCS_PRINTF("COMMIT Width %d\n",COMMIT_WIDTH)

	ORCS_PRINTF("===============Structures Sizes============\n")
	ORCS_PRINTF("Fetch Buffer ->%u\n",this->fetchBuffer.get_capacity())
	ORCS_PRINTF("Decode Buffer ->%u\n",this->decodeBuffer.get_capacity())
	ORCS_PRINTF("RAT ->%u\n",RAT_SIZE)
	ORCS_PRINTF("ROB ->%u\n",ROB_SIZE)
	ORCS_PRINTF("MOB Read ->%u\n",MOB_READ)
	ORCS_PRINTF("MOB Write->%u\n",MOB_WRITE)
	ORCS_PRINTF("Reservation Station->%lu\n",this->unified_reservation_station.max_size())
	ORCS_PRINTF("Funcional Units->%lu\n",this->unified_functional_units.max_size())

	sleep(5);

}
void processor_t::printStructures(){
	ORCS_PRINTF("Periodic Check -  Structures at %lu\n",orcs_engine.get_global_cycle())
	utils_t::largestSeparator();
	ORCS_PRINTF("Front end Buffers\n")
	utils_t::largeSeparator();
	ORCS_PRINTF("Fetch Buffer ==> %u\n",this->fetchBuffer.get_size())
	utils_t::smallSeparator();
	ORCS_PRINTF("DecodeBuffer ==> %u\n",this->decodeBuffer.get_size())
	utils_t::largestSeparator();
	ORCS_PRINTF("ROB and MOB usage\n")
	ORCS_PRINTF("ROB used %u of %u \n", this->robUsed, ROB_SIZE)
	utils_t::largeSeparator();
	// ORCS_PRINTF("MOB Read used %lu of %u \n", this->memory_order_buffer_read.size(), MOB_READ)
	// utils_t::largeSeparator();
	// ORCS_PRINTF("MOB Write used %lu of %u \n", this->memory_order_buffer_write.size(), MOB_WRITE)
	utils_t::largestSeparator();
	ORCS_PRINTF("Dispatch and execute usage\n")
	utils_t::smallSeparator();
	ORCS_PRINTF("Dispatch Use: %lu\n",this->unified_reservation_station.size())
	utils_t::smallSeparator();
	ORCS_PRINTF("FUs Use: %lu\n",this->unified_functional_units.size())
	utils_t::largestSeparator();
	ORCS_PRINTF("Commit Status uOPs\n")
	utils_t::largeSeparator();
	// ==============================================================
	ORCS_PRINTF("INSTRUCTION_OPERATION_INT_ALU %lu\n",this->get_stat_inst_int_alu_completed())
	utils_t::largeSeparator();
	ORCS_PRINTF("INSTRUCTION_OPERATION_INT_MUL %lu\n",this->get_stat_inst_mul_alu_completed())
	utils_t::largeSeparator();
	ORCS_PRINTF("INSTRUCTION_OPERATION_INT_DIV %lu\n",this->get_stat_inst_div_alu_completed())
	utils_t::largeSeparator();
	ORCS_PRINTF("INSTRUCTION_OPERATION_FP_ALU %lu\n",this->get_stat_inst_int_fp_completed())
	utils_t::largeSeparator();
	ORCS_PRINTF("INSTRUCTION_OPERATION_FP_MUL %lu\n",this->get_stat_inst_mul_fp_completed())
	utils_t::largeSeparator();
	ORCS_PRINTF("INSTRUCTION_OPERATION_FP_DIV %lu\n",this->get_stat_inst_div_fp_completed())
	utils_t::largeSeparator();
	ORCS_PRINTF("INSTRUCTION_OPERATION_MEM_LOAD %lu\n",this->get_stat_inst_load_completed())
	utils_t::largeSeparator();
	ORCS_PRINTF("INSTRUCTION_OPERATION_MEM_STORE %lu\n",this->get_stat_inst_store_completed())
	utils_t::largeSeparator();
	ORCS_PRINTF("INSTRUCTION_OPERATION_BRANCH %lu\n",this->get_stat_inst_branch_completed())
	utils_t::largeSeparator();
	ORCS_PRINTF("INSTRUCTION_OPERATION_NOP %lu\n",this->get_stat_inst_nop_completed())
	utils_t::largeSeparator();
	ORCS_PRINTF("INSTRUCTION_OPERATION_OTHER %lu\n",this->get_stat_inst_other_completed())
	utils_t::largeSeparator();
	ORCS_PRINTF("\n\n\n")
	// ==============================================================
	sleep(2);
}
// ==============================================================
// Debug aid to identify where is teh problems
// ==============================================================