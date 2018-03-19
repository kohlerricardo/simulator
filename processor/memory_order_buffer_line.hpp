class memory_order_buffer_line_t {
    public:
        uint64_t opcode_address;
        uint64_t memory_address;
        uint32_t memory_size;
         
        reorder_buffer_line_t* rob_ptr;                 /// rob pointer

        /// Memory Dependencies Control
        bool uop_executed;
        uint32_t readyAt;        
        package_state_t status;
        memory_operation_t memory_operation;
        uint64_t born_cicle;
        // ====================================================================
        /// Methods
        // ====================================================================
        memory_order_buffer_line_t();
        ~memory_order_buffer_line_t();

        void package_clean();
        std::string content_to_string();

        static int32_t find_free(memory_order_buffer_line_t *input_array, uint32_t size_array);
        static int32_t find_old_request_state_ready(memory_order_buffer_line_t *input_array, uint32_t size_array, package_state_t state);
        static std::string print_all(memory_order_buffer_line_t *input_array, uint32_t size_array);
        // =====================================================================
        // Update status
        // =====================================================================
        void updatePackageUntrated(uint32_t stallTime);
        void updatePackageReady(uint32_t stallTime);
        void updatePackageWait(uint32_t stallTime);
        void updatePackageFree(uint32_t stallTime);
};
