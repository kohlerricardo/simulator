#include"../../simulator.hpp"
// 
disambiguation_hashed_t::disambiguation_hashed_t(/* args */)
{
    // ==============================================================
    // HASHED LOAD/STORE
    // ==============================================================
    this->disambiguation_load_hash = NULL;
 
    // ==============================================================
    this->disambiguation_store_hash = NULL;
    
}  

disambiguation_hashed_t::~disambiguation_hashed_t()
{  
    // ==============================================================
    utils_t::template_delete_array<memory_order_buffer_line_t *>(this->disambiguation_load_hash);
	utils_t::template_delete_array<memory_order_buffer_line_t *>(this->disambiguation_store_hash);
}

void disambiguation_hashed_t::allocate(){
    // LOAD/STORE Hash mask
    this->disambiguation_load_hash_bits_mask = 0;
    this->disambiguation_store_hash_bits_mask = 0;
    // LOAD/STORE Hash shift
    this->disambiguation_load_hash_bits_shift = 0;
    this->disambiguation_store_hash_bits_shift = 0;
    // =========================================================================================
	// DISAMBIGUATION 
	// =========================================================================================
	ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(LOAD_HASH_SIZE), "Wrong disambiguation_load_hash_size.\n")
	for (uint32_t i = 0; i < utils_t::get_power_of_two(LOAD_HASH_SIZE); i++)
	{
		this->disambiguation_load_hash_bits_mask |= 1 << i;
	}
	this->disambiguation_load_hash_bits_shift = utils_t::get_power_of_two(DESAMBIGUATION_BLOCK_SIZE);
	this->disambiguation_load_hash_bits_mask <<= this->disambiguation_load_hash_bits_shift;
	this->disambiguation_load_hash = utils_t::template_allocate_initialize_array<memory_order_buffer_line_t *>(LOAD_HASH_SIZE, NULL);

	/// DISAMBIGUATION OFFSET MASK
	ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(STORE_HASH_SIZE), "Wrong disambiguation_store_hash_size.\n")
	for (uint32_t i = 0; i < utils_t::get_power_of_two(STORE_HASH_SIZE); i++)
	{
		this->disambiguation_store_hash_bits_mask |= 1 << i;
	}
	this->disambiguation_store_hash_bits_shift <<= utils_t::get_power_of_two(DESAMBIGUATION_BLOCK_SIZE);
	this->disambiguation_store_hash_bits_mask <<= this->disambiguation_store_hash_bits_shift;
	this->disambiguation_store_hash = utils_t::template_allocate_initialize_array<memory_order_buffer_line_t *>(STORE_HASH_SIZE, NULL);

}
void disambiguation_hashed_t::make_memory_dependencies(memory_order_buffer_line_t *new_mob_line){

	uint64_t load_hash = new_mob_line->memory_address & this->disambiguation_load_hash_bits_mask;
	uint64_t store_hash = new_mob_line->memory_address & this->disambiguation_store_hash_bits_mask;
	load_hash >>= this->disambiguation_load_hash_bits_shift;
	store_hash >>= this->disambiguation_store_hash_bits_shift;
	// ORCS_PRINTF("Memory address -> %lu, load_hash %lu, store_hash %lu\n",new_mob_line->memory_address,load_hash,store_hash)

	memory_order_buffer_line_t *old_mob_line = NULL;

	/// Check if LOAD_HASH matches
	ERROR_ASSERT_PRINTF(load_hash < LOAD_HASH_SIZE, "load_hash (%lu) > disambiguation_load_hash_size (%d)\n",
						load_hash, LOAD_HASH_SIZE);
	/// Check if STORE_HASH matches
	ERROR_ASSERT_PRINTF(store_hash < STORE_HASH_SIZE, "store_hash (%lu) > disambiguation_store_hash_size (%d)\n",
						store_hash, STORE_HASH_SIZE);
	/// Create R -> W,  R -> R
	if (this->disambiguation_load_hash[load_hash] != NULL)
	{
		old_mob_line = disambiguation_load_hash[load_hash];
		for (uint32_t k = 0; k < ROB_SIZE; k++)
		{
			if (old_mob_line->mem_deps_ptr_array[k] == NULL)
			{
				old_mob_line->mem_deps_ptr_array[k] = new_mob_line;
				new_mob_line->wait_mem_deps_number++;
				break;
			}
		}
	}

	/// Create W -> R, W -> W deps.
	if (this->disambiguation_store_hash[store_hash] != NULL)
	{
		old_mob_line = disambiguation_store_hash[store_hash];
		for (uint32_t k = 0; k < ROB_SIZE; k++)
		{
			if (old_mob_line->mem_deps_ptr_array[k] == NULL)
			{
				old_mob_line->mem_deps_ptr_array[k] = new_mob_line;
				new_mob_line->wait_mem_deps_number++;
				break;
			}
		}
	}

	/// Add the new entry into LOAD or STORE hash
	if (new_mob_line->memory_operation == MEMORY_OPERATION_READ)
	{
		this->disambiguation_load_hash[load_hash] = new_mob_line;
	}
	else
	{
		this->disambiguation_store_hash[store_hash] = new_mob_line;
	}
};
void disambiguation_hashed_t::solve_memory_dependencies(memory_order_buffer_line_t *mob_line){

	/// Remove pointers from disambiguation_hash
	/// Add the new entry into LOAD or STORE hash
	if (mob_line->memory_operation == MEMORY_OPERATION_READ)
	{
		uint64_t load_hash = mob_line->memory_address & this->disambiguation_load_hash_bits_mask;
		load_hash >>= this->disambiguation_load_hash_bits_shift;

		ERROR_ASSERT_PRINTF(load_hash < LOAD_HASH_SIZE, "load_hash (%" PRIu64 ") > disambiguation_load_hash_size (%d)\n",
							load_hash, LOAD_HASH_SIZE);
		if (this->disambiguation_load_hash[load_hash] == mob_line)
		{
			this->disambiguation_load_hash[load_hash] = NULL;
		}
	}
	else
	{
		uint64_t store_hash = mob_line->memory_address & this->disambiguation_store_hash_bits_mask;
		store_hash >>= this->disambiguation_store_hash_bits_shift;

		ERROR_ASSERT_PRINTF(store_hash < STORE_HASH_SIZE, "store_hash (%" PRIu64 ") > disambiguation_store_hash_size (%d)\n",
							store_hash, STORE_HASH_SIZE);

		if (this->disambiguation_store_hash[store_hash] == mob_line)
		{
			this->disambiguation_store_hash[store_hash] = NULL;
		}
	}

	// =========================================================================
	/// SOLVE MEMORY DEPENDENCIES - MOB
	// =========================================================================
	/// Send message to acknowledge the dependency is over
	for (uint32_t j = 0; j < ROB_SIZE; j++)
	{
		/// All the dependencies are solved
		if (mob_line->mem_deps_ptr_array[j] == NULL)
		{
			break;
		}

		/// Keep track of false positives
		if (mob_line->mem_deps_ptr_array[j]->memory_address != mob_line->memory_address)
		{
			if (mob_line->memory_operation == MEMORY_OPERATION_READ)
			{
				this->add_stat_disambiguation_read_false_positive();
			}
			else
			{
				this->add_stat_disambiguation_write_false_positive();
			}
		}

		/// There is an unsolved dependency
		mob_line->mem_deps_ptr_array[j]->wait_mem_deps_number--;

		if (ADDRESS_TO_ADDRESS == 1)
		{
			if (mob_line->mem_deps_ptr_array[j]->uop_executed == true &&
				mob_line->mem_deps_ptr_array[j]->wait_mem_deps_number == 0 &&
				mob_line->mem_deps_ptr_array[j]->memory_operation == MEMORY_OPERATION_READ &&
				mob_line->mem_deps_ptr_array[j]->memory_address == mob_line->memory_address &&
				mob_line->mem_deps_ptr_array[j]->memory_size == mob_line->memory_size)
			{
				this->add_stat_address_to_address();
				mob_line->mem_deps_ptr_array[j]->status = PACKAGE_STATE_READY;
				mob_line->mem_deps_ptr_array[j]->sent = true;
				mob_line->mem_deps_ptr_array[j]->rob_ptr->sent = true;
				mob_line->mem_deps_ptr_array[j]->readyAt = orcs_engine.get_global_cycle() + REGISTER_FORWARD;
				mob_line->mem_deps_ptr_array[j]->forwarded_data=true;
				#if MOB_DEBUG
					ORCS_PRINTF("Forwarded : %s\n",mob_line->mem_deps_ptr_array[j]->content_to_string().c_str())
				#endif

			}
		}
		/// This update the ready cycle, and it is usefull to compute the time each instruction waits for the functional unit
		mob_line->mem_deps_ptr_array[j] = NULL;
	}
};

void disambiguation_hashed_t::statistics(){
    if(orcs_engine.output_file_name == NULL){
    
    ORCS_PRINTF("Total_Read_false_Positives: %lu\n", this->get_stat_disambiguation_read_false_positive())
    ORCS_PRINTF("Total_Write_false_Positives: %lu\n", this->get_stat_disambiguation_write_false_positive())
    ORCS_PRINTF("Total_Resolve_Address_to_Address: %lu\n",this->get_stat_address_to_address())
    }
    else{
        FILE *output = fopen(orcs_engine.output_file_name,"a+");
		if(output != NULL){
            utils_t::largeSeparator(output);
            fprintf(output,"Total_Read_false_Positives: %lu\n", this->get_stat_disambiguation_read_false_positive());
            fprintf(output,"Total_Write_false_Positives: %lu\n", this->get_stat_disambiguation_write_false_positive());
            fprintf(output,"Total_Resolve_Address_to_Address: %lu\n",this->get_stat_address_to_address());
            utils_t::largeSeparator(output);
        }
        fclose(output);
    }
};
// ============================================================================