#include "./../simulator.hpp"
#include <string>
// ============================================================================
memory_controller_t::memory_controller_t(){

    this->emc = NULL;
	// Data Cache
	this->data_cache = NULL;
}
// ============================================================================
memory_controller_t::~memory_controller_t() = default;
// ============================================================================

// ============================================================================
// @allocate objects to EMC
void memory_controller_t::allocate(){
    // ======================= data cache =======================
	this->data_cache = new cache_t;
	this->data_cache->allocate(EMC_DATA_CACHE);

    // ======================= EMC =======================
    #if EMC_ACTIVE
    this->emc = new emc_t[NUMBER_OF_PROCESSORS];
    for (uint32_t i = 0; i < NUMBER_OF_PROCESSORS; i++)
    {
        this->emc[i].allocate();
    }
    #endif
    // ======================= Configurando DRAM ======================= 
    this->latency_burst = LINE_SIZE/BURST_WIDTH;
}
// ============================================================================
void memory_controller_t::statistics(){
    FILE *output = stdout;
    bool close = false;
	if(orcs_engine.output_file_name != NULL){
        close=true;
		output = fopen(orcs_engine.output_file_name,"a+");
    }
	if (output != NULL){
        utils_t::largestSeparator(output);
        fprintf(output,"#Memory Controller\n");
        utils_t::largestSeparator(output);
        fprintf(output,"Requests_Made: %lu\n",this->get_requests_made());
        fprintf(output,"Requests_from_LLC: %lu\n",this->get_requests_llc());
        fprintf(output,"Requests_from_EMC: %lu\n",this->get_requests_emc());
        utils_t::largestSeparator(output);
        if(close) fclose(output);
        #if EMC_ACTIVE
            for (uint32_t i = 0; i < NUMBER_OF_PROCESSORS; i++)
            {
                this->emc[i].statistics();
            }
            utils_t::largestSeparator(output);
            fprintf(output, "##############  EMC_Data_Cache ##################\n");
            this->data_cache->statistics();
        #endif
        }
}
// ============================================================================
void memory_controller_t::clock(){
    #if EMC_ACTIVE
        for (uint32_t i = 0; i < NUMBER_OF_PROCESSORS; i++)
        {
            this->emc[i].clock();        
        }
    #endif

}
// ============================================================================
void memory_controller_t::set_masks(){
            
            uint32_t channel_bits_shift,colbyte_bits_shift,colrow_bits_shift,bank_bits_shift,row_bits_shift=0;
ERROR_ASSERT_PRINTF(this->get_total_controllers() == 1,
                                "Wrong number of memory_controllers (%u).\n", this->get_total_controllers());
            ERROR_ASSERT_PRINTF(this->get_channels_per_controller() > 1 &&
                                utils_t::check_if_power_of_two(this->get_channels_per_controller()),
                                "Wrong number of memory_channels (%u).\n", this->get_channels_per_controller());

            this->controller_bits_shift = 0;
            this->colbyte_bits_shift = 0;
            this->channel_bits_shift = utils_t::get_power_of_two(this->get_line_size());
            this->colrow_bits_shift = this->channel_bits_shift + utils_t::get_power_of_two(this->get_channels_per_controller());
            this->bank_bits_shift = this->colrow_bits_shift + utils_t::get_power_of_two(this->get_bank_row_buffer_size() / this->get_line_size());
            this->row_bits_shift = this->bank_bits_shift + utils_t::get_power_of_two(this->get_bank_per_channel());

            /// COLBYTE MASK
            for (i = 0; i < utils_t::get_power_of_two(this->get_line_size()); i++) {
                this->colbyte_bits_mask |= 1 << (i + this->colbyte_bits_shift);
            }

            /// CHANNEL MASK
            for (i = 0; i < utils_t::get_power_of_two(this->get_channels_per_controller()); i++) {
                this->channel_bits_mask |= 1 << (i + channel_bits_shift);
            }

            /// COLROW MASK
            for (i = 0; i < utils_t::get_power_of_two(this->get_bank_row_buffer_size() / this->get_line_size()); i++) {
                this->colrow_bits_mask |= 1 << (i + this->colrow_bits_shift);
            }

            this->not_column_bits_mask = ~(colbyte_bits_mask | colrow_bits_mask);


            /// BANK MASK
            for (i = 0; i < utils_t::get_power_of_two(this->get_bank_per_channel()); i++) {
                this->bank_bits_mask |= 1 << (i + bank_bits_shift);
            }

            /// ROW MASK
            for (i = row_bits_shift; i < utils_t::get_power_of_two((uint64_t)INT64_MAX+1); i++) {
                this->row_bits_mask |= 1 << i;
            }
}
// ============================================================================
uint64_t memory_controller_t::requestDRAM(){
    this->add_requests_made();
    return RAM_LATENCY;
}
// ============================================================================
