#include "../simulator.hpp"


cache_t::cache_t()
{
    //ctor
	this->sets = NULL;
}

cache_t::~cache_t()
{
}
void cache_t::allocate(uint32_t level){	
	switch(level){
		case INST_CACHE:{
			this->shiftData = utils_t::get_power_of_two(LINE_SIZE);
			this->level = level;
			this->cacheHit=0;
			this->cacheMiss=0;
			this->cacheAccess=0;
			this->nSets = L1_INST_SETS;
			this->nLines = L1_INST_ASSOCIATIVITY;
			this->sets = new cacheSet_t[L1_INST_SETS];
			for (size_t i = 0; i < L1_INST_SETS; i++)
			{
				this->sets[i].linhas = new linha_t[L1_INST_ASSOCIATIVITY];
				std::memset(&this->sets[i].linhas[0],0,(L1_INST_ASSOCIATIVITY*sizeof(linha_t)));
			}
			break;
			}
		case L1:{
			this->shiftData = utils_t::get_power_of_two(LINE_SIZE);
			this->level = level;
			this->cacheHit=0;
			this->cacheMiss=0;
			this->cacheAccess=0;
			this->nSets = L1_DATA_SETS;
			this->nLines = L1_DATA_ASSOCIATIVITY;
			this->sets = new cacheSet_t[L1_DATA_SETS];
			for (size_t i = 0; i < L1_DATA_SETS; i++)
			{
				this->sets[i].linhas = new linha_t[L1_DATA_ASSOCIATIVITY];
				std::memset(&this->sets[i].linhas[0],0,(L1_DATA_ASSOCIATIVITY*sizeof(linha_t)));
			}
			break;
			}
		case L2:{
			this->shiftData = utils_t::get_power_of_two(LINE_SIZE);
			this->level = level;
			this->cacheHit=0;
			this->cacheMiss=0;
			this->cacheAccess=0;
			this->nSets = L2_SETS;
			this->nLines = L2_ASSOCIATIVITY;
			this->sets = new cacheSet_t[L2_SETS];
			for (size_t i = 0; i < L2_SETS; i++)
			{
				this->sets[i].linhas = new linha_t[L2_ASSOCIATIVITY];
				std::memset(&this->sets[i].linhas[0],0,(L2_ASSOCIATIVITY*sizeof(linha_t)));
			}
			break;
		}
		case LLC:{
			this->shiftData = utils_t::get_power_of_two(LINE_SIZE);
			this->level = level;
			this->cacheHit=0;
			this->cacheMiss=0;
			this->cacheAccess=0;
			this->nSets = LLC_SETS;
			this->nLines = LLC_ASSOCIATIVITY;
			this->sets = new cacheSet_t[LLC_SETS];
			for (size_t i = 0; i < LLC_SETS; i++)
			{
				this->sets[i].linhas = new linha_t[LLC_ASSOCIATIVITY];
				std::memset(&this->sets[i].linhas[0],0,(LLC_ASSOCIATIVITY*sizeof(linha_t)));
			}
			break;
			}
		}
};
uint32_t cache_t::idxSetCalculation(uint64_t address){
	uint32_t getBits = (this->nSets)-1;
	uint32_t tag = (address >> this->shiftData);
	uint32_t index = tag&getBits;
	return index;

};
uint32_t cache_t::searchAddress(uint64_t address){
	uint32_t idx = this->idxSetCalculation(address);
	uint32_t tag = (address >> this->shiftData);
	for (size_t i = 0; i < this->nLines; i++)
	{
		if(this->sets[idx].linhas[i].tag == tag){
			this->sets[idx].linhas[i].LRU =  orcs_engine.get_global_cycle();
			this->add_cacheHit();
			return HIT;
		}else{
			this->add_cacheMiss();
			return MISS;
		}
	}
	return MISS;
};
uint32_t cache_t::installLine(uint64_t address){
	uint32_t idx = this->idxSetCalculation(address);
	uint32_t tag = (address >> this->shiftData);
	for (size_t i = 0; i < this->nLines; i++)
	{
		if(this->sets[idx].linhas[i].valid==0){
			this->sets[idx].linhas[i].tag = tag;
			this->sets[idx].linhas[i].LRU = orcs_engine.get_global_cycle();
			this->sets[idx].linhas[i].valid = 1;
			this->sets[idx].linhas[i].dirty = 0;
			return i;
		}
	}
	uint32_t line = this->searchLru(&this->sets[idx]);
	if(this->sets[idx].linhas[line].dirty==1){
		this->writeBack(address,&this->sets[idx].linhas[line]);
		this->add_cacheWriteBack();
		}
	this->sets[idx].linhas[line].tag = tag;
	this->sets[idx].linhas[line].LRU = orcs_engine.get_global_cycle();
	this->sets[idx].linhas[line].valid = 1;	
	this->sets[idx].linhas[line].dirty = 0;	
	return line;
};
inline uint32_t cache_t::searchLru(cacheSet_t *set){
	uint32_t index=0;
	for (size_t i = 1; i < this->nLines; i++)
	{
		index = (set->linhas[index].LRU <= set->linhas[i].LRU)? index : i ;
	}
	return index;
};
inline void cache_t::printLine(linha_t *linha){
	fprintf(stdout,"TAG: %lu\n",linha->tag);
	fprintf(stdout,"DIRTY: %d\n",linha->dirty);
	fprintf(stdout,"LRU: %d\n",linha->LRU);
	fprintf(stdout,"PREFETCHED: %d\n",linha->prefetched);
	fprintf(stdout,"VALID: %d\n",linha->valid);
	usleep(500);
};

//====================
//move line to
// @1 address - endereco do dado
// @2 linha a ser feito WB
//====================
inline void cache_t::writeBack(uint64_t address, linha_t *linha){
	// uint32_t idx = this->idxSetCalculation(address);
	// uint32_t line = this->searchLru(&this->sets[idx]);
	if(this->level == L1){
		//move line to llc alterar
		this->moveLineTo(address,&orcs_engine.cacheManager->data_cache[LLC],linha);
		// invalidando a linha recem feita WB
		linha->valid = 0;
	}else{
		std::memset(linha,0,sizeof(linha_t));
	}
};
//====================
//move line to
// @1 address - endereco do dado
// @2 nivel de cache alvo da mudanca
// @3 *retorno 
//====================
void cache_t::returnLine(uint64_t address,cache_t *cache, int32_t &retorno){
	uint32_t idx = this->idxSetCalculation(address);
	uint32_t line=0;
	uint32_t tag = (address >> this->shiftData);
	for (size_t i = 0; i < this->nLines; i++)
	{
		if(this->sets[idx].linhas[i].tag==tag){
			line = i;
			break;
		}
	}	
	retorno = this->moveLineTo(address,cache,&this->sets[idx].linhas[line]);
};
//====================
//move line to
// @1 address - endereco do dado
// @2 nivel de cache alvo da mudanca
// @3 linha a ser movida
//====================
uint32_t cache_t::moveLineTo(uint64_t address,cache_t *cache, linha_t *linha){
	//calcula endereco na nivel acima
	//cache representa nivel acima
	uint32_t idx = cache->idxSetCalculation(address);
	uint32_t tag = (address >> cache->shiftData);
	uint32_t line=0;
	//busca se ja existe linha naquele nivel.
	for (size_t i = 0; i < cache->nLines; i++)
	{	
		//existindo linha, so copia do outro nivel de cache
		if(cache->sets[idx].linhas[i].tag == tag){
			std::memcpy(&cache->sets[idx].linhas[i],linha,sizeof(linha_t));
			cache->sets[idx].linhas[i].LRU = orcs_engine.get_global_cycle();
			return i;
		}
	}
	line = cache->searchLru(&cache->sets[idx]);
	if(cache->sets[idx].linhas[line].dirty==1){
		cache->writeBack(address,&cache->sets[idx].linhas[line]);
		cache->add_cacheWriteBack();
	}	
	std::memcpy(&cache->sets[idx].linhas[line],linha,sizeof(linha_t));
	cache->sets[idx].linhas[line].tag = tag;
	cache->sets[idx].linhas[line].LRU = orcs_engine.get_global_cycle();
	return line;
	// this->printLine(&cache->sets[idx].linhas[line]);
};
uint32_t cache_t::write(uint64_t address,int32_t line){
	uint32_t tag = (address >> this->shiftData);
	uint32_t idx = this->idxSetCalculation(address);
		if(line == POSITION_FAIL){
			for (size_t i = 0; i < this->nLines; i++)
			{
				if(this->sets[idx].linhas[i].tag == tag){
					line = i;
					break;
				}
			}
		}
		this->sets[idx].linhas[line].dirty=1;
	return OK;	
};
void cache_t::statistics(){
	fprintf(stdout,"Cache Level: %u\n",this->level+1);
	fprintf(stdout,"Cache Access: %u\n",this->get_cacheAccess());
	fprintf(stdout,"Cache Hits: %u %.2f\n",this->get_cacheHit(),float((this->get_cacheHit()/this->get_cacheAccess())*100));
	fprintf(stdout,"Cache Miss: %u %.2f\n",this->get_cacheMiss(),float((this->get_cacheMiss()/this->get_cacheAccess())*100));
	fprintf(stdout,"Cache Read: %u %.2f\n",this->get_cacheRead(),float((this->get_cacheRead()/this->get_cacheAccess())*100));
	fprintf(stdout,"Cache Write: %u %.2f\n",this->get_cacheWrite(),float((this->get_cacheWrite()/this->get_cacheAccess())*100));
	fprintf(stdout,"Cache WriteBack: %u %.2f\n",this->get_cacheWriteBack(),float((this->get_cacheWriteBack()/this->get_cacheWrite())*100));
}