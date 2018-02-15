#ifndef UTILS_H
#define UTILS_H


class utils_t
{
    public:
    static uint64_t powerOf2(uint64_t number);
    static uint32_t verifyPowerOf2(uint64_t number);
    static void printCacheInformation(cache_t *cache);
    static void usage();
};

#endif // UTILS_H
