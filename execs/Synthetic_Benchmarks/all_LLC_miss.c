#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>  

#define N 16
struct list {
    uint64_t value;
    struct list *next_element;
    char pad[48];
};
typedef struct list element;

int main (int argc,char **argv){

    element *array_matrix;
    uint64_t expoente,size,rep,i,j,value;
    
    if(argc < 2){
        printf("USAGE: %s <sizeArray><repeticoes>\n",argv[0]);
        exit(1);
    }
    expoente = atol(argv[1]);
    rep = atol(argv[2]);
    size = 1<<expoente;
    if (size % 32 != 0) {
        printf("The array size needs to be divisible by 32 (due to unrolling).\n");
        exit(EXIT_FAILURE);
    }
    printf("Struct size %"PRIu64"\n", (uint64_t)sizeof(element));
    printf("Repetitions:%"PRIu64" Size:%"PRIu64"\n", rep, size);
    printf("Memory to be accessed: %"PRIu64"KB %"PRIu64"MB\n", (uint64_t)(size*sizeof(element))/1024, (uint64_t)((size*sizeof(element))/1024)/1024);      
    array_matrix = (element*)malloc(sizeof(element)*size);
    if(array_matrix ==NULL){
        printf("ERRO, memoria nao alocada");
        exit(1);
    }
    element *ptr_this;

    ptr_this = array_matrix;
    
    printf("Filling Array\n");
    for (i = 0; i < size; i++) {
        ptr_this->value = 1;
        ptr_this->next_element = &array_matrix[i+1];
        ptr_this = ptr_this->next_element;
        ptr_this->next_element = NULL;
    }
    // asm volatile ("nop");
    // asm volatile ("nop");
    // asm volatile ("nop");

    printf("Array principal\n");
    ptr_this = array_matrix;
    for (i = 0; i < rep; i++) {
        ptr_this = array_matrix;
        for (j = 0; j < size ; j ++){
            ptr_this = ptr_this->next_element;
            value = ptr_this->value;
        }
    }
    // asm volatile ("nop");
    // asm volatile ("nop");
    // asm volatile ("nop");


    free(array_matrix);

    printf("%"PRIu64"\n", value);
    exit(EXIT_SUCCESS);
}