#ifndef CACHE_MANAGER_H
#define CACHE_MANAGER_H
class cache_manager_t{
    private:
        uint64_t instructionSearched;
        uint64_t instructionLLCSearched;
        uint64_t readMiss;
        uint64_t readHit;
        uint64_t writeMiss;
        uint64_t writeHit;
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
        INSTANTIATE_GET_SET_ADD(uint64_t,readMiss);
        INSTANTIATE_GET_SET_ADD(uint64_t,readHit);
        INSTANTIATE_GET_SET_ADD(uint64_t,writeMiss);
        INSTANTIATE_GET_SET_ADD(uint64_t,writeHit);
        // ==========================================
        // Prefetcher
        // ==========================================
        #if PREFETCHER_ACTIVE
        prefetcher_t *prefetcher;
        #endif 
        // ==========================================
        // EMC Data Collect
        // ==========================================

        uint32_t search_EMC_Data(memory_order_buffer_line_t *mob_line);

};  

#endif // !CACHE_MANAGER_H