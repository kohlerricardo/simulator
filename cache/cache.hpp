#ifndef CACHE_H
#define CACHE_H


class cache_t
{
    public:
        cache_t();
        virtual ~cache_t();
        /**
        Functions of Cache
        */
        //functions
        void allocate(uint32_t level);
        void statistics();
        //AddressCalc
        uint32_t idxSetCalculation(uint64_t address);
        uint32_t tagSetCalculation(uint64_t address);
        //Search Function
        uint32_t searchAddress(uint64_t address);
        //write function
        uint32_t writeAllocate(uint64_t address);
        inline void writeBack(uint64_t address);
        // inline void writeBack(uint32_t idx,uint32_t line);
        uint32_t installLine(uint64_t address);
        inline uint32_t searchLru(cacheSet_t *set);
        //moving lines between caches
        void moveLineTo(uint64_t address,cache_t *cache,linha_t *linha);
        void returnLine(uint64_t address,cache_t *cache);
        //debugs on the table
        inline void printLine(linha_t *linha);
        //atributtes
        uint32_t id;
        uint32_t level;
        uint32_t nSets;
        uint32_t nLines;
        cacheSet_t *sets;
        uint32_t shiftData;
        uint32_t cacheHit;
        uint32_t cacheMiss;
        uint32_t cacheAccess;

};

#endif // CACHE_H
