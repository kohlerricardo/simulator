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
    bool close = false;
    FILE *output = stdout;
	if(orcs_engine.output_file_name != NULL){
		output = fopen(orcs_engine.output_file_name,"a+");
        close=true;
    }
	if (output != NULL){
        fprintf(output, "======================== MEMORY DESAMBIGUATION ===========================\n");
        utils_t::largestSeparator(output);
        #if PERFECT
            fprintf(output,"Disambiguation method: PERFECT\n");
            if(close)fclose(output);
            this->disambiguator->statistics();
        #endif
        #if HASHED
            fprintf(output,"Disambiguation method: HASHED\n");
            if(close)fclose(output);
            this->disambiguator->statistics();
        #endif
    }
};