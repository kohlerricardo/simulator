class emc_t{
    
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
        // EMC Methods
        // ==========================================================================
        emc_t();
        ~emc_t();
        void allocate();
        void statistics();
        void core_to_emc(); //Get the uops chain from core
        void dispatch();    //Dispatch uops to ufs 
        void execute();     //get th uops executed
        void emc_to_core(); //Send uops executed to core
        
};