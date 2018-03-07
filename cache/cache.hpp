#ifndef CACHE_H
#define CACHE_H


class cache_t
{

     private:
        uint32_t cacheHit;
        uint32_t cacheMiss;
        uint32_t cacheAccess;
        uint32_t cacheRead;
        uint32_t cacheWrite;
        uint32_t cacheWriteBack;
        //=============
        // instruction related
        //=============

        

    public:
        cache_t();
        ~cache_t();
                //atributtes
        uint32_t id;
        uint32_t level;
        uint32_t nSets;
        uint32_t nLines;
        cacheSet_t *sets;
        uint32_t shiftData;

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
        uint32_t write(uint64_t address,int32_t line);
        inline void writeBack(uint64_t address,linha_t *line);
        // inline void writeBack(uint32_t idx,uint32_t line);
        uint32_t installLine(uint64_t address);
        inline uint32_t searchLru(cacheSet_t *set);
        //moving lines between caches
        uint32_t moveLineTo(uint64_t address,cache_t *cache,linha_t *linha);
        void returnLine(uint64_t address,cache_t *cache,int32_t &retorno);
        //debugs on the table
        inline void printLine(linha_t *linha);
        //getters setters
        INSTANTIATE_GET_SET_ADD(uint32_t,cacheHit);
        INSTANTIATE_GET_SET_ADD(uint32_t,cacheMiss);
        INSTANTIATE_GET_SET_ADD(uint32_t,cacheAccess);
        INSTANTIATE_GET_SET_ADD(uint32_t,cacheRead);
        INSTANTIATE_GET_SET_ADD(uint32_t,cacheWrite);
        INSTANTIATE_GET_SET_ADD(uint32_t,cacheWriteBack);
   
};

#endif // CACHE_H
