#include "simulator.hpp"


cache_t::cache_t()
{
    //ctor
}

cache_t::~cache_t()
{
    //dtor
}
void cache_t::allocate(uint32_t level){
	
	
	switch(level){
		case L1:{
			this->shiftData = utils_t::powerOf2(BLOCK_SIZE);
			this->level = level;
			this->cacheHit=0;
			this->cacheMiss=0;
			this->cacheAccess=0;
			this->nSets = L1_SETS;
			this->nLines = L1_ASSOCIATIVITY;
			this->sets = new cacheSet_t[L1_SETS];
			for (size_t i = 0; i < L1_SETS; i++)
			{
				this->sets[i].linhas = new linha_t[L1_ASSOCIATIVITY];
				std::memset(this->sets[i].linhas,0,BYTES_ON_LINE);
			}
			break;
			}
		case LLC:{
			this->shiftData = utils_t::powerOf2(BLOCK_SIZE);
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
				std::memset(this->sets[i].linhas,0,BYTES_ON_LINE);
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
			return HIT;
		}else{
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
			this->sets[idx].linhas[i].tag =tag;
			this->sets[idx].linhas[i].LRU = orcs_engine.get_global_cycle();
			this->sets[idx].linhas[i].valid = 1;
			this->sets[idx].linhas[i].dirty = 0;
			return OK;
		}
	}
	uint32_t line = this->searchLru(&this->sets[idx]);
	if(this->sets[idx].linhas[line].dirty==1){
		this->writeBack(address);
		}
	this->sets[idx].linhas[line].tag = tag;
	this->sets[idx].linhas[line].LRU = orcs_engine.get_global_cycle();
	this->sets[idx].linhas[line].valid = 1;	
	this->sets[idx].linhas[line].dirty = 0;	
	return OK;
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
	fprintf(stderr,"TAG: %lu\n",linha->tag);
	fprintf(stderr,"DIRTY: %d\n",linha->dirty);
	fprintf(stderr,"LRU: %d\n",linha->LRU);
	fprintf(stderr,"PREFETCHED: %d\n",linha->prefetched);
	fprintf(stderr,"VALID: %d\n",linha->valid);
	usleep(500);
};
inline void cache_t::writeBack(uint64_t address){
	uint32_t idx = this->idxSetCalculation(address);
	uint32_t line = this->searchLru(&this->sets[idx]);
	if(this->level == L1){
		//move line to llc
			this->moveLineTo(address,&orcs_engine.cache[LLC],&this->sets[idx].linhas[line]);
			orcs_engine.global_cycle+=LLC_LATENCY;
	}else{
	std::memset(&this->sets[idx].linhas[line],0,sizeof(linha_t));
	orcs_engine.global_cycle+=RAM_LATENCY;
	}
};
void cache_t::returnLine(uint64_t address,cache_t *cache){
	uint32_t idx = this->idxSetCalculation(address);
	uint32_t line=0;
	uint32_t tag = (address >> this->shiftData);
	for (size_t i = 0; i < this->nLines; i++)
	{
		if(this->sets[idx].linhas[i].tag==tag){
			line = i;
		}
	}	
	this->moveLineTo(address,cache,&this->sets[idx].linhas[line]);
};
void cache_t::moveLineTo(uint64_t address,cache_t *cache, linha_t *linha){
	uint32_t idx = cache->idxSetCalculation(address);
	uint32_t line = cache->searchLru(&cache->sets[idx]);
	uint32_t tag = (address >> cache->shiftData);
	if(cache->sets[idx].linhas[line].dirty==1){
		cache->writeBack(address);
	}
	
	std::memcpy(&cache->sets[idx].linhas[line],linha,sizeof(linha_t));
	cache->sets[idx].linhas[line].tag = tag;
	cache->sets[idx].linhas[line].LRU = orcs_engine.get_global_cycle();
	// this->printLine(&cache->sets[idx].linhas[line]);
};
uint32_t cache_t::writeAllocate(uint64_t address){
	uint32_t ret = this->searchAddress(address);
	uint32_t idx = this->idxSetCalculation(address);
	uint32_t tag = (address >> this->shiftData);
	if(ret == HIT){
		this->cacheHit++;
		for (size_t i = 0; i < this->nLines; i++)
		{
			if(this->sets[idx].linhas[i].tag==tag){
				this->sets[idx].linhas[i].dirty=1;
				this->sets[idx].linhas[i].LRU=orcs_engine.get_global_cycle();
				return OK;
			}
		}
		this->installLine(address);
			for (size_t i = 0; i < this->nLines; i++)
			{
				if(this->sets[idx].linhas[i].tag==tag){
						this->sets[idx].linhas[i].dirty=1;
						this->sets[idx].linhas[i].LRU=orcs_engine.get_global_cycle();
						return OK;
					}
			}
	}
	else{
		this->cacheMiss++;
		//consulta L2 se ta la traz pra L1 senao instala nova
		if(orcs_engine.cache[LLC].searchAddress(address)==HIT){
			this->returnLine(address,this);
		}else{
			this->installLine(address);
		}
		
		for (size_t i = 0; i < this->nLines; i++)
		{
			if(this->sets[idx].linhas[i].tag==tag){
				this->sets[idx].linhas[i].dirty=1;
				this->sets[idx].linhas[i].LRU=orcs_engine.get_global_cycle();
				return OK;
			}
		}
	}
	return OK;
};
void cache_t::statistics(){
	// fprintf(stderr,"Cache Level; %u\n",this->level+1);
	// fprintf(stderr,"Cache Access; %u\n",this->cacheAccess);
	// fprintf(stderr,"Cache Hits; %u\n",this->cacheHit);
	// fprintf(stderr,"Cache Miss; %u\n",this->cacheMiss);
	std::cout<<"Cache Level;"<<this->level+1<<std::endl;
	std::cout<<"Cache Access;"<<this->cacheAccess<<std::endl;
	std::cout<<"Cache Hits;"<<this->cacheHit<<std::endl;
	std::cout<<"Cache Miss;"<<this->cacheMiss<<std::endl;
}