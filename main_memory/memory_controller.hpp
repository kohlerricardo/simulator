#ifndef MEMORY_CONTROLLER_H
#define MEMORY_CONTROLLER_H

class memory_controller_t{

    private:
        uint64_t requests_made;
    public:
        memory_controller_t();
        ~memory_controller_t();
        
        // ==========================================================================
        // EMC Attr
        // ==========================================================================
        
        uint64_t *unified_fus_emc;
        cache_t *data_cache; 
        // ==========================================================================
        // EMC Methods
        // ==========================================================================
        void allocate();    //Aloca recursos do EMC
        void core_to_emc(); //Get the uops chain from core
        void dispatch();    //Dispatch uops to ufs 
        void execute();     //get th uops executed
        void emc_to_core(); //Send uops executed to core
        // ==========================================================================

        void clock();
        void statistics();
        INSTANTIATE_GET_SET_ADD(uint64_t,requests_made)
       
        
};









#endif // MEMORY_CONTROLLER_H