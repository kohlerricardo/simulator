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
	this->memory_order_buffer_read_used = 0; 
	this->memory_order_buffer_read_start = 0; 
	this->memory_order_buffer_read_end = 0; 
	// Memory Order Buffer Write
	this->memory_order_buffer_write = utils_t::template_allocate_array<memory_order_buffer_line_t>(MOB_WRITE);
	this->memory_order_buffer_write_used = 0;
	this->memory_order_buffer_write_start = 0;
	this->memory_order_buffer_write_end = 0;
	// reserving space to reservation station uops
	this->unified_reservation_station.reserve(UNIFIED_RS);
	//reserving space to uops in execution
	this->unified_functional_units.reserve(ROB_SIZE);
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
void processor_t::removeFrontROB(){
	ERROR_ASSERT_PRINTF(this->robUsed > 0, "Removendo do ROB sem estar usado\n")
	ERROR_ASSERT_PRINTF(this->reorderBuffer[this->robStart].reg_deps_ptr_array == NULL, "Removendo sem resolver dependencias\n")
	this->reorderBuffer[this->robStart].package_clean();
	this->robUsed--;
	this->robStart++;
	if (this->robStart >= ROB_SIZE)
	{
		this->robStart = 0;
	}
};
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
void processor_t::rename(){
	size_t i;
	int32_t pos_mob, pos_rob;
	for (i = 0; i < RENAME_WIDTH;i++)
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
			pos_mob = this->search_position_mob_read();
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
			pos_mob = this->search_position_mob_write();
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
		ERROR_ASSERT_PRINTF(this->reorderBuffer[pos_rob].uop.uop_number == this->renameCounter,"Error, Renaming Out of order")
		this->renameCounter++;
		// =======================
		// Setting controls to ROB.
		// =======================
		this->reorderBuffer[pos_rob].stage = PROCESSOR_STAGE_RENAME;
		this->reorderBuffer[pos_rob].uop.updatePackageReady(RENAME_LATENCY);
		//insert pointer to unified Reservoir station
		// acertar as dependencias de registradores
		// =======================
		// Insert into MOB.
		// =======================
		if (this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD)
		{	
			// ORCS_PRINTF("Mem Load\n")
			mob_line->opcode_address = this->reorderBuffer[pos_rob].uop.opcode_address;
			mob_line->memory_address = this->reorderBuffer[pos_rob].uop.memory_address;
			mob_line->memory_size = this->reorderBuffer[pos_rob].uop.memory_size;
			mob_line->memory_operation = MEMORY_OPERATION_READ;
			mob_line->status = PACKAGE_STATE_UNTREATED;
			mob_line->born_cicle = orcs_engine.get_global_cycle();
			mob_line->rob_ptr = &this->reorderBuffer[pos_rob];
		}
		else if (this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_MEM_STORE)
		{
			// ORCS_PRINTF("Mem Store\n")
			mob_line->opcode_address = this->reorderBuffer[pos_rob].uop.opcode_address;
			mob_line->memory_address = this->reorderBuffer[pos_rob].uop.memory_address;
			mob_line->memory_size = this->reorderBuffer[pos_rob].uop.memory_size;
			mob_line->memory_operation = MEMORY_OPERATION_WRITE;
			mob_line->status = PACKAGE_STATE_UNTREATED;
			mob_line->born_cicle = orcs_engine.get_global_cycle();
			mob_line->rob_ptr = &this->reorderBuffer[pos_rob];
		}
		this->reorderBuffer[pos_rob].mob_ptr = mob_line;

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
				//printing debug
		// ORCS_PRINTF("Rename %s\n",this->reorderBuffer[pos_rob].content_to_string().c_str())
		// sleep(1);
	} //end for

} //end method
void processor_t::dispatch(){
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
	for (uint32_t i = 0; i < this->unified_reservation_station.size(); i++){
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
			switch (rob_line->uop.uop_operation){	
				// NOP operation
				case INSTRUCTION_OPERATION_NOP:
				// integer alu// add/sub/logical
				case INSTRUCTION_OPERATION_INT_ALU:
				// branch op. como fazer, branch solved on fetch
				case INSTRUCTION_OPERATION_BRANCH:
				// op not defined
				case INSTRUCTION_OPERATION_OTHER:
					if(fu_int_alu <=INTEGER_ALU){
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
						for(uint8_t k = 0;k<INTEGER_MUL;k++){
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
						for(uint8_t k = 0;k<INTEGER_DIV;k++){
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
						for(uint8_t k = 0;k<FP_ALU;k++){
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
						for(uint8_t k = 0;k<FP_MUL;k++){
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
						for(uint8_t k= 0;k<FP_DIV;k++){
							if(this->fu_fp_div[i]<=orcs_engine.get_global_cycle()){
								this->fu_fp_div[i]=orcs_engine.get_global_cycle()+WAIT_NEXT_FP_DIV;
								fu_fp_div++;
								dispatched=true;
								rob_line->stage=PROCESSOR_STAGE_EXECUTION;
								rob_line->uop.updatePackageReady(LATENCY_FP_DIV);
								break;
							}
						}
					}
				break;
				// ====================================================this->unified_reservation_station.begin()+i
				// Operation LOAD
				case INSTRUCTION_OPERATION_MEM_LOAD:
					if(fu_mem_load < LOAD_UNIT){
						for(uint8_t k= 0;k<LOAD_UNIT;k++){
							if(this->fu_mem_load[i]<=orcs_engine.get_global_cycle()){
								this->fu_mem_load[i]=orcs_engine.get_global_cycle()+WAIT_NEXT_MEM_LOAD;
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
						for(uint8_t k= 0;k<STORE_UNIT;k++){
							if(this->fu_mem_store[i]<=orcs_engine.get_global_cycle()){
								this->fu_mem_store[i]=orcs_engine.get_global_cycle()+WAIT_NEXT_MEM_STORE;
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
				// update Dispatched
				total_dispatched++;
				// insert on FUs waiting structure
				this->unified_functional_units.push_back(rob_line);
				// remove from reservation station
				// ORCS_PRINTF("%s\n",rob_line->content_to_string().c_str())
				// ORCS_PRINTF("i position dispached %u\n",i)
				this->unified_reservation_station.erase(this->unified_reservation_station.begin()+i);
				i--;
			}//end 	if dispatched
		}//end if robline is ready
	}//end for
// sleep(1);
} //end method

void processor_t::execute()
{
	// ================
	// Resolving MOB Ready uops
	// if(this->memory_read_received !=0){
	// 	for (uint32_t i = this->memory_order_buffer_read_start; i < this->memory_order_buffer_read_end; i++){
	// 		if(this->memory_order_buffer_read[i].status == PACKAGE_STATE_READY &&
	// 			this->memory_order_buffer_read[i].readyAt <= orcs_engine.get_global_cycle()){
	// 				ERROR_ASSERT_PRINTF(this->memory_order_buffer_read[i].uop_executed == true, "Removing memory read before being executed.\n")
	// 				//Solve from ROB and send to commit stage
	// 				this->memory_order_buffer_read[i].rob_ptr->stage = PROCESSOR_STAGE_COMMIT;
	// 				this->memory_order_buffer_read[i].rob_ptr->uop.updatePackageReady(COMMIT_LATENCY);
	// 				this->memory_order_buffer_read[i].rob_ptr->mob_ptr = NULL;
	// 				this->solve_registers_dependency(this->memory_order_buffer_read[i].rob_ptr);
	// 				this->memory_order_buffer_read[i].package_clean();
	// 				this->remove_front_mob_read();
	// 				this->memory_read_received--;
	// 		}
	// 	}
	// }
	// ================
	//send to commit the uops ready
	uint32_t uop_total_executed = 0;
	for (size_t i = 0; i < this->unified_functional_units.size(); i++){
		reorder_buffer_line_t *rob_line = this->unified_functional_units[i];
		if(uop_total_executed == EXECUTE_WIDTH){
			break;
		}
		if(rob_line->uop.readyAt<=orcs_engine.get_global_cycle()){
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
                // INTEGERS ==================================================
                    rob_line->stage = PROCESSOR_STAGE_COMMIT;
                    rob_line->uop.updatePackageReady(COMMIT_LATENCY);
                    this->solve_registers_dependency(rob_line);
                    uop_total_executed++;
                    /// Remove from the Functional Units
                    this->unified_functional_units.erase(this->unified_functional_units.begin() + i);
                    i--;
                break;
				// MEMORY LOAD/STORE ==========================================
                /// FUNC_UNIT_MEM_LOAD => READ_BUFFER
                case INSTRUCTION_OPERATION_MEM_LOAD:
                {
                    ERROR_ASSERT_PRINTF(rob_line->mob_ptr != NULL, "Read with a NULL pointer to MOB")
                    this->memory_read_executed++;
					rob_line->mob_ptr->uop_executed = true;
                    /// Waits for the cache to send the answer
                    rob_line->uop.status = PACKAGE_STATE_WAIT;
					
                    rob_line->uop.updatePackageReady(EXECUTE_LATENCY);
                    uop_total_executed++;
                    /// Remove from the Functional Units
                    this->unified_functional_units.erase(this->unified_functional_units.begin() + i);
                    i--;
                }
                break;
                /// FUNC_UNIT_MEM_STORE => WRITE_BUFFER
                case INSTRUCTION_OPERATION_MEM_STORE:
                {
                    ERROR_ASSERT_PRINTF(rob_line->mob_ptr != NULL, "Write with a NULL pointer to MOB")
                    this->memory_write_executed++;
					rob_line->mob_ptr->uop_executed = true;
                    /// Waits for the cache to receive the package
                    rob_line->uop.status = PACKAGE_STATE_WAIT;
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
		} //end if ready package
	}//end for 

} //end method

void processor_t::mob()
{	
	uint32_t memory_op_executed=0;

	if(this->memory_read_executed!=0){
		for (uint32_t i=this->memory_order_buffer_read_start; i < this->memory_order_buffer_read_end; i++){
			if(memory_op_executed == PARALLEL_LOADS){
				break;
			}
			if(this->memory_order_buffer_read[this->memory_order_buffer_read_start].uop_executed == true &&
				this->memory_order_buffer_read[this->memory_order_buffer_read_start].rob_ptr->uop.readyAt<=orcs_engine.get_global_cycle()){
				ORCS_PRINTF("address read-> %lu\n",this->memory_order_buffer_read[this->memory_order_buffer_read_start].memory_address)
				uint32_t ttc = orcs_engine.cacheManager->searchData(this->memory_order_buffer_read[this->memory_order_buffer_read_start].memory_address);
				this->memory_order_buffer_read[this->memory_order_buffer_read_start].rob_ptr->uop.updatePackageReady(ttc);
				this->memory_order_buffer_read[this->memory_order_buffer_read_start].rob_ptr->stage=PROCESSOR_STAGE_COMMIT;
				this->memory_order_buffer_read[this->memory_order_buffer_read_start].rob_ptr->mob_ptr=NULL;
				this->solve_registers_dependency(this->memory_order_buffer_read[this->memory_order_buffer_read_start].rob_ptr);
				this->remove_front_mob_read();
				// i--;
				this->memory_read_executed--;
				memory_op_executed++;
			}
		}//end for read
	}

	if(this->memory_write_executed !=0){
		for (size_t i = 0; i < PARALLEL_STORES; i++){
			if(this->memory_order_buffer_write_used==0){
				break;
			}
			if(this->memory_order_buffer_write[this->memory_order_buffer_write_start].uop_executed == true &&
				this->memory_order_buffer_write[this->memory_order_buffer_write_start].rob_ptr->uop.readyAt<=orcs_engine.get_global_cycle()){
			ORCS_PRINTF("address write -> %lu\n",this->memory_order_buffer_write[this->memory_order_buffer_write_start].memory_address)
			uint32_t ttc = orcs_engine.cacheManager->writeData(this->memory_order_buffer_write[this->memory_order_buffer_write_start].memory_address);
			this->memory_order_buffer_write[this->memory_order_buffer_write_start].rob_ptr->uop.updatePackageReady(ttc);
			this->memory_order_buffer_write[this->memory_order_buffer_write_start].rob_ptr->stage=PROCESSOR_STAGE_COMMIT;
			this->memory_order_buffer_write[this->memory_order_buffer_write_start].rob_ptr->mob_ptr=NULL;
			this->solve_registers_dependency(this->memory_order_buffer_write[this->memory_order_buffer_write_start].rob_ptr);
			this->remove_front_mob_write();
			this->memory_write_executed--;
			}
		}//end for write
	}
} //end method

void processor_t::commit()
{
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
                    // this->add_stat_int_alu_completed();
                break;

                // INTEGERS MUL
                case INSTRUCTION_OPERATION_INT_MUL:
                    // this->add_stat_int_mul_completed();
                break;

                // INTEGERS DIV
                case INSTRUCTION_OPERATION_INT_DIV:
                    // this->add_stat_int_div_completed();
                break;

                // FLOAT POINT ALU
                case INSTRUCTION_OPERATION_FP_ALU:
                    // this->add_stat_fp_alu_completed();
                break;

                // FLOAT POINT MUL
                case INSTRUCTION_OPERATION_FP_MUL:
                    // this->add_stat_fp_mul_completed();
                break;

                // FLOAT POINT DIV
                case INSTRUCTION_OPERATION_FP_DIV:
                    // this->add_stat_fp_div_completed();
                break;

                // MEMORY OPERATIONS - READ
                case INSTRUCTION_OPERATION_MEM_LOAD:
                    // this->add_stat_memory_read_completed(this->reorder_buffer[pos_buffer].uop.born_cycle);
                break;
                // MEMORY OPERATIONS - WRITE
                case INSTRUCTION_OPERATION_MEM_STORE:
                    // this->add_stat_memory_write_completed(this->reorder_buffer[pos_buffer].uop.born_cycle);
                break;
                // BRANCHES
                case INSTRUCTION_OPERATION_BRANCH:
                    /// Solve the Branch Prediction
                    // this->solve_branch(this->reorder_buffer[pos_buffer].uop.opcode_number, PROCESSOR_STAGE_COMMIT, this->reorder_buffer[pos_buffer].uop.uop_operation);
                    // this->add_stat_branch_completed();
                break;

                // NOP
                case INSTRUCTION_OPERATION_NOP:
                    // this->add_stat_nop_completed();
                break;

                // NOT IDENTIFIED
                case INSTRUCTION_OPERATION_OTHER:
                    // this->add_stat_other_completed();
                break;

                case INSTRUCTION_OPERATION_BARRIER:
				case INSTRUCTION_OPERATION_HMC_ROWA:
				case INSTRUCTION_OPERATION_HMC_ROA:
                    ERROR_PRINTF("Invalid instruction BARRIER| HMC ROA | HMC ROWA.\n");
                break;
            }

            ERROR_ASSERT_PRINTF(uint32_t(pos_buffer) == this->robStart, "Commiting different from the position start\n");
		    ORCS_PRINTF("Commited Instruction\n%s\n",this->reorderBuffer[this->robStart].content_to_string().c_str())
			this->removeFrontROB();
			sleep(1);
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
        }
    }

    // =========================================================================
    /// SOLVE REGISTER DEPENDENCIES - RAT
    // =========================================================================
    /// Send message to acknowledge the dependency is over
    for (uint32_t j = 0; j < ROB_SIZE; j++) {
        /// There is an unsolved dependency
        if (rob_line->reg_deps_ptr_array[j] != NULL) {
#if DEBUG
		ORCS_PRINTF("=====================================\n")
		ORCS_PRINTF("Instruction %s\n",rob_line->content_to_string().c_str())
		ORCS_PRINTF("Dependence %u\n%s\n",j,rob_line->reg_deps_ptr_array[j]->content_to_string().c_str())
		ORCS_PRINTF("=====================================\n")
#endif
            rob_line->reg_deps_ptr_array[j]->wait_reg_deps_number--;
            /// This update the ready cycle, and it is usefull to compute the time each instruction waits for the functional unit
            if (rob_line->reg_deps_ptr_array[j]->uop.readyAt <= orcs_engine.get_global_cycle()) {
                rob_line->reg_deps_ptr_array[j]->uop.updatePackageReady(1);
            }
#if DEBUG
		ORCS_PRINTF("=====================================\n")
		ORCS_PRINTF("Instruction %s\n",rob_line->content_to_string().c_str())
		ORCS_PRINTF("Dependence %u\n%s\n",j,rob_line->reg_deps_ptr_array[j]->content_to_string().c_str())
		ORCS_PRINTF("=====================================\n")
#endif
            rob_line->reg_deps_ptr_array[j] = NULL;
        }
        /// All the dependencies are solved
        else {
            break;
        }
    }
}


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
		if(this->unified_functional_units.size()>0){
			// Verifica se existem uops ans unidades funcionais, para executar e acessar memoria
			this->mob();
			this->execute();
		}
		if (this->unified_reservation_station.size() > 0){
			//verificar size para evitar chamar se nao existir uops a serem despachados
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