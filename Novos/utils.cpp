#include "simulator.hpp"

uint64_t utils_t::powerOf2(uint64_t number){
    if(number == 0 ){
        return 0;
    }
    if(verifyPowerOf2(number)){
        int64_t expoente = -1;
        while (number != 0) {
            number >>= 1;
            expoente++;
        }
        return expoente;
    }else{
        fprintf(stderr,"Erro! Numero informado nao e potencia de 2\n");
        return FAIL;
    }
};

uint32_t utils_t::verifyPowerOf2(uint64_t number){
    uint32_t i, count = 0;
    uint64_t pow;
    /// Count the number of 1 bits in the number
    for (i = 0; i < 64 && number != 0 ; i++) {
        pow = number & (1 << i);
        count += (pow != 0);
        number -= pow;
    }

    if (count == 1) {
        return OK;
    }
    else {
        return FAIL;
    }
};

void utils_t::usage(){
    fprintf(stderr,"./CacheSimulator <n_levels><level1><level2>...<levelX>\n\n");
    

};


