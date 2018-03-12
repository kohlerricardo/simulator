#include "../simulator.hpp"

// =====================================================================
processor_t::processor_t()
{
	//Setting Pointers to NULL
	// ========MOB======
	this->memory_order_buffer_read = NULL;
	this->memory_order_buffer_write = NULL;
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
processor_t::~processor_t()
{
	//NULLing Pointers
	//deleting MOB read and MOB write
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
	this->set_stall_wrong_branch(0);
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
	// Memory Order Buffer Read
	this->memory_order_buffer_read = utils_t::template_allocate_array<memory_order_buffer_line_t>(MOB_READ);
	// Memory Order Buffer Write
	this->memory_order_buffer_write = utils_t::template_allocate_array<memory_order_buffer_line_t>(MOB_WRITE);
	// reserving space to reservation station uops
	this->unified_reservation_station.reserve(UNIFIED_RS);
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
	this->unified_reservation_station.reserve(UNIFIED_RS);
	// reserving space to uops on UFs pipeline, waitng to executing ends
	this->unified_functional_units.reserve(ROB_SIZE);
};
bool processor_t::isBusy()
{
	return (this->traceIsOver == false ||
			!this->fetchBuffer.is_empty() ||
			!this->decodeBuffer.is_empty() ||
			this->robUsed != 0);
};
// =====================================================================
void processor_t::fetch()
{
	opcode_package_t operation;
	// Trace ->fetchBuffer
	for (int i = 0; i < FETCH_WIDTH; i++)
	{
		operation.package_clean();

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
		operation.status = PACKAGE_STATE_UNTREATED;
		//============================
		///Solve Branch
		//============================
		if (hasBranch)
		{
			//solve
			uint32_t stallWrongBranch = orcs_engine.branchPredictor->solveBranch(this->previousBranch, operation);
			this->set_stall_wrong_branch(orcs_engine.get_global_cycle() + stallWrongBranch);
			if (POSITION_FAIL == this->fetchBuffer.push_back(this->previousBranch))
			{
				break;
			}
			hasBranch = false;
			this->fetchBuffer.back()->updatePackageReady(FETCH_LATENCY);
			this->previousBranch.package_clean();
		}
		//============================
		// Operation Branch, set flag
		//============================
		if (operation.opcode_operation == INSTRUCTION_OPERATION_BRANCH)
		{
			orcs_engine.branchPredictor->branches++;
			this->previousBranch = operation;
			hasBranch = true;
			continue;
		}
		//============================
		//Insert into fetch buffer
		//============================
		if (POSITION_FAIL == this->fetchBuffer.push_back(operation))
		{
			break;
		}
		this->fetchBuffer.back()->updatePackageReady(FETCH_LATENCY);
	}
	//============================
	//Atualiza status dos pacotes
	// Ready At
	//============================
}
/**
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
void processor_t::decode()
{
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
		ERROR_ASSERT_PRINTF(this->decodeCounter == this->fetchBuffer.front()->opcode_number, "Error On decode");
		this->decodeCounter++;
		// ORCS_PRINTF("OPCODE DECODED %s\n",this->fetchBuffer.front()->content_to_string().c_str())
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
				//limpa regs
				for (uint32_t i = 0; i < MAX_REGISTERS; i++)
				{
					new_uop.read_regs[i] = POSITION_FAIL;
					new_uop.write_regs[i] = POSITION_FAIL;
				}
				//insert regs refs
				new_uop.read_regs[0] = this->fetchBuffer.front()->base_reg;
				new_uop.read_regs[1] = this->fetchBuffer.front()->index_reg;
				//insert write
				new_uop.write_regs[0] = 258;
			}
			new_uop.updatePackageReady(DECODE_LATENCY);
			// printf("\n UOP Created %s \n",new_uop.content_to_string().c_str());
			statusInsert = this->decodeBuffer.push_back(new_uop);
			// assert((statusInsert != POSITION_FAIL) && "Erro, Tentando decodificar mais uops que o maximo permitido");
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
				//limpa regs
				for (uint32_t i = 0; i < MAX_REGISTERS; i++)
				{
					new_uop.read_regs[i] = POSITION_FAIL;
					new_uop.write_regs[i] = POSITION_FAIL;
				}
				//insert regs refs
				new_uop.read_regs[0] = this->fetchBuffer.front()->base_reg;
				new_uop.read_regs[1] = this->fetchBuffer.front()->index_reg;
				//insert write
				new_uop.write_regs[0] = 258;
			}
			new_uop.updatePackageReady(DECODE_LATENCY);
			// printf("\n UOP Created %s \n",new_uop.content_to_string().c_str());
			statusInsert = this->decodeBuffer.push_back(new_uop);
			// assert((statusInsert != POSITION_FAIL) && "Erro, Tentando decodificar mais uops que o maximo permitido");
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
				//registers /258 aux onde pos[i] = fail
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
				ERROR_ASSERT_PRINTF(inserted_258, "Todos Max regs usados.%u\n", MAX_REGISTERS)
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
			// printf("\n UOP Created %s \n",new_uop.content_to_string().c_str());
			statusInsert = this->decodeBuffer.push_back(new_uop);
			assert((statusInsert != POSITION_FAIL) && "Erro, Tentando decodificar mais uops que o maximo permitido");
			// ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL,"Erro, Tentando decodificar mais uops que o maximo permitido")
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
			// assert((statusInsert != POSITION_FAIL) && "Erro, Tentando decodificar mais uops que o maximo permitido");
			ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL, "Erro, Tentando decodificar mais uops que o maximo permitido")
		}
		this->fetchBuffer.pop_front();
	}
};

// ============================================================================
void processor_t::update_registers(reorder_buffer_line_t *robLine)
{
	//Register to READ
	// ORCS_PRINTF("%s\n",robLine->content_to_string().c_str())
	for (uint32_t i = 0; i < MAX_REGISTERS; i++)
	{
		if (robLine->uop.read_regs[i] < 0)
		{
			break;
		}
		uint32_t read_register = robLine->uop.read_regs[i];
		ERROR_ASSERT_PRINTF(read_register < RAT_SIZE, "Read Register (%d) > Register Alias Table Size (%d)\n", read_register, RAT_SIZE);
		if (this->register_alias_table[read_register] != NULL)
		{
			// ORCS_PRINTF("Renaming -> %u\n", read_register)
			for (uint32_t j = 0; j < ROB_SIZE; j++)
			{
				if (this->register_alias_table[read_register]->reg_deps_ptr_array[j] == NULL)
				{
					// ORCS_PRINTF("position %u renamed \n",j)
					this->register_alias_table[read_register]->reg_deps_ptr_array[j] = robLine;
					robLine->wait_reg_deps_number++;
					break;
				}
			}
		}
	}
	// =============================
	// OK
	// =============================
	/// Control the Register Dependency - Register WRITE
	for (uint32_t k = 0; k < MAX_REGISTERS; k++)
	{
		if (robLine->uop.write_regs[k] < 0)
		{
			break;
		}
		uint32_t write_register = robLine->uop.write_regs[k];
		// ORCS_PRINTF("Renameing %u\n",write_register)
		ERROR_ASSERT_PRINTF(write_register < RAT_SIZE, "Write Register (%d) > Register Alias Table Size (%d)\n", write_register, RAT_SIZE);

		this->register_alias_table[write_register] = robLine;
	}
};
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
void processor_t::removeFrontROB()
{
	ERROR_ASSERT_PRINTF(this->robUsed > 0, "Removendo do ROB sem estar usado")
	ERROR_ASSERT_PRINTF(this->reorderBuffer[this->robStart].reg_deps_ptr_array == NULL, "Removendo sem resolver dependencias")
	this->reorderBuffer[this->robStart].package_clean();
	this->robUsed--;
	this->robStart++;
	if (this->robStart >= ROB_SIZE)
	{
		this->robStart = 0;
	}
};
void processor_t::rename()
{
	size_t i;
	int32_t pos_mob, pos_rob;
	reorder_buffer_line_t robEntry;
	for (i = 0; i < RENAME_WIDTH; i++)
	{
		// Checando se há uop decodificado, se está pronto, e se o ciclo de pronto
		// é maior ou igual ao atual
		if (this->decodeBuffer.is_empty() ||
			this->decodeBuffer.front()->status != PACKAGE_STATE_READY ||
			this->decodeBuffer.front()->readyAt > orcs_engine.get_global_cycle())
		{
			break;
		}
		ERROR_ASSERT_PRINTF(this->decodeBuffer.front()->uop_number == this->renameCounter, "Erro, renomeio incorreto\n")
		memory_order_buffer_line_t *mob_line = NULL;
		//=======================
		// Memory Operation Read
		//=======================
		if (this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_MEM_LOAD)
		{
			pos_mob = memory_order_buffer_line_t::find_free(this->memory_order_buffer_read, MOB_READ);
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
		this->reorderBuffer[pos_rob].uop = *this->decodeBuffer.front();
		//remove uop from decodebuffer
		this->decodeBuffer.front()->package_clean();
		this->decodeBuffer.pop_front();
		//////////////////
		//add counter renamed uops
		this->renameCounter++;
		// =======================
		// Setting controls to ROB.
		// =======================
		this->reorderBuffer[pos_rob].stage = PROCESSOR_STAGE_RENAME;
		this->reorderBuffer[pos_rob].mob_ptr = mob_line;
		this->reorderBuffer[pos_rob].uop.updatePackageReady(RENAME_LATENCY);
		//insert pointer to unified Reservoir station
		// acertar as dependencias de registradores
		// =======================
		// Insert into MOB.
		// =======================
		if (this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD)
		{
			mob_line->opcode_address = robEntry.uop.opcode_address;
			mob_line->memory_address = robEntry.uop.memory_address;
			mob_line->memory_size = robEntry.uop.memory_size;
			mob_line->memory_operation = MEMORY_OPERATION_READ;
			mob_line->status = PACKAGE_STATE_UNTREATED;
			mob_line->rob_ptr = &this->reorderBuffer[pos_rob];
		}
		else if (this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_MEM_STORE)
		{
			mob_line->opcode_address = robEntry.uop.opcode_address;
			mob_line->memory_address = robEntry.uop.memory_address;
			mob_line->memory_size = robEntry.uop.memory_size;
			mob_line->memory_operation = MEMORY_OPERATION_WRITE;
			mob_line->status = PACKAGE_STATE_UNTREATED;
			mob_line->rob_ptr = &this->reorderBuffer[pos_rob];
		}
		//====================
		// Settign status
		//====================
		this->update_registers(&this->reorderBuffer[pos_rob]);
		if (this->reorderBuffer[pos_rob].wait_reg_deps_number == 0)
		{
			// ORCS_PRINTF("inserido no Reservation Station\n%s\n",this->reorderBuffer[pos_rob].content_to_string().c_str())
			this->reorderBuffer[pos_rob].uop.updatePackageReady(DISPATCH_LATENCY);
			this->reorderBuffer[pos_rob].stage = PROCESSOR_STAGE_DISPATCH;
			this->unified_reservation_station.push_back(&this->reorderBuffer[pos_rob]);
		}
	} //end for

} //end method
void processor_t::dispatch()
{
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

	ORCS_PRINTF("Size %lu\n", this->unified_reservation_station.size())
	ORCS_PRINTF("capacidade %lu\n", this->unified_reservation_station.capacity())

	for (uint32_t i = 0; i < this->unified_reservation_station.size() && UNIFIED_RS; i++){
		//pointer to entry
		reorder_buffer_line_t *rob_line = this->unified_reservation_station[i];
		if (rob_line == NULL){
			this->add_stall_empty_RS();
			break;
		}
		if (total_dispatched>=PROCESSOR_STAGE_DISPATCH){
			break;
		}
		if(rob_line->uop.readyAt <= orcs_engine.get_global_cycle()){
			ERROR_ASSERT_PRINTF(rob_line->wait_reg_deps_number == 0,"Error, Instruction with dependencies not solved")
			ERROR_ASSERT_PRINTF(rob_line->uop.status == PACKAGE_STATE_READY,"Error, uop not ready being dispatched")
			ERROR_ASSERT_PRINTF(rob_line->stage == PROCESSOR_STAGE_DISPATCH,"Error, uop not in dispach stage")

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
				//
					if(fu_int_alu <=INTEGER_ALU){
						for (size_t i = 0; i < INTEGER_ALU; i++){
							if(this->fu_int_alu[i]<=orcs_engine.get_global_cycle()){
								//Branch instruction, ????
								if(rob_line->uop.uop_operation == INSTRUCTION_OPERATION_BRANCH){
									//do something
								}
							this->fu_int_alu[i]= orcs_engine.get_global_cycle()+WAIT_NEXT_INT_ALU;
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
				break;
				// ====================================================
				// Integer division
				case INSTRUCTION_OPERATION_INT_DIV:
				break;
				// ====================================================
				// Floating point ALU operation
				case INSTRUCTION_OPERATION_FP_ALU:
				break;
				// ====================================================
				// Floating Point Multiplication
				case INSTRUCTION_OPERATION_FP_MUL:
				break;
				
				// ====================================================
				// Floating Point Division
				case INSTRUCTION_OPERATION_FP_DIV:
				break;
				// ====================================================
				// Operation LOAD
				case INSTRUCTION_OPERATION_MEM_LOAD:
				break;
			
				// ====================================================
				// Operation STORE
				case INSTRUCTION_OPERATION_MEM_STORE:
				break;

				// ====================================================
			
			}
		}
	}

} //end method

void processor_t::execute()
{

} //end method

void processor_t::mob()
{

} //end method

void processor_t::commit()
{

} //end method

void processor_t::clock()
{
	/////////////////////////////////////////////////
	//// Verifica se existe coisas no ROB
	//// CommitStage
	//// ExecuteStage
	//// DispatchStage
	/////////////////////////////////////////////////
	if (this->robUsed != 0)
	{
		this->commit();
		this->execute();
		this->mob();
		if (this->unified_reservation_station.size() > 0)
		{ //verificar size para evitar chamar se nao existir uops a serem despachados
			this->dispatch();
		}
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
};
// =====================================================================
void processor_t::statistics()
{

	std::cout << "######################################################\n"
			  << std::endl;
	std::cout << "Total Cicle ;" << orcs_engine.get_global_cycle() << std::endl;
};