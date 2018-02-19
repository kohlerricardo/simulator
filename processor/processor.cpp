#include "../simulator.hpp"

// =====================================================================
processor_t::processor_t() {

};
processor_t::~processor_t(){};

// =====================================================================
void processor_t::allocate() {
	//Allocating Buffers
	this->decodeBuffer.allocate(DECODE_BUFFER);

};

// =====================================================================
void processor_t::fetchDecode(){
opcode_package_t new_instruction;
if (!orcs_engine.trace_reader->trace_fetch(&new_instruction)) {
	/// If EOF
	orcs_engine.simulator_alive = false;
}
}


// =====================================================================


void processor_t::clock(){
/// Get the next instruction from the trace

		this->fetchDecode();
};
// =====================================================================
void processor_t::statistics() {
	
	std::cout<< "######################################################\n"<< std::endl;
	std::cout<< "Total Cicle ;"<<orcs_engine.get_global_cycle()<< std::endl;


};