#ifndef MEMORY_CONTROLLER_H
#define MEMORY_CONTROLLER_H

class memory_controller_t{

    private:
        // Statistics DRAM
        uint64_t requests_made; //Data Requests made
        uint64_t operations_executed; // number of operations executed
        uint64_t requests_emc; //Data Requests made to DRAM
        uint64_t requests_llc; //Data Requests made to LLC
        uint64_t row_buffer_miss; //Data Requests made to LLC
        
        // =================================================
        // attr DRAM
        // =================================================
        uint64_t channel_bits_mask;
        uint64_t rank_bits_mask;
        uint64_t bank_bits_mask;
        uint64_t row_bits_mask;
        uint64_t col_row_bits_mask;
        uint64_t col_byte_bits_mask;
        uint64_t latency_burst;
        // Shifts bits
        uint64_t channel_bits_shift;
        uint64_t colbyte_bits_shift;
        uint64_t colrow_bits_shift;
        uint64_t bank_bits_shift;
        uint64_t row_bits_shift;

        // Struct object defines RAM
        typedef struct RAM{
            uint64_t last_row_accessed;
            uint64_t cycle_ready;
        }RAM_t;
        
    public:
        // ==========================================================================
        // Memory Controller Atributes
        // ==========================================================================
        emc_t *emc;
        uint8_t emc_active;
        cache_t *data_cache;
        RAM_t *ram; 
        // ==========================================================================
        // Methods DRAM
        // ==========================================================================
        void set_masks();
        // ==========================================================================
        // Memory Controller Methods
        // ==========================================================================
        void allocate();    //Aloca recursos do Memory Controller
        // Get channel to access DATA
        inline  uint64_t get_channel(uint64_t address){
            return (address&this->channel_bits_mask)>>this->channel_bits_shift;
        }
        // get memory bank accessed
        inline  uint64_t get_bank(uint64_t address){
            return (address&this->bank_bits_mask)>>this->bank_bits_shift;
        }
        //get row accessed
        inline uint64_t get_row(uint64_t address){
            return (address&this->row_bits_mask)>>this->row_bits_shift;
        }
        // ==========================================================================
        memory_controller_t();
        ~memory_controller_t();
        void clock();
        void statistics();
        //statistiscs methods
        INSTANTIATE_GET_SET_ADD(uint64_t,requests_made)
        INSTANTIATE_GET_SET_ADD(uint64_t,operations_executed)
        INSTANTIATE_GET_SET_ADD(uint64_t,requests_emc)
        INSTANTIATE_GET_SET_ADD(uint64_t,requests_llc)
        INSTANTIATE_GET_SET_ADD(uint64_t,row_buffer_miss)
        //request DRAM data
        uint64_t requestDRAM(uint64_t address);
        
};









#endif // MEMORY_CONTROLLER_H