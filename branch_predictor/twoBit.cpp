#include "../simulator.hpp"

twoBit_t::twoBit_t(){
    this->btb = NULL
};
twoBit_t::~twoBit_t(){
    if(this->btb) delete[] this->btb;
};
void twoBit_t::allocate(){
    uint32_t size  = BTB_ENTRIES/BTB_WAYS;
    this->btb = new btb_t[size];
    for (size_t i = 0; i < size; i++)
    {
        this->btb[i].btb_entry = new btb_line_t[BTB_WAYS];
    }
};
void twoBit_t::train(uint64_t address){

};

uint32_t twoBit_t::predict(uint64_t address){
    this->searchLine(address);
}
uint32_t twobit_t::searchLine(uint64_t pc){
	uint32_t getBits = (BTB_ENTRIES/BTB_WAYS);
	uint32_t tag = (pc >> 2);
	uint32_t index = tag&(getBits-1);
	// std::cout<< "bits %u, tag %u index %u\n",getBits,tag,index);
	for (size_t i = 0; i < WAYS; i++)
	{
		//std::cout<< "%u\n",this->btb[index].btb_entry[i].tag);
		if(this->btb[index].btb_entry[i].tag == pc){
			//std::cout<< "BTB_Hit");
			this->btb[index].btb_entry[i].lru=orcs_engine.get_global_cycle();
			//save locate from line
			this->index = index;
			this->way = i;
			return HIT;
		}
	}
	//std::cout<< "BTB_Miss");
	return MISS;
}
uint32_t twoBit_t::installLine(opcode_package_t instruction){
	uint32_t getBits = (BTB_ENTRIES/BTB_WAYS);
	uint32_t tag = (instruction.opcode_address >> 2);
	uint32_t index = tag&(getBits-1);
	// std::cout<< "bits %u, tag %u index %u\n",getBits,tag,index);
	for (size_t i = 0; i < BTB_WAYS; i++)
	{
		// instala no primeiro invalido 
		if(this->btb[index].btb_entry[i].validade == 0){
			this->btb[index].btb_entry[i].tag=instruction.opcode_address;
			this->btb[index].btb_entry[i].lru=orcs_engine.get_global_cycle();
			this->btb[index].btb_entry[i].targetAddress=instruction.opcode_address+instruction.opcode_size;
			this->btb[index].btb_entry[i].validade=1;
			this->btb[index].btb_entry[i].typeBranch=instruction.branch_type;
			this->btb[index].btb_entry[i].bht=0;
			return OK;
		}			
	}
	uint32_t way = this->searchLru(&this->btb[index]);
	this->btb[index].btb_entry[way].tag=instruction.opcode_address;
	this->btb[index].btb_entry[way].lru=orcs_engine.get_global_cycle();
	this->btb[index].btb_entry[way].targetAddress=instruction.opcode_address+instruction.opcode_size;
	this->btb[index].btb_entry[way].validade=1;
	this->btb[index].btb_entry[way].typeBranch=instruction.branch_type;
	this->btb[index].btb_entry[way].bht=0;
	//indexes
	this->index = index;
	this->way = way;
	return OK;
};
inline uint32_t twoBit_t::searchLRU(btb_t *btb){
	uint32_t index=0;
	for (size_t i = 1; i < BTB_WAYS; i++)
	{
		index = (btb->btb_entry[index].lru <= btb->btb_entry[i].lru)? index : i ;
	}
	return index;
};