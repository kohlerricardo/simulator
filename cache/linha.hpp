#ifndef LINHA_H
#define LINHA_H


class linha_t
{
    public:
        uint64_t tag;
        uint32_t dirty;
        uint64_t lru;
        uint32_t prefetched;
        uint32_t valid;
        uint64_t readyAt;
};

#endif // LINHA_H
