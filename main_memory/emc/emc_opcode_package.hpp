class emc_opcode_package_t{
    public:
        uop_package_t uop;                          /// opcode to be executed
        memory_order_buffer_line_t *mob_ptr;             /// entry to MOB Line of EMC
        uint32_t wait_reg_deps_number;              /// Must wait BEFORE execution
        emc_opcode_package_t* *reg_deps_ptr_array;  /// Elements to wake-up AFTER execution
        uint32_t wake_up_elements_counter;          /// Counter of elements to Wake Up
        reorder_buffer_line_t *rob_ptr;             /// Pointer to ROB entry for accelerate return to core
        
        // ============================================================================
        emc_opcode_package_t();
        ~emc_opcode_package_t();
        void package_clean();
        std::string content_to_string();
        // ============================================================================

};