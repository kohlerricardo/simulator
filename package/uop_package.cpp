#include "../simulator.hpp"

uop_package_t::uop_package_t(){};

uop_package_t::~uop_package_t(){};
void uop_package_t::package_clean() {
    /// TRACE Variables
    strcpy(this->opcode_assembly, "N/A");
    this->opcode_operation = INSTRUCTION_OPERATION_NOP;
    this->opcode_address = 0; 
    this->opcode_size = 0;

    memset(this->read_regs, POSITION_FAIL, sizeof(int32_t) * MAX_REGISTERS);
    memset(this->write_regs, POSITION_FAIL, sizeof(int32_t) * MAX_REGISTERS);

    this->uop_operation = INSTRUCTION_OPERATION_NOP;
    this->memory_address = 0;
    this->memory_size = 0;


};

void uop_package_t::opcode_to_uop(uint64_t uop_number, instruction_operation_t uop_operation, uint64_t memory_address, uint32_t memory_size, opcode_package_t opcode) {
    // ERROR_ASSERT_PRINTF(this->state == PACKAGE_STATE_FREE,
    //                     "Trying to decode to uop in a non-free location\n");

    uop_number = uop_number;
    /// TRACE Variables
    strncpy(this->opcode_assembly, opcode.opcode_assembly, sizeof(this->opcode_assembly));
    this->opcode_operation = opcode.opcode_operation;
    this->opcode_address = opcode.opcode_address;
    this->opcode_size = opcode.opcode_size;

    memcpy(this->read_regs, opcode.read_regs, sizeof(int32_t) * MAX_REGISTERS);
    memcpy(this->write_regs, opcode.write_regs, sizeof(int32_t) * MAX_REGISTERS);

    this->uop_operation = uop_operation;
    this->memory_address = memory_address;
    this->memory_size = memory_size;

 
};