#include "../../simulator.hpp"

disambiguation_perfect_t::disambiguation_perfect_t(){
	this->stat_address_to_address=0;
	this->stat_disambiguation_read_false_positive=0;
	this->stat_disambiguation_write_false_positive = 0;
};
disambiguation_perfect_t::~disambiguation_perfect_t(){
};
void disambiguation_perfect_t::allocate(){
};
void disambiguation_perfect_t::make_memory_dependencies(memory_order_buffer_line_t *mob_line){
	//makes dependencies Read After Write (RAW)
	if(mob_line->memory_operation == MEMORY_OPERATION_READ){
		for (uint16_t i = orcs_engine.processor->memory_order_buffer_write_start;; i++){
			if(i == orcs_engine.processor->memory_order_buffer_write_end)break;
			if(i >= MOB_WRITE) i=0;
			if((mob_line->memory_address == orcs_engine.processor->memory_order_buffer_write[i].memory_address)){
					mob_line->wait_mem_deps_number++;
					for(size_t j = 0; j < ROB_SIZE; j++){
						if(orcs_engine.processor->memory_order_buffer_write[i].mem_deps_ptr_array[j]==NULL){
							orcs_engine.processor->memory_order_buffer_write[i].mem_deps_ptr_array[j]=mob_line;
							break;
						}
					}//end for
					
				}//end add and age comp.
		}//end for
	}//end read dependences
	else if (mob_line->memory_operation == MEMORY_OPERATION_WRITE){
		//make dependencies Write After Read (WAR) and Write After Write(WAW)
		// Write After Read (WAR)
		for (uint16_t i = orcs_engine.processor->memory_order_buffer_read_start;; i++){
			if(i == orcs_engine.processor->memory_order_buffer_read_end)break;
			if(i >= MOB_READ) i=0;
			if((mob_line->memory_address == orcs_engine.processor->memory_order_buffer_read[i].memory_address)){
					mob_line->wait_mem_deps_number++;
					for(size_t j = 0; j < ROB_SIZE; j++){
						if(orcs_engine.processor->memory_order_buffer_read[i].mem_deps_ptr_array[j]==NULL){
							orcs_engine.processor->memory_order_buffer_read[i].mem_deps_ptr_array[j]=mob_line;
							break;
						}
					}//end for
					
				}//end add and age comp.
		}//end for
		// Write After Write (WAW)
		for (uint16_t i = orcs_engine.processor->memory_order_buffer_write_start;; i++){
					if(i == orcs_engine.processor->memory_order_buffer_write_end)break;
					if(i >= MOB_WRITE) i=0;
					if((mob_line->memory_address == orcs_engine.processor->memory_order_buffer_write[i].memory_address)){
							mob_line->wait_mem_deps_number++;
							for(size_t j = 0; j < ROB_SIZE; j++){
								if(orcs_engine.processor->memory_order_buffer_write[i].mem_deps_ptr_array[j]==NULL){
									orcs_engine.processor->memory_order_buffer_write[i].mem_deps_ptr_array[j]=mob_line;
									break;
								}
							}//end for
							
						}//end add and age comp.
				}//end for
	}//end write  dependences
};
void disambiguation_perfect_t::solve_memory_dependencies(memory_order_buffer_line_t *mob_line){
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
		mob_line->mem_deps_ptr_array[j]->wait_mem_deps_number--;
		mob_line->mem_deps_ptr_array[j] = NULL;
	}
	if (ADDRESS_TO_ADDRESS == 1){	
		for(uint i=orcs_engine.processor->memory_order_buffer_read_start;;i++){
			if(i == orcs_engine.processor->memory_order_buffer_read_end)break;
			if(i >= MOB_READ) i=0;
			if (orcs_engine.processor->memory_order_buffer_read[i].uop_executed == true &&
			orcs_engine.processor->memory_order_buffer_read[i].wait_mem_deps_number == 0 &&
			orcs_engine.processor->memory_order_buffer_read[i].memory_address == mob_line->memory_address &&
			orcs_engine.processor->memory_order_buffer_read[i].memory_size == mob_line->memory_size){
				this->add_stat_address_to_address();
				orcs_engine.processor->memory_order_buffer_read[i].status = PACKAGE_STATE_READY;
				orcs_engine.processor->memory_order_buffer_read[i].readyAt = orcs_engine.get_global_cycle() + REGISTER_FORWARD;
			}
		}
	}
};
void disambiguation_perfect_t::statistics(){FILE *output = stdout;
	if(orcs_engine.output_file_name != NULL)
		output = fopen(orcs_engine.output_file_name,"a+");
	if (output != NULL){
            utils_t::largeSeparator(output);
            fprintf(output,"Total_Read_false_Positives: %lu\n", this->get_stat_disambiguation_read_false_positive());
            fprintf(output,"Total_Write_false_Positives: %lu\n", this->get_stat_disambiguation_write_false_positive());
            fprintf(output,"Total_Resolve_Address_to_Address: %lu\n",this->get_stat_address_to_address());
            utils_t::largeSeparator(output);
        }
        fclose(output);
};