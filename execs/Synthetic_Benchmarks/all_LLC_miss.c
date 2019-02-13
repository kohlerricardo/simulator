#include<stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <inttypes.h>  
#include <wchar.h>
#define N 16
int main (int argc,char **argv){

    uint32_t *array_matrix;
    uint32_t expoente;
    uint32_t acumula=0;
    uint32_t nLines;
    if(argc < 2){
        printf("USAGE: %s <# EXP LinesMatrix>\n",argv[0]);
        exit(1);
    }
    expoente = atoi(argv[1]);
    nLines = pow(2,expoente);
    // printf("Exp %u, nLines= %u\n",expoente,nLines);
    array_matrix = malloc(nLines*N*sizeof(uint32_t));
    if(array_matrix ==NULL){
        printf("ERRO, memoria nao alocada");
        exit(1);
    }
    for(size_t i = 0; i < nLines*N; i++)
    {
        array_matrix[i]=1;
    }
    for(size_t i = 0; i < nLines; i++)
    {
        for(size_t j = 0; j < N; j++)
        { 
            // printf("position %lu, value %u\n",(i*N)+j,array_matrix[(i*N)+j]);
            // printf("%u\t",array_matrix[(i*N)+j]);
            acumula += array_matrix[(i*N)+j];
            j=N; 
        }
            // printf("\n");
        
    }
    printf("%u\n",acumula);
    free(array_matrix);
    return 0;
}