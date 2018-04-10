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
        linha_t* linha_ptr_sup;
        linha_t* linha_ptr_inf;
        linha_t(){
            this->clean_line();
        }
        ~linha_t(){
            // deleting pointes
            if(this->linha_ptr_inf) delete this->linha_ptr_inf;
            if(this->linha_ptr_sup) delete this->linha_ptr_sup;
            // Nulling pointers
            this->linha_ptr_inf = NULL;
            this->linha_ptr_sup = NULL;
        }
        void clean_line(){
            this->tag = 0;
            this->dirty = 0;
            this->lru = 0;
            this->prefetched = 0;
            this->valid = 0;
            this->readyAt = 0;
            this->linha_ptr_inf = NULL;
            this->linha_ptr_sup = NULL;
        }
};

#endif // LINHA_H
