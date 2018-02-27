#include "../simulator.hpp"

// =====================================================================
processor_t::processor_t() {

};
processor_t::~processor_t(){};

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
//======================================================================
// Initializating structures
//======================================================================

	this->fetchBuffer.allocate(FETCH_BUFFER);
	this->decodeBuffer.allocate(DECODE_BUFFER);

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
		if(this->fetchBuffer.is_full()){
			this->add_stallFetch();
			break;
		}
		if(!orcs_engine.trace_reader->trace_fetch(&operation)){
			this->traceIsOver = true;
			break;
		}
		///////
		//add control variables
		//////
		operation.opcode_number = this->fetchCounter;
		this->fetchCounter++;
		operation.status = PACKAGE_STATE_READY;
		///
		if(hasBranch){
		//solve
		orcs_engine.global_cycle+=orcs_engine.branchPredictor->solveBranch(this->previousBranch,operation);
		hasBranch = false;
		this->previousBranch.package_clean();
		}
		if(operation.opcode_operation == INSTRUCTION_OPERATION_BRANCH){
			orcs_engine.branchPredictor->branches++;
			this->previousBranch = operation;
			hasBranch=true;
		}
		 
		if(POSITION_FAIL==this->fetchBuffer.push_back(operation)){
			break;
		}
		operation.package_clean();
	}
	//consulta I$ para saber quando a instrucao ficara pronta. Se resula em I$ miss adianta relogio?

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
			this->add_stallDecode();
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
	for (i = 0; i <RENAME_WIDTH; i++)
	{
		if(this->decodeBuffer.is_empty()){
			break;
		}
		printf(this->decodeBuffer.front()->content_to_string().c_str()+'\n');
		this->decodeBuffer.pop_front();
	}
}
void processor_t::clock(){
/////////////////////////////////////////////////
//// Verifica se existe coisas no ROB
//// CommitStage
//// ExecuteStage
//// DispatchStage
/////////////////////////////////////////////////

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
//// Fetch and decode
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