#include "../simulator.hpp"
#include <string>
uop_package_t::uop_package_t(){};

uop_package_t::~uop_package_t(){};
void uop_package_t::package_clean()
{
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
    //controle
    this->opcode_number = 0;
    this->uop_number = 0;
    this->readyAt = orcs_engine.get_global_cycle();;
    this->status =PACKAGE_STATE_FREE;
};

void uop_package_t::opcode_to_uop(uint64_t uop_number, instruction_operation_t uop_operation, uint64_t memory_address, uint32_t memory_size, opcode_package_t opcode)
{
    // ERROR_ASSERT_PRINTF(this->state == PACKAGE_STATE_FREE,
    //                     "Trying to decode to uop in a non-free location\n");

    this->uop_number = uop_number;
    /// TRACE Variables
    strncpy(this->opcode_assembly, opcode.opcode_assembly, sizeof(this->opcode_assembly));
    this->opcode_operation = opcode.opcode_operation;
    this->opcode_address = opcode.opcode_address;
    this->opcode_size = opcode.opcode_size;
    this->opcode_number = opcode.opcode_number;
    memcpy(this->read_regs, opcode.read_regs, sizeof(int32_t) * MAX_REGISTERS);
    memcpy(this->write_regs, opcode.write_regs, sizeof(int32_t) * MAX_REGISTERS);

    this->uop_operation = uop_operation;
    this->memory_address = memory_address;
    this->memory_size = memory_size;
};
void uop_package_t::updatePackageUntrated(uint32_t stallTime){
    this->status = PACKAGE_STATE_UNTREATED;
    this->readyAt = orcs_engine.get_global_cycle()+stallTime;
};
void uop_package_t::updatePackageReady(uint32_t stallTime){
    this->status = PACKAGE_STATE_READY;
    this->readyAt = orcs_engine.get_global_cycle()+stallTime;
};
/// Convert Instruction variables into String
std::string uop_package_t::content_to_string() {
    std::string content_string;
    content_string = "";
    content_string = content_string + " " + get_enum_instruction_operation_char(this->uop_operation);

    content_string = content_string + " Address $" + utils_t::big_uint64_to_string(this->memory_address);
    content_string = content_string + " Size:" + utils_t::uint32_to_string(this->memory_size);


    content_string = content_string + " | RRegs[";
    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        if (this->read_regs[i] >= 0) {
            content_string = content_string + " " + utils_t::uint32_to_string(this->read_regs[i]);
        }
    }

    content_string = content_string + " ] | WRegs[";
    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        if (this->write_regs[i] >= 0) {
            content_string = content_string + " " + utils_t::uint32_to_string(this->write_regs[i]);
        }
    }
    content_string = content_string + " ]";

    return content_string;
};
