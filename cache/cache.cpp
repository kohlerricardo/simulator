#include "../simulator.hpp"


cache_t::cache_t()
{
    //ctor
	this->sets = NULL;
}

cache_t::~cache_t()
{
	// if(this->sets) delete &sets;
}    
// ==================
// @*linha -linha to be printed
// ==================
inline void cache_t::printLine(linha_t *linha){
	ORCS_PRINTF("[TAG: %lu| DIRTY: %u| lru : %lu| PREFETCHED: %u| VALID: %u| READY AT %lu]\n",linha->tag,linha->dirty,linha->lru,linha->prefetched, linha->valid,linha->readyAt)
	#if SLEEP
		usleep(500);
	#endif
};
// ==================
// print cache configuration
// ==================
inline void cache_t::printCacheConfiguration(){
	ORCS_PRINTF("[Cache Level: %s|Cache ID: %u| Cache Sets: %u| Cache Lines: %u] \n",get_enum_cache_level_char(this->level),this->id,this->nSets,this->nLines)
	#if SLEEP
		usleep(500);
	#endif
};

void cache_t::allocate(cacheLevel_t level){	
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
				// std::memset(&this->sets[i].linhas[0],0,(L1_INST_ASSOCIATIVITY*sizeof(linha_t)));
			}
			this->set_cacheAccess(0);
			this->set_cacheHit(0);
			this->set_cacheMiss(0);
			this->set_cacheRead(0);
			this->set_cacheWrite(0);
			this->set_cacheWriteBack(0);
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
				// std::memset(&this->sets[i].linhas[0],0,(L1_DATA_ASSOCIATIVITY*sizeof(linha_t)));
			}
			this->set_cacheAccess(0);
			this->set_cacheHit(0);
			this->set_cacheMiss(0);
			this->set_cacheRead(0);
			this->set_cacheWrite(0);
			this->set_cacheWriteBack(0);
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
				// std::memset(&this->sets[i].linhas[0],0,(L2_ASSOCIATIVITY*sizeof(linha_t)));
			}
			this->set_cacheAccess(0);
			this->set_cacheHit(0);
			this->set_cacheMiss(0);
			this->set_cacheRead(0);
			this->set_cacheWrite(0);
			this->set_cacheWriteBack(0);
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
				// std::memset(&this->sets[i].linhas[0],0,(LLC_ASSOCIATIVITY*sizeof(linha_t)));
			}
			this->set_cacheAccess(0);
			this->set_cacheHit(0);
			this->set_cacheMiss(0);
			this->set_cacheRead(0);
			this->set_cacheWrite(0);
			this->set_cacheWriteBack(0);
			break;
		}
	}
};
// ==================
// @address -address to get index
// @return tag to data in cache
// ==================
inline uint32_t cache_t::tagSetCalculation(uint64_t address){
	uint32_t tag = (address >> this->shiftData);
	return tag;

};
// ==================
// @address -address to get index
// @return index of data in cache
// ==================
inline uint32_t cache_t::idxSetCalculation(uint64_t address){
	uint32_t getBits = (this->nSets)-1;
	uint32_t tag = this->tagSetCalculation(address);
	uint32_t index = tag&getBits;
	return index;

};
// ==================
// @address -address to make a read
// @ttc latency to complete
// @return HIT or MISS
// ==================
uint32_t cache_t::read(uint64_t address,uint32_t &ttc){
	uint32_t idx = this->idxSetCalculation(address);
	uint32_t tag = this->tagSetCalculation(address);
	this->add_cacheRead();
	this->add_cacheAccess();
	for (size_t i = 0; i < this->nLines; i++)
	{
		if(this->sets[idx].linhas[i].tag == tag){
			this->add_cacheHit();
			// =====================================================
			// Se ready Cycle for menor que o atual, a latencia é
			// apenas da leitura, sendo um hit.
			// =====================================================
			if(this->sets[idx].linhas[i].readyAt<=orcs_engine.get_global_cycle()){
				this->sets[idx].linhas[i].lru = orcs_engine.get_global_cycle();
				//add cache hit
				if(this->level == INST_CACHE){
					ttc+=L1_INST_LATENCY;
				}else if(this->level == L1){
					ttc+=L1_DATA_LATENCY;
				}else if(this->level == L2){
					ttc+=L2_LATENCY;
				}else{
					ttc+=LLC_LATENCY;
				}
				return HIT;
			}
			// =====================================================
			// Se ready Cycle for maior que o atual, a latencia é
			// dada pela demora a chegar, mais a latencia do acesso
			// =====================================================
			else{
				if(this->level == INST_CACHE){
					ttc+=(this->sets[idx].linhas[i].readyAt - orcs_engine.get_global_cycle());
					ttc+=L1_INST_LATENCY;
				}else if(this->level == L1){
					ttc+=(this->sets[idx].linhas[i].readyAt - orcs_engine.get_global_cycle());
					ttc+=L1_DATA_LATENCY;
				}else if(this->level == L2){
					ttc+=(this->sets[idx].linhas[i].readyAt - orcs_engine.get_global_cycle());
					ttc+=L2_LATENCY;
				}else{
					ttc+=(this->sets[idx].linhas[i].readyAt - orcs_engine.get_global_cycle());
					ttc+=LLC_LATENCY;
				}
				this->sets[idx].linhas[i].lru = ttc;
				return HIT;
			}
			//Cache MISS
		}else{
			this->add_cacheMiss();
			if(this->level == INST_CACHE){
					ttc+=L1_INST_LATENCY;
				}else if(this->level == L1){
					ttc+=L1_DATA_LATENCY;
				}else if(this->level == L2){
					ttc+=L2_LATENCY;
				}else{
					ttc+=LLC_LATENCY;
				}
			return MISS;
		}
	}
	return MISS;
};
// ============================
// @address write address
// ============================
uint32_t cache_t::write(uint64_t address){
	uint32_t tag = this->tagSetCalculation(address);
	uint32_t idx = this->idxSetCalculation(address);
	uint32_t line = 0;
	this->add_cacheWrite();
	this->add_cacheAccess();
			for (size_t i = 0; i < this->nLines; i++){
				if(this->sets[idx].linhas[i].tag == tag){
					this->add_cacheHit();
					line = i;
					break;
				}
			}
		//acertar lru.
		if(this->sets[idx].linhas[line].readyAt<=orcs_engine.get_global_cycle()){
			this->sets[idx].linhas[line].dirty=1;
			this->sets[idx].linhas[line].lru = orcs_engine.get_global_cycle();
		}else{
			this->sets[idx].linhas[line].dirty=1;
			// this->sets[idx].linhas[line].lru = this->sets[idx].linhas[line].readyAt+L1_DATA_LATENCY;
		}
	
	return OK;	
};
// ==================
// @address - address to install a line
// @return - pointer to line  
// ==================
linha_t* cache_t::installLine(uint64_t address){
	uint32_t idx = this->idxSetCalculation(address);
	uint32_t tag = this->tagSetCalculation(address);
	for (size_t i = 0; i < this->nLines; i++)
	{
		if(this->sets[idx].linhas[i].valid==0){
			this->sets[idx].linhas[i].tag = tag;
			this->sets[idx].linhas[i].lru = orcs_engine.get_global_cycle();
			this->sets[idx].linhas[i].valid = 1;
			this->sets[idx].linhas[i].dirty = 0;
			this->sets[idx].linhas[i].readyAt = orcs_engine.get_global_cycle()+LATENCY_TOTAL;
			// ORCS_PRINTF("address %lu ready at %lu\n",address,this->sets[idx].linhas[i].readyAt)
			return &this->sets[idx].linhas[i];
		}
	// ORCS_PRINTF("not valid line\n")
	}
	uint32_t line = this->searchLru(&this->sets[idx]);
	this->add_changeLine();
	// ORCS_PRINTF("line after lru search %u\n",line)
	if(this->sets[idx].linhas[line].dirty==1){
		if(this->level == INST_CACHE){
			this->printLine(&this->sets[idx].linhas[line]);
		}
		this->writeBack(&this->sets[idx].linhas[line]);
		this->add_cacheWriteBack();
		}
	this->sets[idx].linhas[line].tag = tag;
	this->sets[idx].linhas[line].lru = orcs_engine.get_global_cycle();
	this->sets[idx].linhas[line].valid = 1;	
	this->sets[idx].linhas[line].dirty = 0;	
	this->sets[idx].linhas[line].readyAt = orcs_engine.get_global_cycle()+LATENCY_TOTAL;
	// ORCS_PRINTF("address %lu ready at %lu\n",address,this->sets[idx].linhas[line].readyAt)
	return &this->sets[idx].linhas[line];
};
// ===================
// @set - cache set to locate lru
// @return index of line lru 
// ===================
inline uint32_t cache_t::searchLru(cacheSet_t *set){
	uint32_t index=0;
	for (size_t i = 1; i < this->nLines; i++)
	{
		index = (set->linhas[index].lru <= set->linhas[i].lru)? index : i ;
	}
	return index;
};

//====================
//write back
// @1 address - endereco do dado
// @2 linha a ser feito WB
//====================
inline void cache_t::writeBack(linha_t *linha){
	if(this->level == L1){
		ERROR_ASSERT_PRINTF(linha->linha_ptr_sup!=NULL,"Erro, Linha sem referencia a nivel mais alto ")
		//Access pointer to copy status.
		linha->linha_ptr_sup->dirty = linha->dirty;//DIRTY
		linha->linha_ptr_sup->lru = linha->lru;//LRU
		linha->linha_ptr_sup->readyAt = linha->readyAt;//READY_AT
		// Nulling Pointers
		linha->linha_ptr_sup->linha_ptr_inf = NULL;//Pointer to Lower Level
		// invalidando a linha recem feita WB. 
		linha->clean_line();
	}else{
		if(linha->linha_ptr_inf !=NULL){
			linha->linha_ptr_inf->clean_line();//invalidando linha Lower level
		}
		linha->clean_line();
	}
};
//====================
//move line to
// @1 address - endereco do dado
// @2 nivel de cache alvo da mudanca
// @3 *retorno 
//====================
void cache_t::returnLine(uint64_t address,cache_t *cache){
	uint32_t idx = this->idxSetCalculation(address);
	int32_t line=POSITION_FAIL;
	uint32_t tag = this->tagSetCalculation(address);
	// pega a linha desta cache
	for (size_t i = 0; i < this->nLines; i++)
	{
		if(this->sets[idx].linhas[i].tag==tag){
			line = i;
			break;
		}
	}
	ERROR_ASSERT_PRINTF(line!=POSITION_FAIL,"Error, linha LLC não encontrada para retorno")
	linha_t *linha_l1 = NULL;
	linha_l1 = cache->installLine(address);
	this->sets[idx].linhas[line].linha_ptr_inf=linha_l1;
	linha_l1->linha_ptr_sup = &this->sets[idx].linhas[line];
	//copia dados da linha superior
	linha_l1->dirty = linha_l1->linha_ptr_sup->dirty;
	linha_l1->lru = linha_l1->linha_ptr_sup->lru;
	linha_l1->prefetched = linha_l1->linha_ptr_sup->prefetched;
	linha_l1->readyAt = linha_l1->linha_ptr_sup->readyAt;
};
//====================
//move line to
// @1 address - endereco do dado
// @2 nivel de cache alvo da mudanca
// @3 linha a ser movida
// @return index da linha movida
//====================
uint32_t cache_t::moveLineTo(uint64_t address,cache_t *cache, linha_t *linha){
	//calcula endereco na nivel acima
	//cache representa nivel acima
	uint32_t idx = cache->idxSetCalculation(address); //cache idx level up
	uint32_t tag = cache->tagSetCalculation(address);
	uint32_t line=0;
	//busca se ja existe linha naquele nivel.
	for (size_t i = 0; i < cache->nLines; i++)
	{	
		//existindo linha, so copia do outro nivel de cache
		if(cache->sets[idx].linhas[i].tag == tag){
			std::memcpy(&cache->sets[idx].linhas[i],linha,sizeof(linha_t));
			cache->sets[idx].linhas[i].lru = orcs_engine.get_global_cycle();
			return i;
		}
	}
	//Busca se há alguma linha invalida
	for (size_t i = 0; i < cache->nLines; i++)
	{	
		//existindo linha, so copia do outro nivel de cache
		if(cache->sets[idx].linhas[i].valid == 0){
			std::memcpy(&cache->sets[idx].linhas[i],linha,sizeof(linha_t));
			cache->sets[idx].linhas[i].lru = orcs_engine.get_global_cycle();
			return i;
		}
	}
	// não há invalido, e não tem livre, buscando lru
	line = cache->searchLru(&cache->sets[idx]);
	// se sujo, faz WB
	if(cache->sets[idx].linhas[line].dirty==1){
		cache->writeBack(&cache->sets[idx].linhas[line]);
		cache->add_cacheWriteBack();
	}	
	std::memcpy(&cache->sets[idx].linhas[line],linha,sizeof(linha_t));
	cache->sets[idx].linhas[line].tag = tag;
	cache->sets[idx].linhas[line].lru = orcs_engine.get_global_cycle();
	return line;
	// this->printLine(&cache->sets[idx].linhas[line]);
};

// ==========================================
// @address endereco para realizar o shotdown da 
// linha de cache no level 1 quando coerente
// ==========================================
void cache_t::shotdown(uint64_t address){
	uint32_t tag = this->tagSetCalculation(address);
	uint32_t idx = this->idxSetCalculation(address);
	for(size_t i = 0; i < this->nLines; i++){
		if(this->sets[idx].linhas[i].tag == tag){
#if DEBUG	
			printf("shoting down Line %u -> %lu\n ",idx,i);
			this->printLine(&this->sets[idx].linhas[i]);
#endif
			this->sets[idx].linhas[i].valid = 0;
			break;
		}
	}
};
// ====================
// statistics of a level of cache
// ====================
void cache_t::statistics(){
	ORCS_PRINTF("Cache Level: %s\n",get_enum_cache_level_char(this->level))
	ORCS_PRINTF("Cache Access: %lu\n",this->get_cacheAccess())
	ORCS_PRINTF("Cache Hits: %lu %.2f\n",this->get_cacheHit(),float((this->get_cacheHit()*100.0)/this->get_cacheAccess()))
	ORCS_PRINTF("Cache Miss: %lu %.2f\n",this->get_cacheMiss(),float((this->get_cacheMiss()*100.0)/this->get_cacheAccess()))
	ORCS_PRINTF("Cache Read: %lu %.2f\n",this->get_cacheRead(),float((this->get_cacheRead()*100.0)/this->get_cacheAccess()))
	ORCS_PRINTF("Cache Write: %lu %.2f\n",this->get_cacheWrite(),float((this->get_cacheWrite()*100.0)/this->get_cacheAccess()))
	if(this->get_cacheWriteBack()!=0){
		ORCS_PRINTF("Cache WriteBack: %lu %.2f\n",this->get_cacheWriteBack(),float((this->get_cacheWriteBack()*100.0)/this->get_changeLine()))
	}
}