#include "../../simulator.hpp"

desambiguation_t::desambiguation_t(){
    #if PERFECT
        this->disambiguator = NULL;
    #endif
    #if HASHED
        this->disambiguator = NULL;
    #endif
}

desambiguation_t::~desambiguation_t(){
    #if PERFECT
        delete this->disambiguator;
    #endif
    #if HASHED
        delete this->disambiguator;
    #endif
}
void desambiguation_t::allocate(){
    #if PERFECT
        this->disambiguator = new disambiguation_perfect_t;
        this->disambiguator->allocate();
    #endif
    #if HASHED
        this->disambiguator = new disambiguation_hashed_t;
        this->disambiguator->allocate();
    #endif
};
void desambiguation_t::make_memory_dependences(memory_order_buffer_line_t *mob_line){
    this->disambiguator->make_memory_dependencies(mob_line);
};
void desambiguation_t::solve_memory_dependences(memory_order_buffer_line_t *mob_line){
    this->disambiguator->solve_memory_dependencies(mob_line);
};
void desambiguation_t::statistics(){
    if(orcs_engine.output_file_name == NULL){
        #if PERFECT
            ORCS_PRINTF("Disambiguation method: PERFECT\n")
            this->disambiguator->statistics();
        #endif
        #if HASHED
            ORCS_PRINTF("Disambiguation method: HASHED\n")
            this->disambiguator->statistics();
        #endif
    }
    else{
        FILE *output = fopen(orcs_engine.output_file_name,"a+");
		if(output != NULL){
        #if PERFECT
            fprintf(output,"Disambiguation method: PERFECT\n");
            this->disambiguator->statistics();
        #endif
        #if HASHED
            fprintf(output,"Disambiguation method: HASHED\n");
            this->disambiguator->statistics();
        #endif
        }
        fclose(output);
    }
};