#include "../simulator.hpp"
class uop_package_t{
    
    public:
    uop_package_t();
    ~uop_package_t();
    
    /// TRACE Variables
    char opcode_assembly[MAX_ASSEMBLY_SIZE];
    instruction_operation_t opcode_operation;
    uint64_t opcode_address;
    uint32_t opcode_size;

    int32_t read_regs[MAX_REGISTERS];
    int32_t write_regs[MAX_REGISTERS];

    instruction_operation_t uop_operation;
    uint64_t memory_address;
    uint32_t memory_size;

    void opcode_to_uop(uint64_t uop_number, instruction_operation_t uop_operation, uint64_t memory_address, uint32_t memory_size, opcode_package_t opcode);
    void package_clean();

};