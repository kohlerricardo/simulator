#ifndef EMC_H
#define EMC_H
class emc_t{
    private:
        uint64_t access_LLC;
        uint64_t access_LLC_Hit;
        uint64_t access_LLC_Miss;
    public:

        // ==========================================================================
        // EMC Attr
        // ==========================================================================
        cache_t *data_cache; 
        memory_order_buffer_line_t *unified_lsq; //EMC load store queue
        emc_opcode_package_t *uop_buffer; //uop buffer to store emc operations
        container_ptr_emc_opcode_package_t unified_rs;
        container_ptr_emc_opcode_package_t unified_fus;
        // ==========================================================================
        uint32_t memory_op_executed;
        // ==========================================================================
        // Attr uop Buffer
        uint32_t uop_buffer_start;
        uint32_t uop_buffer_end;
        uint32_t uop_buffer_used;
        // ==========================================================================
        // EMC FUs for execution
        uint64_t *fu_int_alu;
        uint64_t *fu_mem_load;
        uint64_t *fu_mem_store;

        // ==========================================================================
        // control attr 
        bool ready_to_execute;
        bool executed;
        // ==========================================================================
        // EMC Methods
        // ==========================================================================
        emc_t();
        ~emc_t();
        void clock();
        void allocate();
        void statistics();
        // ==========================================================================
        int32_t get_position_uop_buffer(); // get free uop buffer position
        void remove_front_uop_buffer();//remove front of uop buffer
        // ==========================================================================
        void solve_emc_dependencies(emc_opcode_package_t *emc_opcode);
        // ==========================================================================
        void dispatch();    //Dispatch uops to ufs 
        void execute();     //get th uops executed
        void emc_to_core(); //Send uops executed to core
        // ==========================================================================
        // EMC memory interact
        void lsq_read();
        
        INSTANTIATE_GET_SET_ADD(uint64_t,access_LLC);
        INSTANTIATE_GET_SET_ADD(uint64_t,access_LLC_Hit);
        INSTANTIATE_GET_SET_ADD(uint64_t,access_LLC_Miss);
};

#endif // !EMC_H