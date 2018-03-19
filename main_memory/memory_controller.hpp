#ifndef MEMORY_CONTROLLER_H
#define MEMORY_CONTROLLER_H

class memory_controller_t{

    private:
        uint64_t requests_made;
        uint64_t requests_prefetcher;
        uint64_t requests_duplicated;
    public:
        memory_controller_t();
        ~memory_controller_t();
        void clock();
        void statistics();
        INSTANTIATE_GET_SET_ADD(uint64_t,requests_made)
        INSTANTIATE_GET_SET_ADD(uint64_t,requests_prefetcher)
        INSTANTIATE_GET_SET_ADD(uint64_t,requests_duplicated)
        
};









#endif // MEMORY_CONTROLLER_H