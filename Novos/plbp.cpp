#include "simulator.hpp"
#include <algorithm>
#include <cmath>
plbp_t::plbp_t(){
    this->W = NULL;
    this->GA = NULL;
    this->GHR = NULL;
};
plbp_t::~plbp_t(){
    if((this->W)) {
        for (size_t i = 0; i < N; i++)
        {            
            for (size_t j = 0; j < M; j++)
            {   
                delete [] this->W[i][j];
            }
            delete[] this->W[i];

        }
    }
    delete[] W;
    
    if(this->GA) delete [] GA;
    if(this->GHR) delete [] GHR;
};

void plbp_t::allocate(){
    this->W = new int8_t**[N];
    
    for (size_t i = 0; i < N; i++)
    {
        this->W[i] = new int8_t*[M];
        for (size_t j = 0; j < M; j++)
        {
            this->W[i][j] = new int8_t[H+1];
            
        }
    }
    
    this->GA = new uint32_t[M];
    this->GHR = new uint8_t[H];
    this->saida = 0;
}
uint32_t plbp_t::predict(uint64_t address){
    uint32_t indexA = address%N;
    uint32_t indexB = address%M;
    this->saida = this->W[indexA][indexB][0];
    for (size_t i = 0; i < H; i++)
    {
     (this->GHR[i]==TAKEN)?this->saida += this->W[indexA][this->GA[i]][i]
     :this->saida -= this->W[indexA][this->GA[i]][i];   
    }

    return (this->saida >=0)? TAKEN: NOT_TAKEN;
    
}
void plbp_t::train(uint64_t address,uint32_t taken){
    uint32_t indexA = address%N;
    uint32_t indexB = address%M;
    // fprintf(stderr,"%f\n",this->saida);
    if((abs(this->saida)<THETA)||(this->predict(address) != taken)){
        if(taken==TAKEN){
            this->W[indexA][indexB][0] = this->W[indexA][indexB][0] + 1;
        }else{
            this->W[indexA][indexB][0] = this->W[indexA][indexB][0] - 1;
        }
        for (size_t i = 0; i < H; i++)
        {
            if(this->GHR[i]==TAKEN){
                this->W[indexA][this->GA[i]][i]+=1;
            }else{
                this->W[indexA][this->GA[i]][i]-=1;
            }
        }
    }
    for (size_t i = 0; i < H; i++)
    {
        this->GA[i+1] = this->GA[i];
        this->GHR[i+1] = this->GHR[i];
    }
    this->GA[0]=indexB;
    this->GHR[0]=taken;
};
