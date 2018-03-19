#ifndef CACHE_H
#define CACHE_H


class cache_t
{

     private:
        //=============
        // Statistics related
        //=============
        uint32_t cacheHit;
        uint32_t cacheMiss;
        uint32_t cacheAccess;
        uint32_t cacheRead;
        uint32_t cacheWrite;
        uint32_t cacheWriteBack;
    public:
        cache_t();
        ~cache_t();
        //atributtes
        uint32_t id;
        cacheLevel_t level;
        uint32_t nSets;
        uint32_t nLines;
        cacheSet_t *sets;
        uint32_t shiftData;
        //====================
        // Debug functions - Utils
        //====================
        inline void printLine(linha_t *linha);
        inline void printCacheConfiguration();
        // ============================================================================
        // Functions with void return
        // ============================================================================
        void statistics();
        void allocate(cacheLevel_t level);//allocate data structure
        void shotdown(uint64_t address);//shotdown line inclusive cache
        void writeBack(uint64_t address,linha_t *line); //makes writeback of line
        void returnLine(uint64_t address,cache_t *cache,int32_t &retorno);//return line from lower cache level
        // ============================================================================
        // Functions with uint return
        // ============================================================================
        uint32_t idxSetCalculation(uint64_t address);//calculate index of data
        uint32_t tagSetCalculation(uint64_t address);//makes tag from address
        uint32_t searchLru(cacheSet_t *set);//searh LRU to substitue
        uint32_t installLine(uint64_t address);//install line of cache |mem_controller -> caches|
        uint32_t moveLineTo(uint64_t address,cache_t *cache,linha_t *linha);// move line to a upper or lower cache level
        uint32_t read(uint64_t address,uint32_t &ttc);
        uint32_t write(uint64_t address,int32_t line);
        //getters setters
        INSTANTIATE_GET_SET_ADD(uint32_t,cacheHit);
        INSTANTIATE_GET_SET_ADD(uint32_t,cacheMiss);
        INSTANTIATE_GET_SET_ADD(uint32_t,cacheAccess);
        INSTANTIATE_GET_SET_ADD(uint32_t,cacheRead);
        INSTANTIATE_GET_SET_ADD(uint32_t,cacheWrite);
        INSTANTIATE_GET_SET_ADD(uint32_t,cacheWriteBack);
   
};

#endif // CACHE_H
