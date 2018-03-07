#include "../simulator.hpp"

// =====================================================================
processor_t::processor_t() {
this->memory_order_buffer_read = NULL;
this->memory_order_buffer_write = NULL;
};
processor_t::~processor_t(){
	//NULLing Pointers
	// delete this->fetchBuffer;
	// delete this->decodeBuffer;
utils_t::template_delete_array<memory_order_buffer_line_t>(memory_order_buffer_read);
utils_t::template_delete_array<memory_order_buffer_line_t>(memory_order_buffer_write);
};

// =====================================================================
void processor_t::allocate() {
//======================================================================
// Initializating variables
//======================================================================
	this->traceIsOver = false;
	this->hasBranch = false;
	this->insertError = false;
	this->fetchCounter = 1;
	this->decodeCounter = 1;
	this->renameCounter = 1;
	this->set_stall_wrong_branch(0);
//======================================================================
// Initializating structures
//======================================================================
	//Fetch Buffer
	this->fetchBuffer.allocate(FETCH_BUFFER);
	//Decode Buffer
	this->decodeBuffer.allocate(DECODE_BUFFER);
	//RAT
    this->register_alias_table = utils_t::template_allocate_initialize_array<reorder_buffer_line_t*>(RAT_SIZE, NULL);
	//ROB
	this->reorderBuffer.allocate(ROB_SIZE);
	for (size_t i = 0; i < this->reorderBuffer.get_capacity(); i++)
	{
		this->reorderBuffer[i].reg_deps_ptr_array = utils_t::template_allocate_initialize_array<reorder_buffer_line_t*>(ROB_SIZE,NULL);
	}
	//MOB
	this->memory_order_buffer_read = utils_t::template_allocate_array<memory_order_buffer_line_t>(MOB_READ);
	this->memory_order_buffer_write = utils_t::template_allocate_array<memory_order_buffer_line_t>(MOB_WRITE);
	
};
bool processor_t::isBusy() {
    return (this->traceIsOver == false ||
			!this->fetchBuffer.is_empty() ||
            !this->decodeBuffer.is_empty());// ||
            // reorder_buffer_position_used != 0);
}
// =====================================================================
void processor_t::fetch(){
//===========================
//Busca trace,
// Consulta I$ to stalls 
//===========================
	opcode_package_t operation;
	// Trace ->fetchBuffer
	for(int i = 0 ;i < FETCH_WIDTH;i++){
		//=============================
		//Stall full fetch buffer
		//=============================
		if(this->fetchBuffer.is_full()){
			this->add_stall_full_FetchBuffer();
			break;
		}
		//=============================
		//Stall branch wrong predict
		//=============================
		if(this->get_stall_wrong_branch()>orcs_engine.get_global_cycle()){
			break;
		}
		//=============================
		//Get new Opcode
		//=============================
		if(!orcs_engine.trace_reader->trace_fetch(&operation)){
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
		if(hasBranch){
		//solve
		uint32_t stallWrongBranch=orcs_engine.branchPredictor->solveBranch(this->previousBranch,operation);
		this->set_stall_wrong_branch(orcs_engine.get_global_cycle()+stallWrongBranch);
		if(POSITION_FAIL==this->fetchBuffer.push_back(this->previousBranch)){
			break;
		}
		hasBranch = false;
		this->fetchBuffer.back()->updatePackageReady(FETCH_LATENCY);
		this->previousBranch.package_clean();
		}
		//============================
		// Operation Branch, set flag
		//============================
		if(operation.opcode_operation == INSTRUCTION_OPERATION_BRANCH){
			orcs_engine.branchPredictor->branches++;
			this->previousBranch = operation;
			hasBranch=true;
			continue;
		}
		//============================
		//Insert into fetch buffer
		//============================
		if(POSITION_FAIL==this->fetchBuffer.push_back(operation)){
			break;
		}
		this->fetchBuffer.back()->updatePackageReady(FETCH_LATENCY);
		operation.package_clean();
	}
	//============================
	//Atualiza status dos pacotes
	// Ready At
	//============================
}
//===========================
//Elimina os elementos do fetch buffer
// ============================================================================
/// Divide the opcode into
///  1st. uop READ MEM. + unaligned
///  2st. uop READ 2 MEM. + unaligned
///  3rd. uop BRANCH
///  4th. uop ALU
///  5th. uop WRITE MEM. + unaligned
// ============================================================================
/// To maintain the right dependencies between the uops and opcodes
/// If the opcode generates multiple uops, they must be in this format:
///
/// READ    ReadRegs    = BaseRegs + IndexRegs
///         WriteRegs   = 258 (Aux Register)
///
/// ALU     ReadRegs    = * + 258 (Aux Register) (if is_read)
///         WriteRegs   = * + 258 (Aux Register) (if is_write)
///
/// WRITE   ReadRegs    = * + 258 (Aux Register)
///         WriteRegs   = NULL
// ============================================================================

void processor_t::decode(){
	uop_package_t new_uop;
	int32_t statusInsert = POSITION_FAIL;
	for (size_t i = 0; i < DECODE_WIDTH; i++)
	{
		if(this->fetchBuffer.is_empty()||
			this->fetchBuffer.front()->status != PACKAGE_STATE_READY ||
			this->fetchBuffer.front()->readyAt > orcs_engine.get_global_cycle()){
				break;
			}
		if(this->decodeBuffer.get_capacity()-this->decodeBuffer.get_size()<MAX_UOP_DECODED){
			this->add_stall_full_DecodeBuffer();
			break;
		}
		// ERROR_ASSERT_PRINTF(this->decodeCounter == this->fetchBuffer.front()->opcode_number, "Error On decode");
		assert((this->decodeCounter == this->fetchBuffer.front()->opcode_number) && "Error On Decode");
		this->decodeCounter++;
		// printf("OPCODE DECODED %s\n",this->fetchBuffer.front()->content_to_string().c_str());
		//DECODING READ UOP
		if(this->fetchBuffer.front()->is_read){
			new_uop.package_clean();
			//creating uop
			new_uop.opcode_to_uop(this->uopCounter++,
						INSTRUCTION_OPERATION_MEM_LOAD,
						this->fetchBuffer.front()->read_address,
						this->fetchBuffer.front()->read_size,
						*this->fetchBuffer.front());
			//SE OP DIFERE DE LOAD, ZERA REGISTERS
			if(this->fetchBuffer.front()->opcode_operation != INSTRUCTION_OPERATION_MEM_LOAD){
				//limpa regs
				 for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
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
			assert((statusInsert != POSITION_FAIL) && "Erro, Tentando decodificar mais uops que o maximo permitido");
			// ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL,"Erro, Tentando decodificar mais uops que o maximo permitido")
		}
		//is read2
		if(this->fetchBuffer.front()->is_read2){
			new_uop.package_clean();
			//creating uop
			new_uop.opcode_to_uop(this->uopCounter++,
						INSTRUCTION_OPERATION_MEM_LOAD,
						this->fetchBuffer.front()->read2_address,
						this->fetchBuffer.front()->read2_size,
						*this->fetchBuffer.front());
			//SE OP DIFERE DE LOAD, ZERA REGISTERS
			if(this->fetchBuffer.front()->opcode_operation != INSTRUCTION_OPERATION_MEM_LOAD){
				//limpa regs
				 for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
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
			assert((statusInsert != POSITION_FAIL) && "Erro, Tentando decodificar mais uops que o maximo permitido");
			// ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL,"Erro, Tentando decodificar mais uops que o maximo permitido")
		}
		//ALU.
		if(this->fetchBuffer.front()->opcode_operation != INSTRUCTION_OPERATION_BRANCH &&
			this->fetchBuffer.front()->opcode_operation != INSTRUCTION_OPERATION_MEM_LOAD &&
			this->fetchBuffer.front()->opcode_operation != INSTRUCTION_OPERATION_MEM_STORE){
				new_uop.package_clean();
				new_uop.opcode_to_uop(this->uopCounter++,
				this->fetchBuffer.front()->opcode_operation,
				0,
				0,
				*this->fetchBuffer.front());
				if (this->fetchBuffer.front()->is_read || this->fetchBuffer.front()->is_read2){
                // ===== Read Regs =============================================
				//registers /258 aux onde pos[i] = fail
			    bool inserted_258 = false;
                for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
                    if (new_uop.read_regs[i] == POSITION_FAIL) {
                        new_uop.read_regs[i] = 258;
                        inserted_258 = true;
                        break;
                    }
                }
				assert(!inserted_258 && "Max registers used");
                // ERROR_ASSERT_PRINTF(inserted_258, "Todos Max regs usados.", MAX_REGISTERS)
            }
            if (this->fetchBuffer.front()->is_write){
                // ===== Write Regs =============================================
                //registers /258 aux onde pos[i] = fail
                bool inserted_258 = false;
                for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
                    if (new_uop.write_regs[i] == POSITION_FAIL) {
                        new_uop.write_regs[i] = 258;
                        inserted_258 = true;
                        break;
                    }
                }
                // ERROR_ASSERT_PRINTF(inserted_258, "Todos Max regs usados.", MAX_REGISTERS)
				assert(!inserted_258 && "Max registers used");
            }
			new_uop.updatePackageReady(DECODE_LATENCY);
			// printf("\n UOP Created %s \n",new_uop.content_to_string().c_str());
			statusInsert = this->decodeBuffer.push_back(new_uop);
			assert((statusInsert != POSITION_FAIL) && "Erro, Tentando decodificar mais uops que o maximo permitido");
			// ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL,"Erro, Tentando decodificar mais uops que o maximo permitido")
		}
		//branch
		if(this->fetchBuffer.front()->opcode_operation == INSTRUCTION_OPERATION_BRANCH){
			new_uop.package_clean();
			new_uop.opcode_to_uop(this->uopCounter++,
			INSTRUCTION_OPERATION_BRANCH,
			0,
			0,
			*this->fetchBuffer.front());
			if (this->fetchBuffer.front()->is_read || this->fetchBuffer.front()->is_read2){
                // ===== Read Regs =============================================
				//registers /258 aux onde pos[i] = fail
			    bool inserted_258 = false;
                for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
                    if (new_uop.read_regs[i] == POSITION_FAIL) {
                        new_uop.read_regs[i] = 258;
                        inserted_258 = true;
                        break;
                    }
                }
                // ERROR_ASSERT_PRINTF(inserted_258, "Todos Max regs usados.", MAX_REGISTERS)
				assert(!inserted_258 && "Max registers used");
            }
            if (this->fetchBuffer.front()->is_write){
                // ===== Write Regs =============================================
                //registers /258 aux onde pos[i] = fail
                bool inserted_258 = false;
                for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
                    if (new_uop.write_regs[i] == POSITION_FAIL) {
                        new_uop.write_regs[i] = 258;
                        inserted_258 = true;
                        break;
                    }
                }
                // ERROR_ASSERT_PRINTF(inserted_258, "Todos Max regs usados.", MAX_REGISTERS)
				assert(!inserted_258 && "MAX regs inserted");
            }
			new_uop.updatePackageReady(DECODE_LATENCY);
			// printf("\n UOP Created %s \n",new_uop.content_to_string().c_str());
			statusInsert = this->decodeBuffer.push_back(new_uop);
			assert((statusInsert != POSITION_FAIL) && "Erro, Tentando decodificar mais uops que o maximo permitido");
			// ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL,"Erro, Tentando decodificar mais uops que o maximo permitido")
		}
		//Decoding WRITE
		if(this->fetchBuffer.front()->is_write){
			new_uop.package_clean();
			// make package
			new_uop.opcode_to_uop(this->uopCounter++,
			INSTRUCTION_OPERATION_MEM_STORE,
			this->fetchBuffer.front()->write_address,
			this->fetchBuffer.front()->write_size,
			*this->fetchBuffer.front());
			if(this->fetchBuffer.front()->opcode_operation !=INSTRUCTION_OPERATION_MEM_STORE){
				bool inserted_258 = false;
                for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
                    if (new_uop.read_regs[i] == POSITION_FAIL) {
                        new_uop.read_regs[i] = 258;
                        inserted_258 = true;
                        break;
                    }
                }
                // ERROR_ASSERT_PRINTF(inserted_258, "Could not insert register_258, all MAX_REGISTERS(%d) used.", MAX_REGISTERS)
				assert(!inserted_258 && "Max registers used");
                // ===== Write Regs =============================================
                /// Clear WRegs
                for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
                    new_uop.write_regs[i] = POSITION_FAIL;
                }
			}
			new_uop.updatePackageReady(DECODE_LATENCY);
			// printf("\n UOP Created %s \n",new_uop.content_to_string().c_str());
			statusInsert = this->decodeBuffer.push_back(new_uop);
			assert((statusInsert != POSITION_FAIL) && "Erro, Tentando decodificar mais uops que o maximo permitido");
			// ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL,"Erro, Tentando decodificar mais uops que o maximo permitido")
		}
	this->fetchBuffer.pop_front();
	}

};
void processor_t::rename(){
	size_t i;
	uint32_t pos_mob;
	for (i = 0; i <RENAME_WIDTH; i++)
	{	
		if(this->decodeBuffer.is_empty() || 
			this->decodeBuffer.front()->status != PACKAGE_STATE_READY ||
			this->decodeBuffer.front()->readyAt > orcs_engine.get_global_cycle()){
			break;
		}
		ERROR_ASSERT_PRINTF(this->decodeBuffer.front()->uop_number == this->renameCounter,"Erro, renomeio incorreto")
		memory_order_buffer_line_t *mob_line = NULL;
		//=======================
		// Memory Operation Read
		//=======================
		if(this->decodeBuffer.front()->uop_operation==INSTRUCTION_OPERATION_MEM_LOAD){
			pos_mob = memory_order_buffer_line_t::find_free(this->memory_order_buffer_read,MOB_READ);
			if(pos_mob==POSITION_FAIL){
				this->add_stall_full_MOB_Read();
				break;
			}
			mob_line = &this->memory_order_buffer_read[pos_mob];
		}
		//=======================
		// Memory Operation Write
		//=======================
		if(this->decodeBuffer.front()->uop_operation==INSTRUCTION_OPERATION_MEM_STORE){
			pos_mob = memory_order_buffer_line_t::find_free(this->memory_order_buffer_write,MOB_WRITE);
			if(pos_mob==POSITION_FAIL){
				this->add_stall_full_MOB_Write();
				break;
			}
			mob_line = &this->memory_order_buffer_write[pos_mob];
		}
		//=======================
		// Verificando se tem espaco no ROB se sim bamos inserir
		//=======================		
		if(this->reorderBuffer.is_full()){
			this->add_stall_full_ROB();
			break;
		}
		

		//remove uop from buffer
		this->decodeBuffer.pop_front();
	}//end for
	
}//end method
void processor_t::clock(){
/////////////////////////////////////////////////
//// Verifica se existe coisas no ROB
//// CommitStage
//// ExecuteStage
//// DispatchStage
/////////////////////////////////////////////////
if(this->reorderBuffer.get_size()>0){
	printf("no ROB");
}
/////////////////////////////////////////////////
//// Verifica se existe coisas no DecodeBuffer
//// Rename
/////////////////////////////////////////////////
	if(!this->decodeBuffer.is_empty()){
		this->rename();
	}

/////////////////////////////////////////////////
//// Verifica se existe coisas no FetchBuffer
//// Decode
/////////////////////////////////////////////////
	if(!this->fetchBuffer.is_empty()){
		this->decode();
#if SLEEP
	sleep(1);
#endif
	}
/////////////////////////////////////////////////
//// Verifica se trace is over
//// Fetch
/////////////////////////////////////////////////
	if((!this->traceIsOver)){
		this->fetch();
	}

	if(!this->isBusy()){
		orcs_engine.simulator_alive=false;
	}


		



};
// =====================================================================
void processor_t::statistics() {
	
	std::cout<< "######################################################\n"<< std::endl;
	std::cout<< "Total Cicle ;"<<orcs_engine.get_global_cycle()<< std::endl;


};