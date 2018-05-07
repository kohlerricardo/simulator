#ifndef MEMORY_CONTROLLER_H
#define MEMORY_CONTROLLER_H

class memory_controller_t{

    private:
        uint64_t requests_made; //Data Requests made
        uint64_t operations_executed; // number of operations executed
        uint64_t requests_dram; //Data Requests made to DRAM
        uint64_t requests_llc; //Data Requests made to LLC
        
    public:
        // ==========================================================================
        // Memory Controller Atributes
        // ==========================================================================
        emc_t *emc;



        // ==========================================================================
        // Memory Controller Methods
        // ==========================================================================
        void allocate();    //Aloca recursos do Memory Controller
        // ==========================================================================
        memory_controller_t();
        ~memory_controller_t();
        void clock();
        void statistics();
        INSTANTIATE_GET_SET_ADD(uint64_t,requests_made)
        INSTANTIATE_GET_SET_ADD(uint64_t,operations_executed)
        INSTANTIATE_GET_SET_ADD(uint64_t,requests_dram)
        INSTANTIATE_GET_SET_ADD(uint64_t,requests_llc)

       
        
};









#endif // MEMORY_CONTROLLER_H