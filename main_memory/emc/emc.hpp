#ifndef EMC_H
#define EMC_H
class emc_t{
    private:
        uint64_t access_LLC;
        uint64_t access_LLC_Hit;
        uint64_t access_LLC_Miss;
    //=============
	//Statistics Commit
	//=============
	uint64_t stat_inst_int_alu_completed;
	uint64_t stat_inst_mul_alu_completed;
	uint64_t stat_inst_div_alu_completed;
	uint64_t stat_inst_int_fp_completed;
	uint64_t stat_inst_mul_fp_completed;
	uint64_t stat_inst_div_fp_completed;
	uint64_t stat_inst_nop_completed;
	uint64_t stat_inst_load_completed;
	uint64_t stat_inst_store_completed;
	uint64_t stat_inst_branch_completed;
	uint64_t stat_inst_other_completed;

    public:

        // ==========================================================================
        // EMC Attr
        // ==========================================================================
        memory_order_buffer_line_t *unified_lsq; //EMC load store queue
        emc_opcode_package_t *uop_buffer; //uop buffer to store emc operations
        container_ptr_emc_opcode_package_t unified_rs;
        container_ptr_emc_opcode_package_t unified_fus;
        // ==========================================================================
        circular_buffer_t<emc_opcode_package_t> uop_wait_finish;
        // ==========================================================================
        uint32_t memory_op_executed;
        // ==========================================================================
        // Attr uop Buffer
        uint32_t uop_buffer_start;
        uint32_t uop_buffer_end;
        uint32_t uop_buffer_used;
        // ======================
		// Funcional Unitis - FUs
		// ======================
		// Integer FUs
		uint64_t *fu_int_alu;
		uint64_t *fu_int_mul;
		uint64_t *fu_int_div;
		// Floating Points FUs
		uint64_t *fu_fp_alu;
		uint64_t *fu_fp_mul;
		uint64_t *fu_fp_div;
		// Memory FUs
		uint64_t *fu_mem_load;
		uint64_t *fu_mem_store;
        // ==========================================================================
        // Structures to predictor
        int8_t *memory_access_counter_table;
        uint32_t mact_bits_mask;
        void update_mact_entry(uint64_t pc,int32_t value);
        uint64_t direct_ram_access;
        uint64_t emc_llc_access;
        uint64_t incorrect_prediction_ram_access;
        uint64_t incorrect_prediction_LLC_access;
        INSTANTIATE_GET_SET_ADD(uint64_t,direct_ram_access)
        INSTANTIATE_GET_SET_ADD(uint64_t,emc_llc_access)
        INSTANTIATE_GET_SET_ADD(uint64_t,incorrect_prediction_ram_access)
        INSTANTIATE_GET_SET_ADD(uint64_t,incorrect_prediction_LLC_access)
        // ==========================================================================
        // control attr 
        bool ready_to_execute;
        bool executed;
        bool has_store;
        uint32_t processor_id;
        INSTANTIATE_GET_SET(uint32_t,processor_id)
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
        void emc_dispatch();    //Dispatch uops to ufs 
        void emc_execute();     //get th uops executed
        void emc_commit();      //Send uops executed to core
        // ==========================================================================
        // EMC memory interact
        void lsq_read();
        void lsq_forward(memory_order_buffer_line_t *emc_mob_line);
        // ==========================================================================
        //EMC Core Interact
        void emc_send_back_core(emc_opcode_package_t *emc_opcode);
        // void emc_send_back_core();
        INSTANTIATE_GET_SET_ADD(uint64_t,access_LLC)
        INSTANTIATE_GET_SET_ADD(uint64_t,access_LLC_Hit)
        INSTANTIATE_GET_SET_ADD(uint64_t,access_LLC_Miss)
        
        // ====================================================================
		// Statistics inst completed
		// ====================================================================
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_branch_completed)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_div_alu_completed)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_div_fp_completed)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_int_alu_completed)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_int_fp_completed)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_mul_alu_completed)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_mul_fp_completed)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_load_completed)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_store_completed)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_nop_completed)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_other_completed)

        // ==========================================================================
        // EMC Debug Methods
        // ==========================================================================
        void print_structures();
        // ============================================================================
        // Functions and attr for ORACLE
        // ============================================================================
        uint32_t oracle_emc_data_cache_misses;
        uint32_t oracle_emc_LLC_misses;
        INSTANTIATE_GET_SET_ADD(uint64_t,oracle_emc_data_cache_misses)
		INSTANTIATE_GET_SET_ADD(uint64_t,oracle_emc_LLC_misses)
        void oracle_access_emc(memory_order_buffer_line_t *emc_mob_line);
};

#endif // !EMC_H