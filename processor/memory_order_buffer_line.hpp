class memory_order_buffer_line_t {
    public:
        uint64_t opcode_address;
        uint64_t memory_address;
        uint32_t memory_size;
         
        reorder_buffer_line_t* rob_ptr;                 /// rob pointer

        /// Memory Dependencies Control
        bool uop_executed;
        uint64_t uop_number;
        uint64_t readyAt;        
        package_state_t status;
        memory_operation_t memory_operation;
        uint64_t born_cicle;
        uint32_t wait_mem_deps_number;                  /// Must wait BEFORE execution
        memory_order_buffer_line_t* *mem_deps_ptr_array;     /// Elements to wake-up AFTER execution
        // ====================================================================
        /// Methods
        // ====================================================================
        memory_order_buffer_line_t();
        ~memory_order_buffer_line_t();

        void package_clean();
        std::string content_to_string();

        static int32_t find_free(memory_order_buffer_line_t *input_array, uint32_t size_array);
        static int32_t find_old_request_state_ready(memory_order_buffer_line_t *input_array, uint32_t size_array, package_state_t state);
        void  print_all_operation_deps(uint64_t &ld_llc_miss,uint64_t &llc_miss_deps,uint64_t &inst_ld_ld);
        // =====================================================================
        // Update status
        // =====================================================================
        void updatePackageUntrated(uint32_t stallTime);
        void updatePackageReady(uint32_t stallTime);
        void updatePackageWait(uint32_t stallTime);
        void updatePackageFree(uint32_t stallTime);
};
