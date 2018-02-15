#include "./simulator.hpp"

// =====================================================================
processor_t::processor_t() {

};

// =====================================================================
void processor_t::allocate() {
	size_t size = ENTRY/WAYS;
	this->btb = new btb_t[size];
	for (size_t i = 0; i < size; i++)
	{
		this->btb[i].btb_entry = new btb_line_t[WAYS];
	}
	
	// Initialize statistics counters
	this->branches = 0;
	this->btbHits = 0;
	this->btbMiss = 0;
	this->branchNotTaken=0;
	this->branchTaken=0;
	this->BtMiss=0;
	this->BntMiss=0;
	// direct acess
	this->index = 0;
	this->assoc = 0;
	this->has_branch = FAIL;
	//allocate plbt variables
	this->predict = NOT_TAKEN;
	this->oldAdd = 0;
};

// =====================================================================

void processor_t::clock(){
/// Get the next instruction from the trace
opcode_package_t new_instruction;
if (!orcs_engine.trace_reader->trace_fetch(&new_instruction)) {
	/// If EOF
	orcs_engine.simulator_alive = false;
}
if(this->has_branch==OK){

	if(this->btb[this->index].btb_entry[this->assoc].typeBranch !=BRANCH_COND){
		branchTaken++;
	}
#if ONE_BIT	
	else{
		// One Bit history, OK
	if(new_instruction.opcode_address!=this->btb[this->index].btb_entry[this->assoc].targetAddress){
		this->branchTaken++;
		if(this->btb[this->index].btb_entry[this->assoc].bht==NOT_TAKEN){
			this->btb[this->index].btb_entry[this->assoc].bht=TAKEN;
			this->BtMiss++;
			orcs_engine.global_cycle+=BTB_MISS_PENALITY;
		}				
	}else{
		this->branchNotTaken++;
		if(this->btb[this->index].btb_entry[this->assoc].bht==TAKEN){
			this->btb[this->index].btb_entry[this->assoc].bht=NOT_TAKEN;
			this->BntMiss++;
			orcs_engine.global_cycle+=BTB_MISS_PENALITY;
			}
		}
	}
#endif
#if TWO_BIT
	// Two Bit OK
	else{
		if(new_instruction.opcode_address!=\
			this->btb[this->index].btb_entry[this->assoc].targetAddress){
				this->branchTaken++;
				if((this->btb[this->index].btb_entry[this->assoc].bht>1)){
					if(this->btb[this->index].btb_entry[this->assoc].bht==2){
						this->btb[this->index].btb_entry[this->assoc].bht++;
						}
					}else{	
						orcs_engine.global_cycle+=BTB_MISS_PENALITY;
						this->BtMiss++;
						this->btb[this->index].btb_entry[this->assoc].bht++;				
					}
			}
			else{
				this->branchNotTaken++;
				if(this->btb[this->index].btb_entry[this->assoc].bht<2){
					if(this->btb[this->index].btb_entry[this->assoc].bht==1){
						this->btb[this->index].btb_entry[this->assoc].bht--;
					}
				}else{
					this->BntMiss++;
					this->btb[this->index].btb_entry[this->assoc].bht--;
					orcs_engine.global_cycle+=BTB_MISS_PENALITY;
					
					
				}
			}
		}
#endif
#if PIECEWISE
		if((new_instruction.opcode_address!=\
			this->btb[this->index].btb_entry[this->assoc].targetAddress)&&(this->btb[this->index].btb_entry[this->assoc].typeBranch == BRANCH_COND)){
				this->branchTaken++;
				if(this->predict == TAKEN){
					orcs_engine.plbp->train(this->btb[this->index].btb_entry[this->assoc].tag,this->predict);
				}else{
					orcs_engine.plbp->train(this->btb[this->index].btb_entry[this->assoc].tag,TAKEN);
					this->BtMiss++;
					orcs_engine.global_cycle+=BTB_MISS_PENALITY;
				}
			}
		else{
			this->branchNotTaken++;
			if(this->predict == NOT_TAKEN){
				orcs_engine.plbp->train(this->btb[this->index].btb_entry[this->assoc].tag,this->predict);
			}else{
				orcs_engine.plbp->train(this->btb[this->index].btb_entry[this->assoc].tag,NOT_TAKEN);
				this->BntMiss++;
				orcs_engine.global_cycle+=BTB_MISS_PENALITY;
			}
		}
#endif
	this->has_branch = FAIL;
	}
	if(new_instruction.opcode_operation==INSTRUCTION_OPERATION_BRANCH)
	{	
		this->branches++;
		this->has_branch = OK;
		//BTB
			uint32_t hit = this->searchLine(new_instruction.opcode_address);
			if(hit==HIT){
				this->btbHits++;
				this->updateLruAll(HIT);
			}else{
				this->btbMiss++;
				this->updateLruAll(BTB_MISS_PENALITY);
				//install line
				this->installLine(new_instruction);					
				orcs_engine.global_cycle+=BTB_MISS_PENALITY;
			}
#if PIECEWISE
			this->predict = orcs_engine.plbp->predict(new_instruction.opcode_address);
#endif
		this->nextInstruction = new_instruction.opcode_address+new_instruction.opcode_size;
		this->oldAdd = new_instruction.opcode_address;
		}
		// loads e stores - decomposicao 
			if(new_instruction.is_read){
				this->searchCache(new_instruction.read_address);
			}
			if(new_instruction.is_read2){
				this->searchCache(new_instruction.read2_address);
			}
			if(new_instruction.is_write){
				this->writeCache(new_instruction.write_address);
			}
		
	
};
// =====================================================================
void processor_t::statistics() {
	
	std::cout<< "######################################################\n"<< std::endl;
	std::cout<< "processor_t"<< std::endl;
	std::cout<< "*******\nBTB\n********"<< std::endl;
	std::cout<< "BTB Hits ;"<<this->btbHits<< std::endl;
	std::cout<< "BTB Miss ;"<<this->btbMiss<< std::endl;
	std::cout<< "**********\nPredictor\n**********"<< std::endl;
	std::cout<< "Taken Branches; "<< this->branchTaken<< std::endl;
	std::cout<< "Not Taken Branches; "<<this->branchNotTaken<< std::endl;
	std::cout<< "Correct Prediction Taken; "<<(this->branchTaken-this->BtMiss)<< std::endl;
	std::cout<< "Misprediction Taken; "<<this->BtMiss<< std::endl;
	std::cout<< "Correct Prediction NotTaken; "<<(this->branchNotTaken-this->BntMiss)<< std::endl;
	std::cout<< "Mispredicion NotTaken; "<<this->BntMiss<< std::endl;
	std::cout<< "Total Branches; "<<this->branches<< std::endl;
	std::cout<< "Total Cicle ;"<<orcs_engine.get_global_cycle()<< std::endl;


};
void processor_t::updateLruAll(uint32_t add){
	size_t size = ENTRY/WAYS;
	for(size_t i = 0;i<size;i++){
		for (size_t j = 0; j < WAYS; j++)
		{
			this->btb[i].btb_entry[j].lru +=add;
		}
	}
};

uint32_t processor_t::searchLine(uint32_t pc){
	uint32_t getBits = (ENTRY/WAYS)-1;
	uint32_t tag = (pc >> 2);
	uint32_t index = tag&getBits;
	// std::cout<< "bits %u, tag %u index %u\n",getBits,tag,index);
	for (size_t i = 0; i < WAYS; i++)
	{
		//std::cout<< "%u\n",this->btb[index].btb_entry[i].tag);
		if(this->btb[index].btb_entry[i].tag == pc){
			//std::cout<< "BTB_Hit");
			this->btb[index].btb_entry[i].lru=0;
			//save locate from line
			this->index = index;
			this->assoc = i;
			return HIT;
		}
	}
	//std::cout<< "BTB_Miss");
	return MISS;
}
uint32_t processor_t::installLine(opcode_package_t instruction){
	uint32_t getBits = (ENTRY/WAYS)-1;
	uint32_t tag = (instruction.opcode_address >> 2);
	uint32_t index = tag&getBits;
	// std::cout<< "bits %u, tag %u index %u\n",getBits,tag,index);
	for (size_t i = 0; i < WAYS; i++)
	{
		// instala no primeiro invalido 
		if(this->btb[index].btb_entry[i].validade == 0){
			this->btb[index].btb_entry[i].tag=instruction.opcode_address;
			this->btb[index].btb_entry[i].lru=0;
			this->btb[index].btb_entry[i].targetAddress=instruction.opcode_address+instruction.opcode_size;
			this->btb[index].btb_entry[i].validade=1;
			this->btb[index].btb_entry[i].typeBranch=instruction.branch_type;
			this->btb[index].btb_entry[i].bht=0;
			return OK;
		}			
	}
	uint32_t way = this->searchLru(&this->btb[index]);
	this->btb[index].btb_entry[way].tag=instruction.opcode_address;
	this->btb[index].btb_entry[way].lru=0;
	this->btb[index].btb_entry[way].targetAddress=instruction.opcode_address+instruction.opcode_size;
	this->btb[index].btb_entry[way].validade=1;
	this->btb[index].btb_entry[way].typeBranch=instruction.branch_type;
	this->btb[index].btb_entry[way].bht=0;
	//indexes
	this->index = index;
	this->assoc = way;
	return OK;
};
inline uint32_t processor_t::searchLru(btb_t *btb){
	uint32_t index=0;
	for (size_t i = 1; i < WAYS; i++)
	{
		index = (btb->btb_entry[index].lru > btb->btb_entry[i].lru)? index : i ;
	}
	return index;
}
void processor_t::searchCache(uint64_t address){
	// L1 Search
	uint32_t ok = orcs_engine.cache[L1].searchAddress(address);
	orcs_engine.global_cycle+=L1_LATENCY;
	if(ok==HIT){
		orcs_engine.cache[L1].cacheAccess++;
		orcs_engine.cache[L1].cacheHit++;
	}else{
		orcs_engine.cache[L1].cacheAccess++;
		orcs_engine.cache[L1].cacheMiss++;
		orcs_engine.global_cycle+=LLC_LATENCY;
		ok = orcs_engine.cache[LLC].searchAddress(address);
		if(ok==HIT){
			orcs_engine.cache[LLC].cacheAccess++;
			orcs_engine.cache[LLC].cacheHit++;
			orcs_engine.cache[LLC].returnLine(address,&orcs_engine.cache[L1]);
		}
		else{
			orcs_engine.cache[LLC].cacheAccess++;
			orcs_engine.cache[LLC].cacheMiss++;
			orcs_engine.global_cycle+=RAM_LATENCY;
			orcs_engine.cache[L1].installLine(address);
		// 	//orcs_engine.cache[LLC].installLine(address);
		}
	}
}
void processor_t::writeCache(uint64_t address){
	orcs_engine.cache[L1].cacheAccess++;
	orcs_engine.cache[L1].writeAllocate(address);
};	