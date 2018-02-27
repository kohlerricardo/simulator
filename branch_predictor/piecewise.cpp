#include "../simulator.hpp"
#include <algorithm>
#include <cmath>

piecewise_t::piecewise_t(){
    this->W = NULL;
    this->GA = NULL;
    this->GHR = NULL;
};
piecewise_t::~piecewise_t(){
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

void piecewise_t::allocate(){
    // fprintf(stderr,"alocando piecewise");
    this->W = new int8_t**[N];
    
    for (size_t i = 0; i < N; i++)
    {
        this->W[i] = new int8_t*[M];
        for (size_t j = 0; j < M; j++)
        {
            this->W[i][j] = new int8_t[H+1];
            
        }
    }
    // memset(this->W,0,((M*N*H)*sizeof(int8_t)));    
    this->GA = new uint32_t[M];
    this->GHR = new uint8_t[H];
    this->saida = 0;
}
taken_t piecewise_t::predict(uint64_t address){
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
void piecewise_t::train(uint64_t address,taken_t predict, taken_t correct){
    uint32_t indexA = address%N;
    uint32_t indexB = address%M;
    // fprintf(stderr,"%f\n",this->saida);
    if((abs(this->saida)<THETA)||(predict != correct)){
        if(correct==TAKEN){
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
    this->GHR[0]=correct;
};
