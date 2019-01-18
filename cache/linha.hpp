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
        linha_t* linha_ptr_l1;
        linha_t* linha_ptr_l2;
        linha_t* linha_ptr_llc;

        linha_t* linha_ptr_emc;
        linha_t(){
            this->clean_line();
        }
        ~linha_t(){
            // deleting pointes
            if(this->linha_ptr_l1 != NULL) delete &linha_ptr_l1;
            if(this->linha_ptr_l2 != NULL) delete &linha_ptr_l2;
            if(this->linha_ptr_llc != NULL) delete &linha_ptr_llc;
            #if EMC_ACTIVE
            if(this->linha_ptr_emc != NULL) delete &linha_ptr_emc;
            #endif
            // Nulling pointers
            this->linha_ptr_l1 = NULL;
            this->linha_ptr_l2 = NULL;
            this->linha_ptr_llc = NULL;
            #if EMC_ACTIVE
            this->linha_ptr_emc = NULL;
            #endif
        }
        void clean_line(){
            this->tag = 0;
            this->dirty = 0;
            this->lru = 0;
            this->prefetched = 0;
            this->valid = 0;
            this->readyAt = 0;
            this->linha_ptr_l1 = NULL;
            this->linha_ptr_l2 = NULL;
            this->linha_ptr_llc = NULL;
            #if EMC_ACTIVE
            this->linha_ptr_emc = NULL;
            #endif
        }
};

#endif // LINHA_H
