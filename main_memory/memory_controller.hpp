#ifndef MEMORY_CONTROLLER_H
#define MEMORY_CONTROLLER_H

class memory_controller_t{

    private:
        uint64_t requests_made;
    public:
        memory_controller_t();
        ~memory_controller_t();
        
        void clock();
        void statistics();
        INSTANTIATE_GET_SET_ADD(uint64_t,requests_made)
       
        
};









#endif // MEMORY_CONTROLLER_H