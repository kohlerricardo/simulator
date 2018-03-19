#ifndef CACHE_MANAGER_H
#define CACHE_MANAGER_H
class cache_manager_t{
    private:
        uint64_t instructionSearched;
        uint64_t instructionLLCSearched;
        uint64_t dataSearched;
        
    public:
        cache_t *data_cache;
        cache_t *inst_cache;

        cache_manager_t();
        ~cache_manager_t();
        void allocate();
        void clock();//for prefetcher
        uint32_t searchInstruction(uint64_t instructionAddress);
        uint32_t searchData(uint64_t dataAddress);
        uint32_t writeData(uint64_t dataAddress);
        INSTANTIATE_GET_SET_ADD(uint64_t,instructionSearched);
        INSTANTIATE_GET_SET_ADD(uint64_t,instructionLLCSearched);
        INSTANTIATE_GET_SET_ADD(uint64_t,dataSearched);
};

#endif // !CACHE_MANAGER_H