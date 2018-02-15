#ifndef CACHESET_H
#define CACHESET_H


class cacheSet_t
{
    public:
    linha_t *linhas;
    cacheSet_t(){
    this->linhas = NULL;
    }
    ~cacheSet_t(){
    if(this->linhas) delete[] &linhas;
    }

};

#endif // CACHESET_H
