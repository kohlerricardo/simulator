#ifndef CACHE_MANAGER_H
#define CACHE_MANAGER_H
class cache_manager_t{
    private:
        uint64_t instructionSearched;
        uint64_t instructionLLCSearched;
        uint64_t miss;
        uint64_t hit;
        
        std::priority_queue<memory_order_buffer_line_t*, std::vector<memory_order_buffer_line_t*>,priority_memory_access_t> read_buffer;
        std::priority_queue<memory_order_buffer_line_t*, std::vector<memory_order_buffer_line_t*>,priority_memory_access_t> write_buffer;
        
    public:
        cache_t *data_cache;
        cache_t *inst_cache;

        cache_manager_t();
        ~cache_manager_t();
        void allocate();
        void clock();//for prefetcher
        void statistics();
        uint32_t searchInstruction(uint64_t instructionAddress);
        uint32_t searchData(memory_order_buffer_line_t *mob_line);
        uint32_t writeData(memory_order_buffer_line_t *mob_line);
        void insertQueueRead(memory_order_buffer_line_t* mob_line);
        void insertQueueWrite(memory_order_buffer_line_t* mob_line);
        INSTANTIATE_GET_SET_ADD(uint64_t,instructionSearched);
        INSTANTIATE_GET_SET_ADD(uint64_t,instructionLLCSearched);
        INSTANTIATE_GET_SET_ADD(uint64_t,hit);
        INSTANTIATE_GET_SET_ADD(uint64_t,miss);
        // ==========================================
        // Prefetcher
        // ==========================================
        #if PREFETCHER_ACTIVE
        prefetcher_t *prefetcher;
        #endif 
        
};  

#endif // !CACHE_MANAGER_H